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
#include <qtimer.h>
#include "synthdata.h"
#include "m_pcmin.h"
#include "module.h"
#include "port.h"


M_pcmin::M_pcmin(QWidget* parent, int port)
  : Module(M_type_pcmin, 2, parent, tr("PCM In"))
 {
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_PCMIN_HEIGHT);
  gain = 0.5;
  mixer_gain[0] = 0.5;
  mixer_gain[1] = 0.5;
  qs.sprintf ("In %2d -> ", port);
  port_out[0] = new Port(qs, PORT_OUT, 0, this);
  qs.sprintf ("In %2d -> ", port + 1);
  port_out[1] = new Port(qs, PORT_OUT, 1, this);
  configDialog->addSlider(tr("&Gain"), gain, 0, 1, false);
  configDialog->addSlider(tr("&Volume 1"), mixer_gain[0], 0, 1, false);
  configDialog->addSlider(tr("V&olume 2"), mixer_gain[1], 0, 1, false);
  if (synthdata->withAlsa) {
    pcmdata[0] = new float[2 * synthdata->periodsize];
    pcmdata[1] = pcmdata[0] + synthdata->periodsize;
  }
}

M_pcmin::~M_pcmin()
{
  if (synthdata->withAlsa)
    delete[] pcmdata[0];
}

void M_pcmin::generateCycle() {

  int l1, l3;
  unsigned int l2;
  float mixgain;

    for (l1 = 0; l1 < 2; l1++) {
      mixgain = gain * mixer_gain[l1];
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          data[l1][l3][l2] = mixgain * pcmdata[l1][l2];
        }
      }
    }
}

