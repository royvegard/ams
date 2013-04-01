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
#include "m_amp.h"
#include "port.h"

M_amp::M_amp(QWidget* parent)
  : Module(M_type_amp, 1, parent, tr("Amplifier"))
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_AMP_HEIGHT);
  port_M_in = new Port(tr("In"), PORT_IN, 0, this);
  cv.out_off = 55;
  port_out = new Port(tr("Out"), PORT_OUT, 0, this);
  gain = 1;
  configDialog->addSlider(tr("&Gain"), gain, -10, 10);
}

void M_amp::generateCycle() {

  int l1;
  unsigned int l2;

    inData = port_M_in->getinputdata();

    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = gain * inData[l1][l2];
      }
    }
}

