/*
    ScreenLogger - A tool that regularly saves screenshots.
  Copyright (C) 2021 Lukas Boersma <mail@lukas-boersma.com>
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "screenshot.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QDir>
#include <QException>
#include <QSettings>
#include <QTextCodec>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDebug>
#include <QMessageBox>
#include <qstandardpaths.h>
#include <string>
#include <cmath>
#include <QTemporaryDir>
#include <QDesktopServices>

namespace
{

class SettingsFileException: public std::exception
{
public:
    SettingsFileException(const QString& _msg):
        msg(_msg.toStdString())
    {

    }

    virtual const char* what() const noexcept override
    {
        return msg.c_str();
    }

private:
    std::string msg;
};

}

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::Dialog | Qt::WindowCloseButtonHint),
    ui(new Ui::SettingsDialog)
{
    connect(this, &QDialog::rejected, [this]() {
        reloadSettings();
    });

    auto codec = QTextCodec::codecForName("UTF-8");
    settingsFile.setIniCodec(codec);

    ui->setupUi(this);
    ui->imageFolderBtn->setIcon(ui->imageFolderBtn->style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->aboutBtn->setIcon(ui->aboutBtn->style()->standardIcon(QStyle::SP_MessageBoxInformation));

    ui->timeBetweenCapturesUnitCombo->addItem("Seconds");
    ui->timeBetweenCapturesUnitCombo->addItem("Minutes");

    QWidget::setTabOrder(ui->aboutBtn, ui->dlgBtnBox->buttons().at(0));
    QWidget::setTabOrder(ui->dlgBtnBox->buttons().at(0), ui->dlgBtnBox->buttons().at(1));

    reloadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::accept()
{
    if(applySettings())
    {
        QDialog::accept();
    }
}

void SettingsDialog::selectImageFolder()
{
    auto newDir = QFileDialog::getExistingDirectory(this, "Select image folder", ui->imageFolderEdit->text());
    ui->imageFolderEdit->setText(newDir);
}

void SettingsDialog::reloadSettings()
{
    Settings newSettings = settings;

    try
    {
        // Image folder
        {
            auto imageFolderDefault = QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + QDir::separator() + "/ScreenLog/");
            auto imageFolderVariant = settingsFile.value("ImageFolder", imageFolderDefault);

            if(!imageFolderVariant.canConvert(QVariant::String))
            {
                settingsFile.remove("ImageScale");
                throw SettingsFileException("Invalid value for ImageFolder setting");
            }

            newSettings.imageFolder = imageFolderVariant.toString();
            ui->imageFolderEdit->setText(newSettings.imageFolder);
        }

        // Time between captures
        {
            auto timeBetweenCapturesUnitVariant = settingsFile.value("TimeBetweenCapturesUnit", "Minutes");

            if(!timeBetweenCapturesUnitVariant.canConvert(QVariant::String))
            {
                settingsFile.remove("TimeBetweenCapturesUnit");
                throw SettingsFileException("Invalid value for TimeBetweenCapturesUnit setting. Must be a string.");
            }

            auto timeBetweenCapturesUnitString = timeBetweenCapturesUnitVariant.toString();
            if(timeBetweenCapturesUnitString != "Seconds" && timeBetweenCapturesUnitString != "Minutes")
            {
                settingsFile.remove("TimeBetweenCapturesUnit");
                throw SettingsFileException("Invalid value for TimeBetweenCapturesUnit setting. Must be either 'Seconds' or 'Minutes'.");
            }

            newSettings.timeBetweenCapturesUnit = timeBetweenCapturesUnitString == "Minutes" ? Settings::TimeUnit::Minutes : Settings::TimeUnit::Seconds;

            ui->timeBetweenCapturesUnitCombo->setCurrentIndex((int)newSettings.timeBetweenCapturesUnit);

            auto timeBetweenCapturesValueVariant = settingsFile.value("TimeBetweenCaptures", "1");

            if(!timeBetweenCapturesValueVariant.canConvert(QVariant::Double))
            {
                settingsFile.remove("TimeBetweenCaptures");
                throw SettingsFileException("Invalid value for TimeBetweenCapturesUnit setting. Must be a number.");
            }

            newSettings.timeBetweenCaptures = timeBetweenCapturesValueVariant.toDouble();

            if(newSettings.timeBetweenCaptures <= 0 || !std::isfinite(newSettings.timeBetweenCaptures))
            {
                settingsFile.remove("TimeBetweenCaptures");
                throw SettingsFileException("Invalid value for TimeBetweenCapturesUnit setting. Must be a positive number.");
            }

            ui->timeBetweenCapturesEdit->setValue(newSettings.timeBetweenCaptures);
        }

        // Folder structure
        {
            auto folderStructureVariant = settingsFile.value("FolderStructure", "Flat");

            if(!folderStructureVariant.canConvert(QVariant::String))
            {
                settingsFile.remove("FolderStructure");
                throw SettingsFileException("Invalid value for FolderStructure setting. Must be a string.");
            }

            auto folderStructureString = folderStructureVariant.toString();

            if(folderStructureString != "Flat" && folderStructureString != "Day")
            {
                settingsFile.remove("FolderStructure");
                throw SettingsFileException("Invalid value for FolderStructure setting. Must be either 'Day' or 'Flat'.");
            }

            newSettings.folderStructure = folderStructureString == "Flat" ? Settings::FolderStructure::Flat : Settings::FolderStructure::Day;

            ui->folderStructureFlatRadioBtn->setChecked(newSettings.folderStructure == Settings::FolderStructure::Flat);
            ui->folderStructureDayRadioBtn->setChecked(newSettings.folderStructure == Settings::FolderStructure::Day);
        }

        // Image scale
        {
            auto imageScaleVariant = settingsFile.value("ImageScale", 1);

            if(!imageScaleVariant.canConvert(QVariant::Double))
            {
                settingsFile.remove("ImageScale");
                throw SettingsFileException("Invalid value for ImageScale setting. Must be a number.");
            }

            newSettings.imageScale = imageScaleVariant.toDouble();

            if(newSettings.imageScale <= 0 || newSettings.imageScale > 1)
            {
                settingsFile.remove("ImageScale");
                throw SettingsFileException("Invalid value for ImageScale setting. Must be larger than 0 and must be less or equal to 1.");
            }

            ui->imageScaleEdit->setValue(newSettings.imageScale * 100.0);
        }

        // Image format
        {
            auto imageFormatVariant = settingsFile.value("ImageFormat", "jpeg");

            if(!imageFormatVariant.canConvert(QVariant::String))
            {
                settingsFile.remove("ImageFormat");
                throw SettingsFileException("Invalid value for ImageFormat setting. Must be a string.");
            }

            auto imageFormatString = imageFormatVariant.toString().toLower();

            if(imageFormatString != "png" && imageFormatString != "jpeg")
            {
                settingsFile.remove("ImageFormat");
                throw SettingsFileException("Invalid value for ImageFormat setting. Must be either 'jpeg' or 'png'.");
            }

            newSettings.imageFormat = imageFormatString == "png" ? Settings::ImageFormat::Png : Settings::ImageFormat::Jpeg;
            ui->imageFormatJpgRadioBtn->setChecked(newSettings.imageFormat == Settings::ImageFormat::Jpeg);
            ui->imageFormatPngRadioBtn->setChecked(newSettings.imageFormat == Settings::ImageFormat::Png);
        }

        // Color mode
        {
            auto colorModeVariant = settingsFile.value("ImageColorMode", "fullcolor");

            if(!colorModeVariant.canConvert(QVariant::String))
            {
                settingsFile.remove("ImageColorMode");
                throw SettingsFileException("Invalid value for ImageColorMode setting. Must be a string.");
            }

            auto colorModeString = colorModeVariant.toString().toLower();

            if(colorModeString != "fullcolor" && colorModeString != "blackandwhite")
            {
                settingsFile.remove("ImageColorMode");
                throw SettingsFileException("Invalid value for ImageColorMode setting. Must be either 'fullcolor' or 'blackandwhite'.");
            }

            newSettings.colorMode = colorModeString == "fullcolor" ? Settings::ColorMode::FullColor : Settings::ColorMode::BlackAndWhite;
            ui->colorModeColorRadioBtn->setChecked(newSettings.colorMode == Settings::ColorMode::FullColor);
            ui->colorModeBWRadioBtn->setChecked(newSettings.colorMode == Settings::ColorMode::BlackAndWhite);
        }

        // Timestamp
        {
            auto timestampVariant = settingsFile.value("ImageTimestampText", true);

            if(!timestampVariant.canConvert(QVariant::Bool))
            {
                settingsFile.remove("ImageTimestampText");
                throw SettingsFileException("Invalid value for ImageTimestampText setting. Must be a boolean value.");
            }

            newSettings.addTimestamp = timestampVariant.toBool();

            ui->timestampCheckBox->setChecked(newSettings.addTimestamp);
        }

        // If we got here, we can safely load the settings
        settings = newSettings;
    }
    catch(const SettingsFileException& exception)
    {
        QMessageBox msgBox;
        msgBox.critical(
            this,
            "Error while loading settings",
            QString("The following error occured while loading the settings:\n ") + exception.what() + "\nThe application will exit now."
        );
        settingsFile.sync();
        QCoreApplication::exit(1);
    }
}

bool SettingsDialog::applySettings()
{
    Settings newSettings = readSettingsFromUi();

    bool dirIsOk = false;
    try
    {
        if(!QDir(newSettings.imageFolder).exists())
        {
            QDir().mkpath(newSettings.imageFolder);
        }
        dirIsOk = QDir(newSettings.imageFolder).exists();
    }
    catch(...)
    {
        dirIsOk = false;
    }

    QFileInfo dir(newSettings.imageFolder);

    if(!dirIsOk)
    {
        if(dir.exists() && !dir.isDir())
        {
            QMessageBox().critical(nullptr, "ScreenLogger Error", "The selected screenshot destination '" + newSettings.imageFolder + "' already exists, but is not a directory. Please select a valid directory in the settings.");
            return false;
        }
        else
        {
            QMessageBox().critical(nullptr, "ScreenLogger Error", "The selected screenshot destination folder '" + newSettings.imageFolder + "' does not exist and could not be created. Please select a valid directory in the settings.");
            return false;
        }
    }

    if(!dir.isWritable())
    {
        QMessageBox().critical(nullptr, "ScreenLogger Error", "The selected screenshot destination '" + newSettings.imageFolder + "' is not writable for ScreenLogger. Please select a writable directory or make sure that ScreenLogger has the permission to create files in your selected directory.");
        return false;
    }

    settingsFile.setValue("ImageFolder", newSettings.imageFolder);
    settingsFile.setValue("TimeBetweenCapturesUnit", newSettings.timeBetweenCapturesUnit == Settings::TimeUnit::Seconds ? "Seconds" : "Minutes");
    settingsFile.setValue("TimeBetweenCaptures", newSettings.timeBetweenCaptures);
    settingsFile.setValue("FolderStructure", newSettings.folderStructure == Settings::FolderStructure::Day ? "Day" : "Flat");
    settingsFile.setValue("ImageScale", newSettings.imageScale);
    settingsFile.setValue("ImageFormat", newSettings.imageFormat == Settings::ImageFormat::Jpeg ? "jpeg" : "png");
    settingsFile.setValue("ImageColorMode", newSettings.colorMode == Settings::ColorMode::FullColor ? "fullcolor" : "blackandwhite");
    settingsFile.sync();

    Settings oldSettings = settings;
    settings = newSettings;

    if(oldSettings.secondsBetweenCaptures() != newSettings.secondsBetweenCaptures())
    {
        emit timeBetweenCapturesChanged();
    }

    return true;
}

Settings SettingsDialog::readSettingsFromUi()
{
    Settings newSettings;
    newSettings.imageFolder = ui->imageFolderEdit->text();
    newSettings.timeBetweenCapturesUnit = (Settings::TimeUnit)ui->timeBetweenCapturesUnitCombo->currentIndex();
    newSettings.timeBetweenCaptures = ui->timeBetweenCapturesEdit->value();
    newSettings.folderStructure = ui->folderStructureFlatRadioBtn->isChecked() ? Settings::FolderStructure::Flat : Settings::FolderStructure::Day;
    newSettings.imageScale = ui->imageScaleEdit->value() / 100.0;
    newSettings.imageFormat = ui->imageFormatPngRadioBtn->isChecked() ? Settings::ImageFormat::Png : Settings::ImageFormat::Jpeg;
    newSettings.colorMode = ui->colorModeColorRadioBtn->isChecked() ? Settings::ColorMode::FullColor : Settings::ColorMode::BlackAndWhite;
    newSettings.addTimestamp = ui->timestampCheckBox->isChecked();
    return newSettings;
}

void SettingsDialog::on_imageFolderBtn_clicked()
{
    selectImageFolder();
}

void SettingsDialog::on_showImageFolderBtn_clicked()
{
    auto path = ui->imageFolderEdit->text();
    QDir().mkpath(path);
    QDesktopServices::openUrl( QUrl::fromLocalFile(path) );
}

void SettingsDialog::on_aboutBtn_clicked()
{
    emit showAboutDialog();
}

void SettingsDialog::on_exampleScreenshot_clicked()
{
    static QTemporaryDir tempDir(QDir::cleanPath(QDir::tempPath() + QDir::separator() + "ScreenLoggerExample"));

    Settings currentSettings = readSettingsFromUi();
    currentSettings.imageFolder = tempDir.path();
    auto fileName = ScreenShot::takeScreenshot(currentSettings);
    if(fileName != "")
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    }
    else
    {
        QMessageBox msgBox;
        msgBox.critical(this, "ScreenLogger Error", "Failed to capture an example screenshot.");
    }
}
