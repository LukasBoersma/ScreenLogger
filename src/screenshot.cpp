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

#include <QDateTime>
#include <QDir>
#include <QMessageBox>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QPainterPath>

namespace
{
    QPixmap grabScreens()
    {
      auto screens = QGuiApplication::screens();
      QList<QPixmap> scrs;
      int w = 0, h = 0, p = 0;
      foreach (auto scr, screens)
      {
        QPixmap pix = scr->grabWindow(0);
        w += pix.width();
        if (h < pix.height()) h = pix.height();
        scrs << pix;
      }
      QPixmap final(w, h);
      QPainter painter(&final);
      final.fill(Qt::black);
      foreach (auto scr, scrs)
      {
        painter.drawPixmap(QPoint(p, 0), scr);
        p += scr.width();
      }
      return final;
    }
}

QString ScreenShot::takeScreenshot(const Settings& settings)
{
    auto now = QDateTime::currentDateTime();
    auto timestamp = now.toString(Qt::ISODate);
    auto fileNamePrefix = timestamp.replace(":", ".");
    auto path = settings.imageFolder;

    if(settings.folderStructure == Settings::FolderStructure::Day)
    {
        path = QDir::cleanPath(path + QDir::separator() + now.toString("yyyy-MM-dd"));
    }

    auto fileEnding = settings.imageFormat == Settings::ImageFormat::Jpeg ? ".jpeg" : ".png";

    try
    {
        if(!QDir(path).exists())
        {
            QDir().mkpath(path);
        }
    }
    catch(...)
    {
        QMessageBox().critical(nullptr, "ScreenLogger Error", "The screenshot capturing failed because the selected screenshot destination folder '" + path + "' does not exist and could not be created. Please select a valid directory in the settings.");
        return "";
    }


    QPixmap screenShot;

    try
    {
        screenShot = grabScreens();
    }  catch (...)
    {
        QMessageBox().critical(nullptr, "ScreenLogger Error", "Capturing the screen contents failed. Please make sure that ScreenLogger has permissions to record the screen contents.");
    }

    auto scaledScreenshot = screenShot.scaled(
                (int)(screenShot.width() * settings.imageScale), (int)(screenShot.height() * settings.imageScale),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            ).toImage();

    scaledScreenshot = scaledScreenshot.convertToFormat(settings.colorMode == Settings::ColorMode::FullColor ? QImage::Format_RGB32 : QImage::Format_Grayscale8);

    if(settings.addTimestamp)
    {
        QPainter painter;
        painter.begin(&scaledScreenshot);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);

        QFont font("Helvetica");
        font.setPixelSize(20);
        font.setBold(true);
        QPainterPath textPath;
        textPath.addText(QPoint(5, scaledScreenshot.height() - 5), font, timestamp);

        QPen outline = painter.pen();
        outline.setWidth(4);
        outline.setStyle(Qt::SolidLine);
        outline.setCosmetic(true);
        QBrush outlineBrush = outline.brush();
        outlineBrush.setColor(QColor("white"));
        outline.setBrush(outlineBrush);
        painter.setPen(outline);

        QBrush infillBrush = painter.brush();
        infillBrush.setColor(QColor("black"));
        infillBrush.setStyle(Qt::SolidPattern);

        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawPath(textPath);
        painter.fillPath(textPath, infillBrush);
        painter.end();
    }

    auto fileName = QDir::cleanPath(path + QDir::separator() + fileNamePrefix + fileEnding);

    if(!scaledScreenshot.save(fileName))
    {
        QMessageBox().critical(nullptr, "ScreenLogger Error", "There was an error while saving the screenshot. Please make sure that ScreenLogger has permissions to save files in the directory that you selected in the settings.");
        return "";
    }

    return fileName;
}

