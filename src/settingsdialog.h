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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include "settings.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    Settings settings;
    QSettings settingsFile;

signals:
    void timeBetweenCapturesChanged();
    void showAboutDialog();

public slots:
    virtual void accept() override;

private slots:
    void on_imageFolderBtn_clicked();
    void on_showImageFolderBtn_clicked();

    void on_aboutBtn_clicked();

    void on_exampleScreenshot_clicked();

private:
    void selectImageFolder();
    void reloadSettings();
    bool applySettings();
    Settings readSettingsFromUi();
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
