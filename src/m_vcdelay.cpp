/*
  Voltage-controlled delay - derived from m_delay.cpp

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
#include "m_vcdelay.h"
#include "port.h"


M_vcdelay::M_vcdelay(QWidget* parent)
    : Module(M_type_vcdelay, 1, parent, tr("VC Delay"))
{
    QString qs;
    int l1;

    setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_VCDELAY_HEIGHT);
    port_M_in = new Port(tr("In"), PORT_IN, 0, this);
    port_M_delay = new Port(tr("Delay"), PORT_IN, 1, this);
    cv.out_off = 55;
    port_out = new Port(tr("Out"), PORT_OUT, 0, this);

    delay = 0;
    configDialog->addSlider(tr("&Delay"), delay, 0, 10);
    vmod = 0;
    configDialog->addSlider(tr("V &Mod"), vmod, 0, 1, TRUE );

    buf = (float **)malloc(synthdata->poly * sizeof(float *));
    for (l1 = 0; l1 < synthdata->poly; l1++) {
        buf[l1] = (float *)malloc(MAX_VCDELAY_FRAMES * sizeof(float));
        memset(buf[l1], 0, MAX_VCDELAY_FRAMES * sizeof(float));
    }
    read_ofs = 0;
}

M_vcdelay::~M_vcdelay() {

    int l1;

    for (l1 = 0; l1 < synthdata->poly; l1++) {
        free(buf[l1]);
    }
    free(buf);
}

void M_vcdelay::generateCycle() {

    int l1, ofs, delay_frames, vc_delay_frames;
    unsigned int l2;

    inData = port_M_in->getinputdata();
    delayData = port_M_delay->getinputdata();

    delay_frames = (int)((float)((MAX_VCDELAY_FRAMES/2) - 3) * delay / 10.0);
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
            buf[l1][read_ofs] = inData[l1][l2];
            vc_delay_frames = delay_frames * (1 + delayData[l1][l2] * vmod);
            ofs = read_ofs - vc_delay_frames;
            ofs = ofs % MAX_VCDELAY_FRAMES;
            if (ofs < 0) ofs += MAX_VCDELAY_FRAMES;	
            data[0][l1][l2] = buf[l1][ofs];
        }
        read_ofs++;
        if (read_ofs >= MAX_VCDELAY_FRAMES) read_ofs = 0;
    }
}

