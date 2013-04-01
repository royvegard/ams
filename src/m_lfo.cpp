#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
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
#include "m_lfo.h"
#include "port.h"

/*

   1.0  |      +
        |     + +
        |    +   +
        |   +     +
        |  +       +
        | +         +
     t  |+-----------+-----------+--->
        |             +         +
        |              +       +
        |               +     +
        |                +   +
        |                 + +
  -1.0  |                  +

 State   |  0  |  1  |  2  |  3  |

*/

M_lfo::M_lfo(QWidget* parent)
  : Module(M_type_lfo, 6, parent, tr("LFO"))
  , wave_period((double)synthdata->rate / (16.0 * freq))
  , sa(-1)
  , dt(4.0 / wave_period)
{
  QString qs;
  long tm;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_LFO_WIDTH, MODULE_LFO_HEIGHT);
  freq = 5;
  phi0 = 0;

  port_M_trigger = new Port(tr("Reset"), PORT_IN, 0, this);
  cv.out_off = 55;
  port_sine = new Port(tr("Sine"), PORT_OUT, 0, this);
  port_tri = new Port(tr("Triangle"), PORT_OUT, 1, this);
  port_sawup = new Port(tr("Saw Up"), PORT_OUT, 2, this);
  port_sawdown = new Port(tr("Saw Down"), PORT_OUT, 3, this);
  port_rect = new Port(tr("Rectangle"), PORT_OUT, 4, this);
  port_sh = new Port(tr("S & H"), PORT_OUT, 5, this);

  configDialog->addSlider(tr("&Frequency"), freq, 0, 100, true);
  configDialog->addSlider(tr("&Phi0"), phi0, 0, 6.283);
  tm = time(NULL) % 1000000;
  srand(abs(tm - 10000 * (tm % 100)));
}

void M_lfo::generateCycle() {

  int l1, l2, k, len, phi0i, l2_out;
  double ldsi, ldsa, ldt, ldr, ldsh, dt0, dsa;

    float **triggerData = port_M_trigger->getinputdata();
   /*
    wave_period = (double)synthdata->rate / (16.0 * freq);
    dsa = 2.0 / wave_period;
    dt0 = 4.0 / wave_period;
    phi0i = (int)(phi0 / 6.283 * wave_period);
    */
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      len = synthdata->cyclesize;
      l2 = -1;
      l2_out = 0;
      do {
        k = (len > 24) ? 16 : len;
        l2 += k;

        /*update frequency change on every pass*/
        wave_period = (double)synthdata->rate / (16.0 * freq);
        dsa = 2.0 / wave_period;
        dt0 = 4.0 / wave_period;
        phi0i = (int)(phi0 / 6.283 * wave_period);


        if (!trigger[l1] && (triggerData[l1][l2] > 0.5)) {
          trigger[l1] = true;
          t[l1] = 0;
          state[l1] = 0;
          dt[l1] = dt0;
          r[l1] = -1;
          si[l1] = 0;
          sa[l1] = -1;
        }
        if (trigger[l1] && (triggerData[l1][l2] < 0.5)) {
          trigger[l1] = false;
        }
        if (t[l1] >= 1.0) {
          state[l1] = 1;
          dt[l1] = -dt0;
        } else if (t[l1] <= -1.0) {
          state[l1] = 3;
          dt[l1] = dt0;
        } else if ((state[l1] == 1) && (t[l1] < 0)) {
          state[l1] = 2;
          r[l1] = 1;
          sh[l1] = 2.0 * (double)rand() / (double)RAND_MAX - 1.0;
        } else if ((state[l1] == 3) && (t[l1] > 0)) {
          state[l1] = 0;
          r[l1] = -1;
          sh[l1] = 2.0 * (double)rand() / (double)RAND_MAX - 1.0;
          sa[l1] = -1;
        }
        si[l1] = (state[l1] < 2) ? t[l1] * (2.0 - t[l1]) : t[l1] * (2.0 + t[l1]);
        sa[l1] += dsa;
        t[l1] += dt[l1];

        if (state[l1] == 1 || state[l1] == 2)
            dt[l1] = -dt0;
        else
            dt[l1] = dt0;

        len -= k;
        ldsi = (si[l1] - old_si[l1]) / (double)k;
        ldsa = (sa[l1] - old_sa[l1]) / (double)k;
        ldsh = (sh[l1] - old_sh[l1]) / (double)k;
        ldt = (t[l1] - old_t[l1]) / (double)k;
        ldr = (r[l1] - old_r[l1]) / (double)k;
        while (k--) {
          old_si[l1] += ldsi;
          old_sa[l1] += ldsa;
          old_sh[l1] += ldsh;
          old_t[l1] += ldt;
          old_r[l1] += ldr;
          data[0][l1][l2_out] = old_si[l1];
          data[1][l1][l2_out] = old_t[l1];
          data[2][l1][l2_out] = old_sa[l1];
          data[3][l1][l2_out] = -old_sa[l1];
          data[4][l1][l2_out] = old_r[l1];
          data[5][l1][l2_out] = old_sh[l1];
          l2_out++;
        }
      } while(len);
    }
}

