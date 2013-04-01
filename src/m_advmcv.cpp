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
#include "m_advmcv.h"
#include "port.h"


M_advmcv::M_advmcv(QWidget* parent)
  : Module(M_type_advmcv, 10, parent, tr("Advanced MCV"))
{
  QString qs;
  int l1, l2;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_ADVMCV_WIDTH, MODULE_ADVMCV_HEIGHT);
  port_gate_out = new Port(tr("Gate"), PORT_OUT, 0, this);
  port_note_out = new Port(tr("Freq"), PORT_OUT, 1, this);
  port_velocity_out = new Port(tr("Velocity"), PORT_OUT, 2, this);
  port_trig_out = new Port(tr("Trigger"), PORT_OUT, 3, this);
  port_aftertouch_out = new Port(tr("Aftertouch"), PORT_OUT, 4, this);
  port_pitchbend_out = new Port(tr("Pitchbend"), PORT_OUT, 5, this);
  for (l1 = 0; l1 < MODULE_ADVMCV_CONTROLLER_PORTS; l1++) {
    qs = tr("Controller %1").arg(l1);
    port_controller_out[l1] = new Port(qs, PORT_OUT, 6+l1, this);
  }

//  QStrList *channelNames = new QStrList(true);
//  channelNames->append("RESERVED FOR LATER USE");
//  for (l1 = 1; l1 < 17; l1++) {
//    qs.sprintf("RESERVED FOR LATER USE", l1);
//    channelNames->append(qs);
//  }
  channel = 0;
  pitch = 0;
  pitchbend = 0;
  aftertouch_cv = 0;
  pitchbend_cv = 0;
  for(l2 = 0; l2 < MODULE_ADVMCV_CONTROLLER_PORTS; l2++) {
    controller_cv[l2] = 0;
    controller_num[l2] = 0;
  }
//  configDialog->addComboBox(0, " ", &channel, channelNames->count(), channelNames);
  configDialog->addIntSlider(tr("&Note Offset"), pitch, -36, 36);
  configDialog->addSlider(tr("&Pitch"), pitchbend, -1, 1);
  for (l1 = 0; l1 < MODULE_ADVMCV_CONTROLLER_PORTS; l1++) {
    qs = tr("Controller &%1").arg(l1);
    configDialog->addIntSlider(qs, controller_num[l1], 0, 127);
  }
}

M_advmcv::~M_advmcv()
{
  synthdata->listM_advmcv.removeAll(this);
}

void M_advmcv::generateCycle()
{
  int l1, l3;
  unsigned int l2;
  float gate, velocity;

  for (l1 = 0; l1 < synthdata->poly; l1++) {
    gate = (synthdata->channel[l1] == channel - 1 || channel == 0) &&
	   synthdata->noteCounter[l1] < 1000000;
    freq[l1] = pitchbend + float(synthdata->notes[l1]+pitch-60) / 12.0;
    velocity = (float)synthdata->velocity[l1] / 127.0;
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      data[0][l1][l2] = gate;
      data[1][l1][l2] = freq[l1];
      data[2][l1][l2] = velocity;
      data[4][l1][l2] = aftertouch_cv;
      data[5][l1][l2] = pitchbend_cv;
      for (l3 = 0; l3 < MODULE_ADVMCV_CONTROLLER_PORTS; l3++)
	data[6+l3][l1][l2] = controller_cv[l3];
    }
    memset(data[3][l1], 0, synthdata->cyclesize * sizeof(float));
    //      data[3][l1][0] = trig[l1];
    data[3][l1][15] = synthdata->noteCounter[l1] == 0; // Added for interpolated input ports (e.g. m_vcenv.cpp)
  }
}

void M_advmcv::aftertouchEvent(int value)
{
  aftertouch_cv = (float)value / 127.0;
}

void M_advmcv::controllerEvent(int controlNum, int value)
{
  for(int l2 = 0; l2 < MODULE_ADVMCV_CONTROLLER_PORTS; l2++)
    if (controlNum == controller_num[l2])
        controller_cv[l2] = (float)value / 127.0;
}

void M_advmcv::pitchbendEvent(int value)
{
    pitchbend_cv = (float)value / 8192.0;
}
