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

#include "settingsdialog.h"
#include "aboutdialog.h"
#include "settings.h"
#include "constants.h"
#include "screenshot.h"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QDebug>
#include <QTimer>
#include <QString>
#include <QStandardPaths>
#include <QDateTime>
#include <QUrl>
#include <QDesktopServices>
#include <QMessageBox>
#include <QSharedMemory>

namespace
{
    QSharedMemory sharedMem("LukasBoersmaScreenLogger");

    bool isAnotherInstanceAlreadyRunning()
    {
        if(!sharedMem.create(1) && sharedMem.error() == QSharedMemory::AlreadyExists)
        {
            // Shared memory already exists. On Linux this does not guarantee that another instance is running.
            // We check if the shared memory can be created after attaching and detach from it.
            // Only if that still fails we can be sure that no other instance is running.
            sharedMem.attach();
            sharedMem.detach();
            if(!sharedMem.create(1) && sharedMem.error() == QSharedMemory::AlreadyExists)
            {
                return true;
            }
        }
        return false;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationDomain("lukas-boersma.com");
    QCoreApplication::setOrganizationName("Lukas Boersma");
    QCoreApplication::setApplicationName("ScreenLogger");

    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    SettingsDialog settingsDialog;
    auto& settings = settingsDialog.settings;

    AboutDialog aboutDialog;
    settingsDialog.connect(&settingsDialog, &SettingsDialog::showAboutDialog, [&](){
        aboutDialog.exec();
    });

    QIcon icon(":/ScreenLogger/icon.png");
    QMenu menu;

    menu.addAction(icon, QString("ScreenLogger ") + SCREENLOGGER_VERSION, [&](){
        aboutDialog.exec();
    });

    menu.addSeparator();

    auto settingsAction = menu.addAction("Settings", [&](){
        settingsDialog.show();
        settingsDialog.setFocus();
    });

    auto settingsBoldFont = settingsAction->font();
    settingsBoldFont.setBold(true);
    settingsAction->setFont(settingsBoldFont);

    menu.addAction("Show image folder", [&](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(settings.imageFolder));
    });

    menu.addSeparator();

    menu.addAction("Exit", [&](){ a.exit(0); })->setMenuRole(QAction::QuitRole);

    QSystemTrayIcon trayIcon;
    trayIcon.setIcon(icon);
    trayIcon.setContextMenu(&menu);
    trayIcon.setToolTip("ScreenLogger is running");

    trayIcon.connect(&trayIcon, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        if(reason == QSystemTrayIcon::DoubleClick)
        {
            settingsDialog.show();
            settingsDialog.setFocus();
        }
    });

    trayIcon.show();

    try
    {
        if(settingsDialog.settingsFile.value("FirstRun", true).toBool())
        {
            trayIcon.showMessage("ScreenLogger is now running in the background", "To change settings or view the screenshots, right-click the red camera icon in your system tray icon panel.");
            settingsDialog.settingsFile.setValue("FirstRun", false);
            settingsDialog.settingsFile.sync();
        }
    }
    catch(...)
    {
        settingsDialog.settingsFile.remove("FirstRun");
        settingsDialog.settingsFile.sync();
    }

    if(isAnotherInstanceAlreadyRunning())
    {
        trayIcon.showMessage("ScreenLogger is already running", "Only one instance of ScreenLogger can run at the same time.");
        return 1;
    }

    QTimer screenshotTimer;
    screenshotTimer.connect(&screenshotTimer, &QTimer::timeout, [&](){
        ScreenShot::takeScreenshot(settings);
    });

    screenshotTimer.setInterval(1000.0 * settings.secondsBetweenCaptures());

    settingsDialog.connect(&settingsDialog, &SettingsDialog::timeBetweenCapturesChanged, [&](){
        screenshotTimer.setInterval(1000.0 * settings.secondsBetweenCaptures());
    });

    screenshotTimer.start();

    return a.exec();
}
