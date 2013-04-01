/*
  Analog Memory - derived from m_vcdelay.cpp

  This is an emulation of the "back half" of a MODCAN type 57A
  module, a.k.a. a "CV recorder".  You still have to clock it
  in the forward direction (i.e. give it a rising ramp as the
  address) and tell it when to record or not (that's what the
  Write Enable input is).   But on the other hand you can
  read it out asynchronously from writing in; this makes bitbanging
  much more feasible.

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
#include "m_analogmemory.h"
#include "port.h"

M_analogmemory::M_analogmemory(QWidget* parent)
  : Module(M_type_analogmemory, 1, parent, tr("Analog Mem"))
{
  QString qs;
  int l1;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_ANALOGMEMORY_HEIGHT);
  port_M_in_data = new Port(tr("In CV"), PORT_IN, 0, this);
  port_M_in_addr = new Port(tr("Write Addr"), PORT_IN, 1, this);
  port_M_in_WrEna = new Port(tr("Write Ena"), PORT_IN, 2, this);
  port_M_out_addr = new Port(tr("Read Addr"), PORT_IN, 3, this);
  cv.out_off = 35;
  port_outcv = new Port(tr("Out CV"), PORT_OUT, 0, this);

  addrmode = 1;
  QStringList mappingNames;
  mappingNames << tr("Direct (no fill)");
  mappingNames << tr("Linear up only, no fill");
  mappingNames << tr("Linear up only, fill");
  mappingNames << tr("Linear down only, no fill");
  mappingNames << tr("Linear down only, fill");
  mappingNames << tr("Reflected");
  configDialog->addComboBox(tr("&Write addressing mode"),
			     addrmode, mappingNames);

  int cells;
  cell2n = MAX_ANALOGMEMORY_2N_FRAMES;
  cells = 1<<MAX_ANALOGMEMORY_2N_FRAMES;
  configDialog->addIntSlider(tr("&N (for 2^N memory cells)"), cell2n, 1,
			      MAX_ANALOGMEMORY_2N_FRAMES);
  writethresh = -0.1;
  configDialog->addSlider(tr("Write &thresh"), writethresh, -1, 1 );
  buf = (float **)malloc(synthdata->poly * sizeof(float *));
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    buf[l1] = (float *)malloc( cells * sizeof(float));
    memset(buf[l1], 0, cells * sizeof(float));
  }
  lastwrite = (int *) malloc (synthdata->poly * sizeof (int));
  memset (lastwrite, 0, synthdata->poly * sizeof (int));
  offset = 0;  // Must recalculate this for each polyphonic voice!
}

M_analogmemory::~M_analogmemory() {

  int l1;

  for (l1 = 0; l1 < synthdata->poly; l1++) {
    free(buf[l1]);
  }
  free(buf);
  free (lastwrite);
}

void M_analogmemory::generateCycle() {

  int l1;
  unsigned int l2;
  int cells;
  int i;

  inData = port_M_in_data->getinputdata();
  inAddr = port_M_in_addr->getinputdata();
  inWrEna = port_M_in_WrEna->getinputdata();
  outAddr = port_M_out_addr->getinputdata();
  cells = 1 << cell2n;

  for (l1 = 0; l1 < synthdata->poly; l1++) {
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      // First we write, then we read.
      // Calculate addresses, scaled -1 to +1 to match LFOs
      offset = (int) (cells * ((((float) inAddr[l1][l2]) + 1.0)/ 2.0));
      if (offset >= cells)
	offset = cells - 1;
      if (offset < 0) offset = 0;
      if (inWrEna[l1][l2] >= writethresh)
	{
	  if (addrmode == 0)  // direct linear
	    buf[l1][offset] = inData[l1][l2];
	  if (addrmode == 1)   // upward only, no fill.
	    if (offset > lastwrite[l1])
	      {
		buf [l1][offset] = inData[l1][l2];
	      };
	  if (addrmode == 2)   // Upward fill.
	    {
	      if ( offset > lastwrite[l1])  // Simpler case- lw < wo
		{
		  //fprintf (stderr, "L");
		  for (i = lastwrite[l1]+1; i <= offset; i++)
		    buf[l1][i] = inData[l1][l2];
		}
	      //else
	      //if (offset < lastwrite[l1]) // Split case, wrapping past 0
	      //  {
	      // //fprintf (stderr, "W\n");
	      //   for (i = lastwrite[l1]+1; i < cells; i++)
	      //     buf[l1][i] = inData[l1][l2];
	      //   for (i = 0; i < offset; i++)
	      //     buf[l1][i] = inData[l1][l2];
	      //  }
	    };
	  if (addrmode == 3)   // downward only, no fill.
	    if (offset < lastwrite[l1])
	      {
		buf [l1][offset] = inData[l1][l2];
	      };
	  if (addrmode == 4)   // Downward, fill.
	    if ( offset < lastwrite[l1])  // Simpler case- lw < wo
	      {
		for (i = lastwrite[l1]-1; i >= offset; i--)
		  buf[l1][i] = inData[l1][l2];
	      };
	  //else
	  //  if (offset < lastwrite[l1]) // Split case, wrapping past 0
	  //  {
	  //    fprintf (stderr, "W\n");
	  //    for (i = lastwrite[l1]+1; i < cells; i++)
	  //      buf[l1][i] = inData[l1][l2];
	  //    for (i = 0; i < offset; i++)
	  //      buf[l1][i] = inData[l1][l2];
	};
      lastwrite[l1] = offset;

      // then read..
      offset = (int) (cells * ((((float)outAddr[l1][l2]) + 1.0)/ 2.0));
      if (offset >= cells)
	offset = cells - 1;
      if (offset < 0) offset = 0;
      data[0][l1][l2] = buf[l1][offset];
    }
  }
}

