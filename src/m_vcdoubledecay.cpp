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
#include "m_vcdoubledecay.h"
#include "port.h"

M_vcdoubledecay::M_vcdoubledecay(QWidget* parent)
  : Module(M_type_vcdoubledecay, 1, parent, tr("VC Double Decay"))
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCDOUBLEDECAY_WIDTH, MODULE_VCDOUBLEDECAY_HEIGHT);
  port_M_gate = new Port(tr("Gate"), PORT_IN, 0, this);
  port_M_retrigger = new Port(tr("Retrigger"), PORT_IN, 1, this);
  port_M_attack = new Port(tr("Attack"), PORT_IN, 2, this);
  port_M_decay = new Port(tr("Decay"), PORT_IN, 3, this);
  port_M_sustain = new Port(tr("Sustain"), PORT_IN, 4, this);
  port_M_release = new Port(tr("Release"), PORT_IN, 5, this);
  port_M_ratio = new Port(tr("Ratio"), PORT_IN, 6, this);
  cv.out_off = 175;
  port_out = new Port(tr("Out"), PORT_OUT, 0, this);
  a0 = 0;
  d0 = 0;
  s0 = 0;
  rl0 = 0;
  r0 = 0.5;
  aGain = 1.0;
  dGain = 1.0;
  sGain = 1.0;
  rlGain = 1.0;
  rGain = 1.0;
  configDialog->addSlider(tr("&Attack Offset"), a0, -8, 8);
  configDialog->addSlider(tr("&Decay Offset"), d0, -8, 8);
  configDialog->addSlider(tr("&Sustain Offset"), s0, 0, 1);
  configDialog->addSlider(tr("&Ratio Offset"), r0, 0, 1);
  configDialog->addSlider(tr("R&elease Offset"), rl0, -8, 8);
  configDialog->addSlider(tr("A&ttack Gain"), aGain, -8, 8);
  configDialog->addSlider(tr("D&ecay Gain"), dGain, -8, 8);
  configDialog->addSlider(tr("S&ustain Gain"), sGain, 0, 1);
  configDialog->addSlider(tr("Rat&io Gain"), rGain, 0, 1);
  configDialog->addSlider(tr("Re&lease Gain"), rlGain, -8, 8);
}

void M_vcdoubledecay::generateCycle() {

  int l1, l2, k, len, l2_out;
  double ts, tsr, tsn, tmp, c1, c2, n1, n, c, astep, de, de2, ds;

    gateData = port_M_gate->getinputdata ();
    retriggerData = port_M_retrigger->getinputdata ();
    attackData = port_M_attack->getinputdata ();
    decayData = port_M_decay->getinputdata ();
    sustainData = port_M_sustain->getinputdata ();
    releaseData = port_M_sustain->getinputdata ();
    ratioData = port_M_ratio->getinputdata ();

    ts = 1.0;
    tsr = 16.0 * ts / (double)synthdata->rate;
    tsn = ts * (double)synthdata->rate / 16.0;
    for (l1 = 0; l1 < synthdata->poly; l1++) {
//      fprintf(stderr, "gate:%d retrigger:%d noteActive:%d state: %d\n", gate[l1], retrigger[l1], noteActive[l1], state[l1]);
      len = synthdata->cyclesize;
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
          state[l1] = 3;
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
        s[l1] = s0 + sGain * sustainData[l1][l2];
        switch (state[l1]) {
          case 0: e[l1] = 0;
                  e2[l1] = 0;
                  break;
          case 1: astep = ((tmp = synthdata->exp2_table(a0 + aGain * attackData[l1][l2])) > 0.001) ? tsr / tmp : tsr / 0.001;
                  e[l1] += astep;
                  e2[l1] += astep;
                  if (e[l1] >= 1.0) {
                    state[l1] = 2;
                    e[l1] = 1.0;
                  }
                  if (e2[l1] >= 1.0) {
                    e2[l1] = 1.0;
                  }
                  break;
          case 2: n1 = tsn * synthdata->exp2_table(d0 + dGain * decayData[l1][l2]);
                  if (n1 < 1) n1 = 1;
                  c1 = 2.3 / n1;
                  c2 = c1 * (r0 + rGain * ratioData[l1][l2]);
                  if (c2 < 0) c2 = 0;
                  e[l1] *= exp(-c1);
                  if (e[l1] <= 1e-20) e[l1] = 0;
                  e2[l1] *= exp(-c2);
                  if (e2[l1] <= 1e-20) e2[l1] = 0;
                  break;
          case 3: n = tsn * synthdata->exp2_table(rl0 + rlGain * releaseData[l1][l2]);
                  if (n < 1) n = 1;
                  c = 2.3 / n;
                  e[l1] *= exp(-c);
                  if (e[l1] <= 1e-20) e[l1] = 0;
                  e2[l1] *= exp(-c);
                  if (e2[l1] < 1e-20)  {
                    e[l1] = 0;
                    e2[l1] = 0;
                    noteActive[l1] = false;
                  }
                  break;
          default: e[l1] = 0;
                   e2[l1] = 0;
                   data[0][l1][l2] = e[l1];
        }

        de = (e[l1] - old_e[l1]) / (double)k;
        de2 = (e2[l1] - old_e2[l1]) / (double)k;
        ds = (s[l1] - old_s[l1]) / (double)k;
        while (k--) {
          old_e[l1] += de;
          old_e2[l1] += de2;
          old_s[l1] += ds;
          data[0][l1][l2_out++] = (1.0 - old_s[l1]) * old_e[l1] + old_s[l1] * old_e2[l1];
        }
      } while (len);
    }
}

