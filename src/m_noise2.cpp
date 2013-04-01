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
#include "m_noise2.h"
#include "port.h"


M_noise2::M_noise2(QWidget* parent)
  : Module(M_type_noise2, 1, parent, tr("Noise2"))
{
  QString qs;
  int l2;
  long t;

  rate = 5;
  level = 0.5;
  count = 0;
  NoiseType=WHITE;
  randmax = 2.0f / (float)RAND_MAX;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_NOISE2_HEIGHT);
  port_white = new Port("Out", PORT_OUT, 0, this);

  QStringList noiseNames;
  noiseNames << tr("White") << tr("Random") << tr("Pink") << tr("Pulsetrain");
  configDialog->addComboBox(tr("&Noise Type"), NoiseType, noiseNames);
  configDialog->addSlider(tr("Random &Rate"), rate, 0, 100);
  configDialog->addSlider(tr("Random &Level"), level, 0, 1);
  r = 0;
  for (l2 = 0; l2 < 3; ++l2) {
    buf[l2] = 0;
  }
  t = time(NULL) % 1000000;
  srand(abs(t - 10000 * (t % 100)));
}

void M_noise2::generateCycle() {

  int l1;
  unsigned int l2;
  float white_noise;

     switch (NoiseType)
     {
      case WHITE:
      {
      	
    	for (l2 = 0; l2 < synthdata->cyclesize; ++l2) {
      		white_noise = rand() * randmax - 1.0f;
      		for (l1 = 0; l1 < synthdata->poly; ++l1) {
        		data[0][l1][l2] = white_noise;
      		}
      	}
      }		
      break;
      case RAND:
      {
        unsigned int random_rate = (unsigned int)(5000.0 * (double)rate + 100.0);
    	for (l2 = 0; l2 < synthdata->cyclesize; ++l2) {
      		count++;
      		if (count > random_rate) {
        		count = 0;
        		r = level * rand() * randmax - 1.0f;
      		}
      		for (l1 = 0; l1 < synthdata->poly; ++l1) {
        		data[0][l1][l2] = r;
      		}
      	}
       }		
      break;
      case PINK:
      {
      	
    		for (l2 = 0; l2 < synthdata->cyclesize; ++l2) {
      			white_noise = rand() * randmax - 1.0f;

		      buf[0] = 0.99765f * buf[0] + white_noise * 0.099046f;
		      buf[1] = 0.963f * buf[1] + white_noise * 0.2965164f;
			buf[2] = 0.57f * buf[2] + white_noise * 1.0526913f;
      			for (l1 = 0; l1 < synthdata->poly; ++l1) {
      				data[0][0][l2] = buf[0] + buf[1] + buf[2] + white_noise * 0.1848f;
        			data[0][l1][l2] = data[0][0][l2];
			}
      		}
      }
      break;
      case PULSETRAIN:
      {
          float px;
          px = 1.00 - (1.0 / pow (10, ((100-rate)/20)));
          for (l2 = 0; l2 < synthdata->cyclesize; ++l2) {
              white_noise = (2 * rand() / ((float)RAND_MAX));
              for (l1 = 0; l1 < synthdata->poly; ++l1) {
                  data[0][l1][l2] = -level;
                  if (white_noise > px)
                      data[0][l1][l2] = level;
              }
          }
      }		
      break;
     }

}

