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
#include "m_mix.h"
#include "port.h"

M_mix::M_mix(int p_in_channels, QWidget* parent)
  : Module(M_type_mix, 1, parent, tr("Mixer %1").arg(p_in_channels))
{
  QString qs;
  int l1;

  in_channels = p_in_channels;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_MIX_WIDTH,
              MODULE_MIX_HEIGHT + 20 + 20 * in_channels);
  gain = 1.0;
  configDialog->addSlider(tr("&Gain"), gain, 0, 2);
  for (l1 = 0; l1 < in_channels; l1++) {
    qs = tr("In %1").arg(l1);
    Port *audio_in_port = new Port(qs, PORT_IN, in_port_list.count(), this);
    in_port_list.append(audio_in_port);
    mixer_gain[l1] = 1.0;
    qs = tr("Volume &%1").arg(l1);
    configDialog->addSlider(qs, mixer_gain[l1], 0, 2);
  }
  cv.out_off += cv.step * in_channels;
  port_out = new Port(tr("Out"), PORT_OUT, 0, this);
}

void M_mix::generateCycle() {

  int l1, l3;
  unsigned int l2;
  float mixgain;

    for (l3 = 0; l3 < in_port_list.count(); l3++) inData [l3] = in_port_list.at(l3)->getinputdata();

    mixgain = gain * mixer_gain[0];
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        data[0][l1][l2] = mixgain * inData[0][l1][l2];
      }
    }
    for (l3 = 1; l3 < in_port_list.count(); l3++) {
      mixgain = gain * mixer_gain[l3];
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[0][l1][l2] += mixgain * inData[l3][l1][l2];
        }
      }
    }
}

