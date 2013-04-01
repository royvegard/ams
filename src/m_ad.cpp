#include <math.h>
#include <QString>
#include <alsa/asoundlib.h>

#include "synthdata.h"
#include "midicontrollable.h"
#include "midicombobox.h"
#include "m_ad.h"
#include "midipushbutton.h"
#include "port.h"


M_ad::M_ad(int outCount, QWidget* parent)
  : Module(M_type_ad, outCount, parent,
	   tr("Analog Driver %1 Out").arg(outCount))
{
  QString qs;
  int l1, l2;
  QHBoxLayout *tuneBox, *detuneBox[2], *driftBox[2];

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_AD_WIDTH,
              MODULE_AD_HEIGHT + 20 + 20 * outCount);
  qs = tr("CV In");
  port_in = new Port(qs, PORT_IN, 0, this);
  cv.out_off = 55;
  for (l1 = 0; l1 < outCount; l1++) {
    qs = tr("CV Out %1").arg(l1);
    port_out[l1] = new Port(qs, PORT_OUT, l1, this);
    for (l2 = 0; l2 < MAXPOLY; l2++) {
      drift_a[l1][l2] = 0.4 * (double)random() / (double)RAND_MAX - 0.2;
      drift_c[l1][l2] = 0.4 * (double)random() / (double)RAND_MAX - 0.2;
    }
  }
  for (l2 = 0; l2 < MAXPOLY; l2++) {
    detune_a[l2] = 0.4 * (double)random() / (double)RAND_MAX - 0.2;
    detune_c[l2] = 0.4 * (double)random() / (double)RAND_MAX - 0.2;
  }
  detune_amp = 0.005;
  detune_mod = 0.01;
  detune_rate = 1;
  drift_amp = 0.005;
  drift_mod = 0.01;
  drift_rate = 3;
  detuneCount = 0;
  driftCount = 0;
  bypass = 0;
  configDialog->initTabWidget();
  QVBoxLayout *paramTab = configDialog->addVBoxTab(tr("&Parameter"));

  configDialog->addSlider(tr("Detune &Amplitude"), detune_amp, 0,
          0.084, true, paramTab);
  configDialog->addSlider(tr("Detune &Modulation"), detune_mod, 0.01,
          1, true, paramTab);
  configDialog->addSlider(tr("Detune &Rate"), detune_rate, 0.01,
          10, true, paramTab);
  configDialog->addSlider(tr("Drift &Amplitude"), drift_amp, 0,
          0.084, true, paramTab);
  configDialog->addSlider(tr("Drift &Modulation"), drift_mod, 0.01,
          1, true, paramTab);
  configDialog->addSlider(tr("Dri&ft Rate"), drift_rate, 0.01,
          10, true, paramTab);

  QVBoxLayout *displayTab = configDialog->addVBoxTab(tr("&Display"));
  for (l1 = 0 ; l1 < 2; l1++) {
    voice[l1] = (synthdata->poly > 1) ? l1 : 0;
    QStringList voiceNames;
    for (l2 = 0; l2 < synthdata->poly; l2++) {
      qs.sprintf("%d", l2);
      voiceNames << qs;
    }
    configDialog->addComboBox(tr("&Voice"), voice[l1], voiceNames, displayTab);
    QObject::connect(configDialog->midiComboBoxList.at(l1)->comboBox,
                     SIGNAL(highlighted(int)), this, SLOT(updateVoices(int)));
    qs = tr("Detune %1").arg(l1);
    configDialog->addLabel(qs, displayTab);
    detuneBox[l1] = configDialog->addHBox(displayTab);
    configDialog->addLabel(" 0.000 ", detuneBox[l1]);
    qs = tr("Drift %1").arg(l1);
    configDialog->addLabel(qs, displayTab);
    driftBox[l1] = configDialog->addHBox(displayTab);
    for (l2 = 0; l2 < outCount; l2++) {
      configDialog->addLabel(" 0.000 ", driftBox[l1]);
    }
  }

  tuneBox = configDialog->addHBox();
  MidiControllableDoOnce * do0 = configDialog->addPushButton(
          tr("Auto&tune"), tuneBox);
  QObject::connect(do0, SIGNAL(triggered()), this, SLOT(autoTune()));
  configDialog->addCheckBox(tr("B&ypass"), bypass, tuneBox);

  timer = new QTimer(this);
  QObject::connect(timer, SIGNAL(timeout()),
                   this, SLOT(timerProc()));
  timer->start(1000);
}

void M_ad::generateCycle() {

  int l1, l3, l4, l5;
  unsigned int l2;
  float dta, dra, rdt, rdr;
  double qdt, qdr;


    inData = port_in->getinputdata();

    dta = detune_amp;
    dra = drift_amp;
    rdt = detune_mod / (float)synthdata->rate;
    rdr = drift_mod / (float)synthdata->rate;
    qdt = (double)outPortCount * (double)synthdata->poly * (double)synthdata->rate / (detune_rate + 1e-3);
    qdr = (double)outPortCount * (double)synthdata->poly * (double)synthdata->rate / (detune_rate + 1e-3);
    if (bypass) {
      for (l3 = 0; l3 < outPortCount; l3++) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            data[l3][l1][l2] = inData[l1][l2];
          }
        }
      }
    } else {
      for (l3 = 0; l3 < outPortCount; l3++) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            data[l3][l1][l2] = inData[l1][l2]
                             + dta * detune_a[l1]
                             + dra * drift_a[l3][l1];
            detune_a[l1] += rdt * detune_c[l1];
            if (detune_a[l1] > 1.0) {
              detune_a[l1] = 1.0;
              detune_c[l1] = -1.0;
            } else if (detune_a[l1] < -1.0) {
              detune_a[l1] = -1.0;
              detune_c[l1] = 1.0;
            }
            if (drift_a[l3][l1] > 1.0) {
              drift_a[l3][l1] = 1.0;
              drift_c[l3][l1] = -1.0;
            } else if (drift_a[l3][l1] < -1.0) {
              drift_a[l3][l1] = -1.0;
              drift_c[l3][l1] = 1.0;
            }
            drift_a[l3][l1] += rdr * drift_c[l3][l1];
            detuneCount++;
            driftCount++;
            if (detuneCount > qdt) {
              detuneCount = 0;
              for (l4 = 0; l4 < synthdata->poly; l4++) {
                detune_c[l4] = 2.0 * (double)random() / (double)RAND_MAX - 1.0;
              }
//              fprintf(stderr, "%5.3f\n", detune_c[0]);
            }
            if (driftCount > qdr) {
              driftCount = 0;
              for (l5 = 0; l5 < outPortCount; l5++) {
                for (l4 = 0; l4 < synthdata->poly; l4++) {
                  drift_c[l5][l4] = 2.0 * (double)random() / (double)RAND_MAX - 1.0;
                }
              }
            }
          }
        }
      }
    }
}

void M_ad::updateVoices(int) {

  QString qs;

  qs = tr("Detune %1").arg(voice[0]);
  configDialog->labelList.at(0)->setText(qs);
  qs = tr("Drift %1").arg(voice[0]);
  configDialog->labelList.at(2)->setText(qs);
  qs = tr("Detune %1").arg(voice[1]);
  configDialog->labelList.at(3 + outPortCount)->setText(qs);
  qs = tr("Drift %1").arg(voice[1]);
  configDialog->labelList.at(5 + outPortCount)->setText(qs);
}

void M_ad::timerProc() {

  QString qs;
  int l1;

  qs.sprintf(" %2.3f", detune_a[voice[0]]);
  configDialog->labelList.at(1)->setText(qs);
  for (l1 = 0; l1 < outPortCount; l1++) {
    qs.sprintf(" %2.3f", drift_a[l1][voice[0]]);
    configDialog->labelList.at(3 + l1)->setText(qs);
  }
  qs.sprintf(" %2.3f", detune_a[voice[1]]);
  configDialog->labelList.at(4 + outPortCount)->setText(qs);
  for (l1 = 0; l1 < outPortCount; l1++) {
    qs.sprintf(" %2.3f", drift_a[l1][voice[1]]);
    configDialog->labelList.at(6 + outPortCount + l1)->setText(qs);
  }
}

void M_ad::autoTune() {

  int l1, l2;

  for (l1 = 0; l1 < synthdata->poly; l1++) {
    detune_a[l1] = 0;
    for (l2 = 0; l2 < outPortCount; l2++) {
      drift_a[l2][l1] = 0;
    }
  }
}

