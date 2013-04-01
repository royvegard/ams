/*
  Bit Grinder - derived from m_delay.cpp

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
#include "m_bitgrind.h"
#include "port.h"

M_bitgrind::M_bitgrind(QWidget* parent)
  : Module(M_type_bitgrind, 1, parent, tr("Bit Grinder"))
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_BITGRIND_HEIGHT);
  port_M_in = new Port(tr("In"), PORT_IN, 0, this);
  port_M_sampleRate = new Port(tr("Rate VC"), PORT_IN, 1, this);
  port_M_bits = new Port(tr("Bits VC"), PORT_IN, 2, this);
  cv.out_off = 55;
  port_out = new Port(tr("Out"), PORT_OUT, 0, this);

  sampleRate = 1.00f;
  configDialog->addSlider(tr("&Fractional data rate"), sampleRate, 0, 1.0);
  sampleRate = 1.00f;
  sampleRateMod = 0.0f;
  configDialog->addSlider(tr("&Rate VC modulation"), sampleRateMod, -1.0, 1.0);
  bits = 24;
  configDialog->addSlider(tr("&Bits per Volt"), bits, 1, 24 );
  bits = 24;
  bitsMod = 0.0;
  configDialog->addSlider(tr("Bits VC &modulation"), bitsMod, -1.0, 1.0);

  // allocate some static storage.
  sval = (float *) malloc (synthdata->poly * sizeof (float));

}

M_bitgrind::~M_bitgrind() {
  free (sval);
}

void M_bitgrind::generateCycle() {

  int l1;
  unsigned int l2;

  inData = port_M_in->getinputdata();
  inSampleRate = port_M_sampleRate->getinputdata();
  inBits = port_M_bits->getinputdata();

  for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      samplecounter += (sampleRate + (inSampleRate[l1][l2] * sampleRateMod))
	* synthdata->rate;

      //    Part 1: sample slower than the data rate, if desired.
      if (samplecounter < 0) samplecounter = 0;
      if ( samplecounter > synthdata->rate)
	{
	  samplecounter -= synthdata->rate;
	  sval[l1] = inData[l1][l2];   // sval is a static
	}

      // Part 2: decrease the bit resolution; assume a -1/+1 range
      // for our bit assignment (note that this means a signal of
      // +/- 2volts will get 2 bits even when set to 1 bit.
      unsigned int temp;
      int activebits;
      activebits = bits + (bitsMod * inBits[l1][l2]);
      temp = (int ((sval[l1]) * 16777216.0 )) + 1073741824 ;   // * 2^24 + 2^30
      // Now we have 24 bit precision integer, biased up by 2^30
      // shift it down and back, to clear those low order bits.
      temp = temp >> (26 - (int(activebits)));
      temp = temp << (26 - (int(activebits)));
      data[0][l1][l2] = ((float (temp) - 1073741824.0 ) / 16777216.0)
	+ 1.0 / (float (activebits));
    }
  }
}

