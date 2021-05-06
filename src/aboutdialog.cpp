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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "constants.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::Dialog | Qt::WindowCloseButtonHint),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->infoLabel->setText(QString(SCREENLOGGER_VERSION) + "\n\n" + ui->infoLabel->text());
    setFixedSize(size());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
