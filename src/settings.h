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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

class Settings
{
public:

    enum class TimeUnit
    {
        Seconds = 0,
        Minutes = 1
    };

    enum class FolderStructure
    {
        Flat = 0,
        Day = 1
    };

    enum class ImageFormat
    {
        Png = 0,
        Jpeg = 1
    };

    enum class ColorMode
    {
        FullColor = 0,
        BlackAndWhite = 1
    };

    QString imageFolder = "";
    TimeUnit timeBetweenCapturesUnit = TimeUnit::Seconds;
    double timeBetweenCaptures = 60;
    FolderStructure folderStructure = FolderStructure::Flat;
    double imageScale = 1;
    ImageFormat imageFormat = ImageFormat::Jpeg;
    ColorMode colorMode = ColorMode::FullColor;
    bool addTimestamp = true;

    double secondsBetweenCaptures() const { return timeBetweenCaptures * (timeBetweenCapturesUnit == TimeUnit::Minutes ? 60 : 1); }

};

#endif // SETTINGS_H
