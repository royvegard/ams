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
#include "m_vquant.h"
#include "port.h"

M_vquant::M_vquant(QWidget* parent)
  : Module(M_type_vquant, 1, parent, tr("Quantizer 2"))
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_VQUANT_HEIGHT);
  port_M_in = new Port(tr("In"), PORT_IN, 0, this);
  cv.out_off = 55;
  port_quant = new Port(tr("Out"), PORT_OUT, 0, this);
  gain = 1.0;
  configDialog->addSlider(tr("&Gain"), gain, 0, 10);
}

void M_vquant::generateCycle() {

  int l1;
  unsigned int l2;

    inData = port_M_in->getinputdata();

    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = (int)(gain * inData[l1][l2]);
      }
    }
}
