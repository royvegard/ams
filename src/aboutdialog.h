/* 
  About ams dialog

  Copyright (C) 2012 Guido Scholz <guido.scholz@bayernline.de>

  This file is part of ams.

  ams is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2 as
  published by the Free Software Foundation.

  ams is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ams.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <qdialog.h>


class AboutDialog: public QDialog {
    Q_OBJECT
   
public:
    AboutDialog(QWidget* parent = 0);

};

#endif    //ABOUTDIALOG_H
