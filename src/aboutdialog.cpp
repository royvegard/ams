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

#include <qfont.h>
#include <QDialogButtonBox>
#include <qlayout.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <QString>
#include <QTabWidget>
#include <QTextEdit>

#include "config.h"
#include "aboutdialog.h"

/* application icon */
#include "../pixmaps/ams_32.xpm"

static const char LICENSE_TEXT[] =
"This program is free software; you can redistribute it and/or\n"
"modify it under the terms of the GNU General Public License\n"
"Version 2 as published by the Free Software Foundation.\n\n"

"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n";


AboutDialog::AboutDialog(QWidget* parent): QDialog(parent)
{
    setModal(true);
    setObjectName("AboutDialog");
    setWindowTitle(tr("About %1").arg(PACKAGE));
    QVBoxLayout* baseLayout = new QVBoxLayout(this);
    baseLayout->setSpacing(10);
    setLayout(baseLayout);

    QLabel* label;

    // first line: pixmap, program name, version
    QHBoxLayout* pixmapLayout = new QHBoxLayout();
    label = new QLabel(this);
    pixmapLayout->addWidget(label);
    label->setPixmap(QPixmap(ams_32_xpm));

    label = new QLabel(PACKAGE_STRING, this);
    pixmapLayout->addWidget(label);
    QFont font;
    font.setPointSize(18);
    font.setWeight(QFont::Bold);
    label->setFont(font);

    pixmapLayout->addStretch();
    baseLayout->addLayout(pixmapLayout);

    // second line: description
    label = new QLabel(tr(
                "AlsaModularSynth (ams) is a realtime modular sythesizer\n"
                "and effect processor."
                ), this);
    baseLayout->addWidget(label);

    // third line: web link
    label = new QLabel("http://alsamodular.sourceforge.net/", this);
    baseLayout->addWidget(label);


    QTabWidget* tab = new QTabWidget(this);

    /*Authors tab*/
    QWidget* authors = new QWidget(this);
    QVBoxLayout* authorsLayout = new QVBoxLayout(authors);

    QTextEdit* authorslist = new QTextEdit();
    authorslist->setReadOnly(true);
    authorslist->setText(
            "Matthias Nagorni and Fons Adriaensen\n"
            "(C) 2002-2003 SuSE AG Nuremberg\n"
            "(C) 2003 Fons Adriaensen\n"
            "(C) 2007 Malte Steiner\n"
            "(C) 2007-2011 Karsten Wiese\n"
            "(C) 2008-2012 Guido Scholz\n"
            );

    authorsLayout->addWidget(authorslist);
    tab->addTab(authors, tr("&Authors"));

    /*Contributors tab*/
    QWidget* contributors = new QWidget(this);
    QVBoxLayout* contributorsLayout = new QVBoxLayout(contributors);

    QTextEdit* contributorslist = new QTextEdit();
    contributorslist->setReadOnly(true);
    contributorslist->setText(QString::fromUtf8(
            "Atte Andre Jensen\n"
            "Bill Allen\n"
            "Bill Yerazunis\n"
            "Christoph Eckert\n"
            "Frank Kober\n"
            "Frank Neumann\n"
            "Jörg Anders\n"
            "Sebastien Alaiwan\n"
            "Steve Harris\n"
            ));

    contributorsLayout->addWidget(contributorslist);
    tab->addTab(contributors, tr("&Contributors"));

    /*Thanks tab*/
    QWidget* thanks = new QWidget(this);
    QVBoxLayout* thanksLayout = new QVBoxLayout(thanks);

    QTextEdit* thankslist = new QTextEdit();
    thankslist->setReadOnly(true);
    thankslist->setLineWrapMode(QTextEdit::NoWrap);
    thankslist->setText(QString::fromUtf8(
    "The VCF Module uses the resonant low-pass filter by Paul Kellett and\n"
    "the Cookbook formulae for audio EQ biquad filter coefficients by\n"
    "Robert Bristow-Johnson. The experimental 24 dB Lowpass filters have\n"
    "been taken from http://musicdsp.org/. They are based on the CSound\n"
    "source code, the paper by Stilson/Smith and modifications by Paul\n"
    "Kellett and Timo Tossavainen. The pink noise conversion formula is by\n"
    "Paul Kellett and has been taken from http://musicdsp.org/ as well.\n\n"
    "The author is grateful to Takashi Iwai for instructions about ALSA.\n"
    "Klaas Freitag, Helmut Herold, Stefan Hundhammer and Arvin Schnell\n"
    "answered many questions about Qt. Thanks to Jörg Arndt for valuable\n"
    "hints regarding speed optimization. Torsten Rahn has helped to\n"
    "improve the color scheme of the program. Thanks to Bernhard Kaindl\n"
    "for helpful discussion.\n"
    ));

    thanksLayout->addWidget(thankslist);
    tab->addTab(thanks, tr("&Thanks"));

    /*License tab*/
    QWidget* license = new QWidget(this);
    QVBoxLayout* licenseLayout = new QVBoxLayout(license);

    QTextEdit* licenselist = new QTextEdit();
    licenselist->setReadOnly(true);
    licenselist->setLineWrapMode(QTextEdit::NoWrap);
    licenselist->setText(LICENSE_TEXT);

    licenseLayout->addWidget(licenselist);
    tab->addTab(license, tr("&License"));

    baseLayout->addWidget(tab);


    /*OK Button*/
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    baseLayout->addWidget(buttonBox);
}

