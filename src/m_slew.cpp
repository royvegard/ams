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
#include "m_slew.h"
#include "port.h"

M_slew::M_slew(QWidget* parent)
  : Module(M_type_slew, 1, parent, tr("Slew Limiter"))
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_SLEW_HEIGHT);
  port_M_in = new Port(tr("In"), PORT_IN, 0, this);
  cv.out_off = 55;
  port_out = new Port(tr("Out"), PORT_OUT, 0, this);
  timeUp = 0.5;
  timeDown = 0.5;
  configDialog->addSlider(tr("Time &Up"), timeUp, 0, 10);
  configDialog->addSlider(tr("Time &Down"), timeDown, 0, 10);
}

void M_slew::generateCycle() {

  int l1;
  unsigned int l2;
  float ds, slewUp, slewDown;

    inData = port_M_in->getinputdata ();

    if (timeUp > 0.0001) {
      slewUp = 1.0 / (timeUp * (float)synthdata->rate);
    } else {
      slewUp = 1.0 / (0.0001 * (float)synthdata->rate);
    }
    if (timeDown > 0.0001) {
      slewDown = -1.0 / (timeDown * (float)synthdata->rate);
    } else {
      slewDown = -1.0 / (0.0001 * (float)synthdata->rate);
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        ds = inData[l1][l2] - lastData[l1];
        if (ds > 0) {
          if (ds > slewUp) ds = slewUp;
        } else {
          if (ds < slewDown) ds = slewDown;
        }
        data[0][l1][l2] = lastData[l1] + ds;
        lastData[l1] = data[0][l1][l2];
      }
    }
}

