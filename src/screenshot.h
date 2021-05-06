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

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include "settings.h"

#include <QList>
#include <QString>


class ScreenShot
{

public:
    static QString takeScreenshot(const Settings &settings);
};

#endif // SCREENSHOT_H
