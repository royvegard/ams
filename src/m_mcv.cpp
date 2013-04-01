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
#include "midicombobox.h"
#include "synthdata.h"
#include "m_mcv.h"
#include "port.h"

M_mcv::M_mcv(QWidget* parent)
  : Module(M_type_mcv, 4, parent, tr("MCV"))
{
  QString qs;
  int l1;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_MCV_HEIGHT);
  port_gate_out = new Port(tr("Gate"), PORT_OUT, 0, this);
  port_note_out = new Port(tr("Freq"), PORT_OUT, 1, this);
  port_velocity_out = new Port(tr("Velocity"), PORT_OUT, 2, this);
  port_trig_out = new Port(tr("Trigger"), PORT_OUT, 3, this);

  QStringList channelNames;
  channelNames << "RESERVED FOR LATER USE";
  for (l1 = 1; l1 < 17; l1++) {
    qs.sprintf("RESERVED FOR LATER USE");
    channelNames << qs;
  }
  channel = 0;
  pitch = 0;
  pitchbend = 0;
  configDialog->addComboBox(" ", channel, channelNames)->hide();
  configDialog->addIntSlider(tr("&Note offset"), pitch, -36, 36);
  configDialog->addSlider(tr("&Pitch"), pitchbend, -1, 1);
}

void M_mcv::generateCycle()
{
  int l1;
  unsigned int l2;
  float gate, velocity;

  for (l1 = 0; l1 < synthdata->poly; l1++) {
    // do legato in mono mode
    gate = (synthdata->channel[l1] == channel - 1 || channel == 0) &&
	   synthdata->noteCounter[l1] < 1000000;
    freq[l1] = pitchbend + float(synthdata->notes[l1]+pitch-60) / 12.0;

    //      if (freq[l1] < 0) freq[l1] = 0;
    velocity = (float)synthdata->velocity[l1] / 127.0;
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      data[0][l1][l2] = gate;
      data[1][l1][l2] = freq[l1];
      data[2][l1][l2] = velocity;
    }
    memset(data[3][l1], 0, synthdata->cyclesize * sizeof(float));
    //      data[3][l1][0] = trig[l1];
    data[3][l1][15] = synthdata->noteCounter[l1] == 0; // Added for interpolated input ports (e.g. m_vcenv.cpp)
  }
}

