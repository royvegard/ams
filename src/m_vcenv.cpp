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
#include "m_vcenv.h"
#include "port.h"

M_vcenv::M_vcenv(QWidget* parent)
  : Module(M_type_vcenv, 1, parent, tr("VC Envelope"))
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCENV_WIDTH, MODULE_VCENV_HEIGHT);
  port_M_gate = new Port(tr("Gate"), PORT_IN, 0, this);
  port_M_retrigger = new Port(tr("Retrigger"), PORT_IN, 1, this);
  port_M_attack = new Port(tr("Attack"), PORT_IN, 2, this);
  port_M_decay = new Port(tr("Decay"), PORT_IN, 3, this);
  port_M_sustain = new Port(tr("Sustain"), PORT_IN, 4, this);
  port_M_release = new Port(tr("Release"), PORT_IN, 5, this);
  cv.out_off = 155;
  port_out = new Port(tr("Out"), PORT_OUT, 0, this);
  a0 = 0.01;
  d0 = 0.3;
  s0 = 0.7;
  r0 = 0.1;
  aGain = 0.0;
  dGain = 0.0;
  sGain = 0.0;
  rGain = 0.0;
  timeScale = 1;
  decayReleaseMode = 1;
  configDialog->addSlider(tr("&Attack Offset"), a0, 0, 1);
  configDialog->addSlider(tr("&Decay Offset"), d0, 0, 1);
  configDialog->addSlider(tr("&Sustain Offset"), s0, 0, 1);
  configDialog->addSlider(tr("&Release Offset"), r0, 0, 1);
  configDialog->addSlider(tr("A&ttack Gain"), aGain, -1, 1);
  configDialog->addSlider(tr("D&ecay Gain"), dGain, -1, 1);
  configDialog->addSlider(tr("S&ustain Gain"), sGain, -1, 1);
  configDialog->addSlider(tr("Re&lease Gain"), rGain, -1, 1);
  QStringList timeScaleNames;
  timeScaleNames << " 0.1 s";
  timeScaleNames << " 1.0 s";
  timeScaleNames << "10.0 s";
  configDialog->addComboBox(tr("T&ime Scale"), timeScale, timeScaleNames);
  QStringList decayReleaseModeNames;
  decayReleaseModeNames << tr("Linear");
  decayReleaseModeNames << tr("Exponential");
  configDialog->addComboBox(tr("De&cay/Release mode"), decayReleaseMode,
          decayReleaseModeNames);
}

void M_vcenv::generateCycle() {

  int l1, l2, l2_out;
  double ts, tsr, tsn, tmp, c, n, de;
  int k, len;

    gateData = port_M_gate->getinputdata ();
    retriggerData = port_M_retrigger->getinputdata ();
    attackData = port_M_attack->getinputdata ();
    decayData = port_M_decay->getinputdata ();
    sustainData = port_M_sustain->getinputdata ();
    releaseData = port_M_release->getinputdata ();

    switch(timeScale) {
      case 0: ts = 0.1;
              break;
      default:
      case 1: ts = 1.0;
              break;
      case 2: ts = 10.0;
              break;
    }
    tsr = 16.0 * ts / (double)synthdata->rate;
    tsn = ts * (double)synthdata->rate / 16.0;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
//      fprintf(stderr, "gate:%d retrigger:%d noteActive:%d state: %d\n", gate[l1], retrigger[l1], noteActive[l1], state[l1]);
      len = synthdata->cyclesize;
      old_e[l1] = e[l1];
      l2 = -1;
      l2_out = 0;
      do {
        k = (len > 24) ? 16 : len;
        l2 += k;
        len -= k;
        if (!gate[l1] && gateData[l1][l2] > 0.5) {
          gate[l1] = true;
          noteActive[l1] = true;
          state[l1] = 1;
        }
        if (gate[l1] && gateData[l1][l2] < 0.5) {
          gate[l1] = false;
          state[l1] = 4;
        }
        if (!retrigger[l1] && retriggerData[l1][l2] > 0.5) {
          retrigger[l1] = true;
          if (gate[l1]) {
            state[l1] = 1;
          }
        }
        if (retrigger[l1] && retriggerData[l1][l2] < 0.5) {
          retrigger[l1] = false;
        }
        switch (state[l1]) {
          case 0: e[l1] = 0;
                  break;
          case 1: e[l1] += ((tmp = a0 + aGain * attackData[l1][l2]) > 0.001) ? tsr / tmp : tsr / 0.001;
                  if (e[l1] >= 1.0) {
                    state[l1] = 2;
                    e[l1] = 1.0;
                  }
                  break;
          case 2: if (decayReleaseMode) {
                    n = tsn * (d0 + dGain * decayData[l1][l2]);
                    if (n < 1) n = 1;
                    c = 2.3 / n;
                    e[l1] *= exp(-c);
                  } else {
                    e[l1] -= ((tmp = d0 + dGain * decayData[l1][l2]) > 0.001) ? tsr / tmp : tsr / 0.001;
                  }
                  if (e[l1] <= s0 + sGain * sustainData[l1][l2] + 1e-20) {
                    state[l1] = 3;
                  } else {
                    break;
                  }
          case 3: e[l1] = s0 + sGain * sustainData[l1][l2];
                  break;
          case 4: if (decayReleaseMode) {
                    n = tsn * (r0 + rGain * releaseData[l1][l2]);
                    if (n < 1) n = 1;
                    c = 2.3 / n;
                    e[l1] *= exp(-c);
                  } else {
                    e[l1] -= ((tmp = r0 + rGain * releaseData[l1][l2]) > 0.001) ? tsr / tmp : tsr / 0.001;
                  }
                  if (e[l1] <= 1e-20) {
                    e[l1] = 0;
                    noteActive[l1] = false;
                  }
                  break;
          default: e[l1] = 0;
        }
        de = (e[l1] - old_e[l1]) / (double)k;
        while (k--) {
          old_e[l1] += de;
          data[0][l1][l2_out++] = old_e[l1];
        }
      } while(len);
    }
}

