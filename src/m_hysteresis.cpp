/*
  Hysteresis translater - derived from m_delay.cpp

  Copyright (C) 2011 Bill Yerazunis <yerazunis@yahoo.com>

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>


#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_hysteresis.h"
#include "port.h"

M_hysteresis::M_hysteresis(QWidget* parent)
    : Module(M_type_hysteresis, 1, parent, tr("Hysteresis"))
{
    QString qs;
    int l1;

    setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_HYSTERESIS_HEIGHT);
    port_M_in = new Port(tr("In"), PORT_IN, 0, this);
    cv.out_off = 55;
    port_out = new Port(tr("Out"), PORT_OUT, 0, this);

    // Set up the config screen
    configDialog->setAddStretch(1);
    center = 0;
    configDialog->addSlider(tr("&Center"), center, -2, 2);
    overlap = 0;
    configDialog->addSlider(tr("&Overlap"), overlap, 0, 4);
    lowslope = 0;
    configDialog->addSlider(tr("Low &Slope"), lowslope, -5, 5);
    lowoffset = -1;
    configDialog->addSlider(tr("Lo&w Offset"), lowoffset, -5, 5);
    highslope = 0;
    configDialog->addSlider(tr("&High Slope"), highslope, -5, 5);
    highoffset = 1;
    configDialog->addSlider(tr("H&igh Offset"), highoffset, -5, 5);
    currentsegment = (float *) malloc (sizeof (float) * synthdata->poly);
    for (l1 = 0; l1 < synthdata->poly; l1++)
        currentsegment[l1] = 0;
}

M_hysteresis::~M_hysteresis() {
    free (currentsegment);
}

void M_hysteresis::generateCycle() {

    int l1;
    unsigned int l2;
    float **inData;
    inData = port_M_in->getinputdata();

    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
            // Which segment are we on?  Whichever one we were on before,
            // unless we've crossed an edge.
            if (inData[l1][l2] < (center - overlap))
                currentsegment[l1] = 0;
            if (inData[l1][l2] > (center + overlap))
                currentsegment[l1] = 1;
            // Set output values
            if (currentsegment[l1] == 0)
                data[0][l1][l2] =  // -1;
            (inData[l1][l2] * lowslope)-1;
            if (currentsegment[l1] == 1)
                data[0][l1][l2] = // 1;
            (inData[l1][l2] * highslope)+1;
        }
    }
}

