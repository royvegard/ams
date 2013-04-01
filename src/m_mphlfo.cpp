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
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_mphlfo.h"
#include "port.h"

M_mphlfo::M_mphlfo(QWidget* parent)
  : Module(M_type_mphlfo, 16, parent, tr("Multiphase LFO")) {

  QString qs;
  int l1;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_MPHLFO_WIDTH,
              MODULE_MPHLFO_HEIGHT);
  for (l1 = 0; l1 < 16; l1++) {
    if (l1 < 8) {
      qs.sprintf("Saw Out %4d", l1 * 45);
    } else {
      qs.sprintf("Tri Out %4d", (l1-8) * 45);
    }
    port_out[l1] = new Port(qs, PORT_OUT, l1, this);
  }
  freq = 0.1;
  gain_saw = 1.0;
  gain_tri = 1.0;
  tri = 0;
  saw = 0;
  state = 0;
  mode = 0;
  d_tri = 4.0 * freq / (double)synthdata->rate;
  d_saw = 0.5 * d_tri;
  configDialog->addSlider(tr("&Frequency (Hz)"), freq, 0.01, 20, true);
  configDialog->addSlider(tr("Gain &Saw"), gain_saw, 0.01, 5, true);
  configDialog->addSlider(tr("Gain &Triangle"), gain_tri, 0.01, 5, true);
  QStringList modeNames;
  modeNames << tr("Saw Up") << tr("Saw Down")
      << tr("Saw Up (0..135) / Saw Down (180..315)");
  configDialog->addComboBox(tr("Saw &Mode"), mode, modeNames);
}

void M_mphlfo::generateCycle() {

  int l1;
  unsigned int l2;
  double tri45, tri90, tri135, saw45, saw90, saw135, saw180, saw225, saw270, saw315;
  double sign_saw1, sign_saw2;

    d_saw = 0;
    d_tri = ((state > 1) && (state < 6)) ? -4.0 * freq / (double)synthdata->rate
                                         :  4.0 * freq / (double)synthdata->rate;
    d_saw = 0.5 * fabs(d_tri);
    switch(mode) {
      case 1:
        sign_saw1 = -1.0;
        sign_saw2 = -1.0;
        break;
      case 2:
        sign_saw1 = 1.0;
        sign_saw2 = -1.0;
        break;
      case 0: // fall through
      default:
        sign_saw1 = 1.0;
        sign_saw2 = 1.0;
        break;
    }
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      tri += d_tri;
      saw += d_saw;
      switch(state) {
        case 1:
          if (tri > 1.0) {
            state++;
            tri = 1.0;
            d_tri = -4.0 * freq / (double)synthdata->rate;
            d_saw = -0.5 * d_tri;
          }
          tri45 = 1.5 - tri;
          tri90 = 1.0 - tri;
          tri135 = 0.5 - tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw;
          saw135 = -1.25 + saw;
          saw180 = -1.0 + saw;
          saw225 = -0.75 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          break;
        case 2:
          if (tri < 0.5) {
            state++;
          }
          tri45 = tri - 0.5;
          tri90 = tri - 1.0;
          tri135 = -1.5 + tri;
          saw45 = 0.25 + saw;
          saw90 = -1.5 + saw;
          saw135 = -1.25 + saw;
          saw180 = -1.0 + saw;
          saw225 = -0.75 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          break;
        case 3:
          tri45 = tri - 0.5;
          tri90 = tri - 1.0;
          tri135 = -0.5 - tri;
          saw45 = -1.75 + saw;
          saw90 = -1.5 + saw;
          saw135 = -1.25 + saw;
          saw180 = -1.0 + saw;
          saw225 = -0.75 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          if (tri < 0.0) {
            state++;
            saw = -1.0;
          }
          break;
        case 4:
          if (tri < -0.5) {
            state++;
          }
          tri45 = tri - 0.5;
          tri90 = -1.0 - tri;
          tri135 = -0.5 - tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw;
          saw135 = 0.75 + saw;
          saw180 = 1.0 + saw;
          saw225 = 1.25 + saw;
          saw270 = 1.5 + saw;
          saw315 = 1.75 + saw;
          break;
        case 5:
          if (tri < -1.0) {
            tri = -1.0;
            d_tri = 4.0 * freq / (double)synthdata->rate;
            d_saw = 0.5 * d_tri;
            state++;
          }
          tri45 = - 1.5 - tri;
          tri90 = -1.0 - tri;
          tri135 = -0.5 - tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw;
          saw135 = 0.75 + saw;
          saw180 = 1.0 + saw;
          saw225 = 1.25 + saw;
          saw270 = 1.5 + saw;
          saw315 = -0.25 + saw;
          break;
        case 6:
          if (tri > -0.5) {
            state++;
          }
          tri45 = 0.5 + tri;
          tri90 = 1.0 + tri;
          tri135 = 1.5 + tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw;
          saw135 = 0.75 + saw;
          saw180 = 1.0 + saw;
          saw225 = 1.25 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          break;
        case 7:
          if (tri > 0.0) {
            state = 0;
          }
          tri45 = 0.5 + tri;
          tri90 = 1.0 + tri;
          tri135 = 0.5 - tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw;
          saw135 = 0.75 + saw;
          saw180 = 1.0 + saw;
          saw225 = -0.75 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          break;
        case 0: // fall through
        default:
          if (tri > 0.5) {
            state++;
          }
          tri45 = 0.5 + tri;
          tri90 = 1.0 - tri;
          tri135 = 0.5 - tri;
          saw45 = 0.25 + saw;
          saw90 = 0.5 + saw;
          saw135 = 0.75 + saw;
          saw180 = -1.0 + saw;
          saw225 = -0.75 + saw;
          saw270 = -0.5 + saw;
          saw315 = -0.25 + saw;
          break;
      }

      o[0] = gain_saw * (1.0 + sign_saw1 * saw45);
      o[1] = gain_saw * (1.0 + sign_saw1 * saw);
      o[2] = gain_saw * (1.0 + sign_saw1 * saw315);
      o[3] = gain_saw * (1.0 + sign_saw1 * saw270);
      o[4] = gain_saw * (1.0 + sign_saw2 * saw225);
      o[5] = gain_saw * (1.0 + sign_saw2 * saw180);
      o[6] = gain_saw * (1.0 + sign_saw2 * saw135);
      o[7] = gain_saw * (1.0 + sign_saw2 * saw90);
      o[8] = gain_tri * (1.0 + tri135);
      o[9] = gain_tri * (1.0 + tri90);
      o[10] = gain_tri * (1.0 + tri45);
      o[11] = gain_tri * (1.0 + tri);
      o[12] = gain_tri * (1.0 - tri135);
      o[13] = gain_tri * (1.0 - tri90);
      o[14] = gain_tri * (1.0 - tri45);
      o[15] = gain_tri * (1.0 - tri);

      for (l1 = 0; l1 < synthdata->poly; l1++) {
        data[0][l1][l2] = o[0];
        data[1][l1][l2] = o[1];
        data[2][l1][l2] = o[2];
        data[3][l1][l2] = o[3];
        data[4][l1][l2] = o[4];
        data[5][l1][l2] = o[5];
        data[6][l1][l2] = o[6];
        data[7][l1][l2] = o[7];
        data[8][l1][l2] = o[8];
        data[9][l1][l2] = o[9];
        data[10][l1][l2] = o[10];
        data[11][l1][l2] = o[11];
        data[12][l1][l2] = o[12];
        data[13][l1][l2] = o[13];
        data[14][l1][l2] = o[14];
        data[15][l1][l2] = o[15];
      }
    }
}

