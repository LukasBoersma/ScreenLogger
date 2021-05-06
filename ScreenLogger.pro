#  ScreenLogger - A tool that regularly saves screenshots.
#  Copyright (C) 2021 Lukas Boersma <mail@lukas-boersma.com>
#  
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

!versionAtLeast(QT_VERSION, 5.12.0) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Use Qt 5.12 or newer")
}

QT       += core gui
QT       -= opengl svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/aboutdialog.cpp \
    src/main.cpp \
    src/screenshot.cpp \
    src/settings.cpp \
    src/settingsdialog.cpp

HEADERS += \
    src/aboutdialog.h \
    src/constants.h \
    src/screenshot.h \
    src/settings.h \
    src/settingsdialog.h

FORMS += \
    src/aboutdialog.ui \
    src/settingsdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res/Resources.qrc

RC_ICONS = res/icon.ico
