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
#include "multi_envelope.h"
#include "midislider.h"
#include "m_advenv.h"
#include "port.h"
#include "midicontrollable.h"


M_advenv::M_advenv(QWidget* parent)
  : Module(M_type_advenv, 2, parent, tr("Advanced ENV"))
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_ADVENV_WIDTH,
          MODULE_ADVENV_HEIGHT);
  attack[0] = 0;
  attack[1] = 0.05;
  attack[2] = 0.5;
  attack[3] = 0.05;
  attack[4] = 1.0;
  attack[5] = 0.05;
  attack[6] = 0.9;
  attack[7] = 0.1;
  sustain = 0.7;
  release[0] = 0.05;
  release[1] = 0.5;
  release[2] = 0.05;
  release[3] = 0.2;
  release[4] = 0.05;
  timeScale = 1.0;
  port_gate = new Port(tr("Gate"), PORT_IN, 0, this);
  port_retrigger = new Port(tr("Retrigger"), PORT_IN, 1, this);
  cv.out_off = 75;
  port_gain_out = new Port("Out", PORT_OUT, 0, this);
  port_inverse_out = new Port(tr("Inverse Out"), PORT_OUT, 1, this);

  MultiEnvelope *multiEnv = configDialog->addMultiEnvelope(1,
          &timeScale, attack, &sustain, release);
  configDialog->initTabWidget();
  QVBoxLayout *sustainTab = configDialog->addVBoxTab(
          tr("Time &Scale/Sustain/Delay"));
  QVBoxLayout *attackTimeTab = configDialog->addVBoxTab(tr("&Attack Time"));
  QVBoxLayout *attackLevelTab = configDialog->addVBoxTab(tr("Attack &Level"));
  QVBoxLayout *releaseTimeTab = configDialog->addVBoxTab(tr("&Release Time"));
  QVBoxLayout *releaseLevelTab = configDialog->addVBoxTab(tr("Release Le&vel"));
  qs = tr("Ti&me Scale");
  configDialog->addSlider(qs, timeScale, 0.1, 10, false, sustainTab);
  qs = tr("S&ustain");
  configDialog->addSlider(qs, sustain, 0, 1, false, sustainTab);
  qs = tr("Dela&y");
  configDialog->addSlider(qs, attack[0], 0, 1, false, sustainTab);
  qs = tr("Attack Time &0");
  configDialog->addSlider(qs, attack[1], 0, 1, false, attackTimeTab);
  qs = tr("Attack Level &0");
  configDialog->addSlider(qs, attack[2], 0, 1, false, attackLevelTab);
  qs = tr("Attack Time &1");
  configDialog->addSlider(qs, attack[3], 0, 1, false, attackTimeTab);
  qs = tr("Attack Level &1");
  configDialog->addSlider(qs, attack[4], 0, 1, false, attackLevelTab);
  qs = tr("Attack Time &2");
  configDialog->addSlider(qs, attack[5], 0, 1, false, attackTimeTab);
  qs = tr("Attack Level &2");
  configDialog->addSlider(qs, attack[6], 0, 1, false, attackLevelTab);
  qs = tr("Attack Time &3");
  configDialog->addSlider(qs, attack[7], 0, 1, false, attackTimeTab);
  qs = tr("Release Time &0");
  configDialog->addSlider(qs, release[0], 0, 1, false, releaseTimeTab);
  qs = tr("Release Level &0");
  configDialog->addSlider(qs, release[1], 0, 1, false, releaseLevelTab);
  qs = tr("Release Time &1");
  configDialog->addSlider(qs, release[2], 0, 1, false, releaseTimeTab);
  qs = tr("Release Level &1");
  configDialog->addSlider(qs, release[3], 0, 1, false, releaseLevelTab);
  qs = tr("Release Time &2");
  configDialog->addSlider(qs, release[4], 0, 1, false, releaseTimeTab);
  multiEnv->listenTo(this);
}

void M_advenv::generateCycle() {

  int l1, status;
  unsigned int l2;
  float tscale, de_a[4], de_d[3];
  float t[8];

    gateData = port_gate->getinputdata();
    retriggerData = port_retrigger->getinputdata();

    tscale = timeScale * synthdata->rate;
    de_a[0] = (attack[1] > 0) ? attack[2] / (tscale * attack[1]) : 0;
    de_a[1] = (attack[3] > 0) ? (attack[4] - attack[2]) / (tscale * attack[3]) : 0;
    de_a[2] = (attack[5] > 0) ? (attack[6] - attack[4]) / (tscale * attack[5]) : 0;
    de_a[3] = (attack[7] > 0) ? (sustain - attack[6]) / (tscale * attack[7]) : 0;
    de_d[0] = (release[0] > 0) ? (release[1] - sustain) / (tscale * release[0]) : 0;
    de_d[1] = (release[2] > 0) ? (release[3] - release[1]) / (tscale * release[2]) : 0;
    de_d[2] = (release[4] > 0) ? - release[3] / (tscale * release[4]) : 0;
    t[0] = tscale * attack[0];
    t[1] = t[0] + tscale * attack[1];
    t[2] = t[1] + tscale * attack[3];
    t[3] = t[2] + tscale * attack[5];
    t[4] = t[3] + tscale * attack[7];
    t[5] = tscale * release[0];
    t[6] = t[5] + tscale * release[2];
    t[7] = t[6] + tscale * release[4];
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (!gate[l1] && (gateData[l1][l2] > 0.5)) {
          gate[l1] = true;
          noteActive[l1] = true;
          if (e[l1] > 0) {
            noteOnOfs[l1] = -ADVENVELOPE_RESPONSE;
            de[l1] = e[l1] / (float)ADVENVELOPE_RESPONSE;
          } else {
            noteOnOfs[l1] = 0;
          }
        }
        if (gate[l1] && (gateData[l1][l2] < 0.5)) {
          gate[l1] = false;
          noteOffOfs[l1] = 0;
          e_noteOff[l1] = e[l1];
          de_release[l1] = (release[0] > 0) ? (release[1] - e_noteOff[l1]) / (tscale * release[0]) : 0;
  //        fprintf(stderr, "de_release[%d]: %f, e_noteOff: %f\n", l1, de_release[l1], e_noteOff[l1]);
        }
        if (!retrigger[l1] && (retriggerData[l1][l2] > 0.5)) {
          retrigger[l1] = true;
          if (e[l1] > 0) {
            noteOnOfs[l1] = -ADVENVELOPE_RESPONSE;
            de[l1] = e[l1] / (float)ADVENVELOPE_RESPONSE;
          } else {
            noteOnOfs[l1] = 0;
          }
        }
        if (retrigger[l1] && (retriggerData[l1][l2] < 0.5)) {
          retrigger[l1] = false;
        }
        if (gate[l1]) {
            status = 1;
            if (noteOnOfs[l1] < 0) status = 0;
            if (noteOnOfs[l1] >= long(t[0])) status = 2;
            if (noteOnOfs[l1] >= long(t[1])) status = 3;
            if (noteOnOfs[l1] >= long(t[2])) status = 4;
            if (noteOnOfs[l1] >= long(t[3])) status = 5;
            if (noteOnOfs[l1] >= long(t[4])) status = 6;
            switch (status) {
              case 0: e[l1] -= de[l1];
                      break;
              case 1: e[l1] = 0;
                      break;
              case 2: e[l1] += de_a[0];
                      break;
              case 3: e[l1] += de_a[1];
                      break;
              case 4: e[l1] += de_a[2];
                      break;
              case 5: e[l1] += de_a[3];
                      break;
              case 6: e[l1] = sustain;
                      break;
              default: e[l1] = 0;
                       break;
            }
            if (e[l1] < 0) e[l1] = 0;
            data[0][l1][l2] = e[l1];
            data[1][l1][l2] = -e[l1];
            noteOnOfs[l1]++;
        } else {                          // Release
          if (noteActive[l1]) {
            status = 1;
            if (noteOffOfs[l1] < 0) status = 0;
            if (noteOffOfs[l1] >= long(t[5])) status = 2;
            if (noteOffOfs[l1] >= long(t[6])) status = 3;
            if (noteOffOfs[l1] >= long(t[7])) status = 4;
            switch (status) {
              case 0: e[l1] = 0;
                      break;
              case 1: e[l1] += de_release[l1];
                      break;
              case 2: e[l1] += de_d[1];
                      break;
              case 3: e[l1] += de_d[2];
                      break;
              case 4: e[l1] = 0;
                      break;
              default: e[l1] = 0;
                     break;
            }
            if (e[l1] < 0) {
//              fprintf(stderr, "status: %d e[%d] < 0: %f\n", status, l1, e[l1]);
              e[l1] = 0;
            }
            noteOffOfs[l1]++;
            if (noteOffOfs[l1] >= int(t[7])) {
//              fprintf(stderr, "noteOffOfs[%d] = %d >= t[7] = %f; e[%d] = %f\n", l1, noteOffOfs[l1], t[7], l1, e[l1]);
              noteActive[l1] = false;
            }
          }
          data[0][l1][l2] = e[l1];
          data[1][l1][l2] = -e[l1];
        }
      }
    }
}

