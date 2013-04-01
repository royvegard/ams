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
#include "m_vcswitch.h"
#include "port.h"

M_vcswitch::M_vcswitch(QWidget* parent)
  : Module(M_type_vcswitch, 3, parent, tr("VC Switch"))
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCSWITCH_WIDTH, MODULE_VCSWITCH_HEIGHT);
  port_M_cv = new Port(tr("CV"), PORT_IN, 0, this);
  port_M_in[0] = new Port(tr("In 0"), PORT_IN, 1, this);
  port_M_in[1] = new Port(tr("In 1"), PORT_IN, 2, this);
  port_out[0] = new Port(tr("Out 0"), PORT_OUT, 0, this);
  port_out[1] = new Port(tr("Out 1"), PORT_OUT, 1, this);
  port_mix = new Port(tr("Mix"), PORT_OUT, 2, this);

  switchLevel = 0.5;
  configDialog->addSlider(tr("&Switch level"), switchLevel, 0, 10);
}

void M_vcswitch::generateCycle() {

  int l1;
  unsigned int l2;
  float mix1, mix2;

    float **inData[2], **cvData;

    inData[0] = port_M_in[0]->getinputdata ();
    inData[1] = port_M_in[1]->getinputdata ();
    cvData = port_M_cv->getinputdata ();

    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (cvData[l1][l2] > switchLevel) {
          data[0][l1][l2] = inData[1][l1][l2];
          data[1][l1][l2] = inData[0][l1][l2];
        } else {
          data[0][l1][l2] = inData[0][l1][l2];
          data[1][l1][l2] = inData[1][l1][l2];
        }
        mix1 = cvData[l1][l2];
        mix2 = 2.0 * switchLevel - mix1;
        if (mix2 < 0) {
          mix2 = 0;
          mix1 = 2.0 * switchLevel;
        }
        data[2][l1][l2] = (mix1 * inData[0][l1][l2] + mix2 * inData[1][l1][l2]) / (mix1 + mix2);
      }
    }
}

