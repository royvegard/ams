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
#include <qtimer.h>
#include <QFileDialog>

#include "synthdata.h"
#include "midicontrollable.h"
#include "midipushbutton.h"
#include "m_wavout.h"
#include "module.h"
#include "port.h"


M_wavout::M_wavout(QWidget* parent)
  : Module(M_type_wavout, 0, parent, tr("WAV Out"))
{
  QString qs;
  QHBoxLayout *hbox1, *hbox2;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_WAVOUT_HEIGHT);
  gain = 0.5;
  mixer_gain[0] = 0.5;
  mixer_gain[1] = 0.5;
  agc = 1;
  doRecord = 0;
  port_in[0] = new Port("In 0", PORT_IN, 0, this);
  port_in[1] = new Port("In 1", PORT_IN, 1, this);

  configDialog->initTabWidget();
  QVBoxLayout *fileTab = configDialog->addVBoxTab(tr("&File"));
  QVBoxLayout *recordTab = configDialog->addVBoxTab(tr("&Record"));
  QVBoxLayout *gainTab = configDialog->addVBoxTab(tr("&Gain"));
  configDialog->addLineEdit("Fi&le:", fileTab);
  hbox1 = configDialog->addHBox(fileTab);
  configDialog->addLabel(tr("Time: 0:00:00        "), recordTab);
  configDialog->labelList.at(0)->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  MidiControllableDoOnce *do0 = configDialog->addPushButton(
          tr("&New file"), hbox1);
  MidiControllableDoOnce *do1 = configDialog->addPushButton(
          tr("&Overwrite current file"), hbox1);
  hbox2 = configDialog->addHBox(recordTab);
  MidiControllableDoOnce *do2 = configDialog->addPushButton(
          tr("Re&cord"), hbox2);
  MidiControllableDoOnce *do3 = configDialog->addPushButton(
          tr("&Stop"), hbox2);
  QObject::connect(do0, SIGNAL(triggered()), this, SLOT(openBrowser()));
  QObject::connect(do1, SIGNAL(triggered()), this, SLOT(createWav()));
  QObject::connect(do2, SIGNAL(triggered()), this, SLOT(recordClicked()));
  QObject::connect(do3, SIGNAL(triggered()), this, SLOT(stopClicked()));
  configDialog->midiPushButtonList.at(2)->pushButton->setEnabled(false);
  configDialog->midiPushButtonList.at(3)->pushButton->setEnabled(false);
  configDialog->addSlider(tr("G&ain"), gain, 0, 1, false, gainTab);
  configDialog->addSlider(tr("&Volume 1"), mixer_gain[0], 0, 1, false, gainTab);
  configDialog->addSlider(tr("V&olume 2"), mixer_gain[1], 0, 1, false, gainTab);
  QStringList agcNames;
  agcNames << tr("Disbled") << tr("Enabled");
  configDialog->addComboBox(tr("&Automatic Gain Control"), agc,
          agcNames, gainTab);

  wavDataSize = 0;
  wavdata = (char *)malloc(synthdata->periodsize * 4);
  memset(wavdata, 0, synthdata->periodsize * 4);
  floatdata = (float *)malloc(2 * synthdata->periodsize * sizeof(float));
  memset(floatdata, 0, 2 * synthdata->periodsize * sizeof(float));
  timer = new QTimer(this);
  QObject::connect(timer, SIGNAL(timeout()),
                   this, SLOT(timerProc()));
}

M_wavout::~M_wavout()
{
  free(wavdata);
  free(floatdata);
}

int M_wavout::setGain(float p_gain) {
  gain = p_gain;
  return(0);
}

float M_wavout::getGain() {
  return(gain);
}

void M_wavout::generateCycle()
{
  int l3;
  unsigned int l1, l2;
  float max, mixgain, wavgain, **indata;
  short s;

  memset(floatdata, 0, 2 * synthdata->cyclesize * sizeof(float));
  wavgain = 32767.0 / synthdata->poly;
  for (l1 = 0; l1 < 2; ++l1)
  {
      indata = port_in[l1]->getinputdata ();
      mixgain = gain * mixer_gain[l1];
      for (l2 = 0; l2 < synthdata->cyclesize; ++l2)
      {
        for (l3 = 0; l3 < synthdata->poly; ++l3) floatdata[2 * l2 + l1] += mixgain * indata[l3][l2];
      }
      if (agc)
      {
          max = 0;
          for (l2 = 0; l2 < synthdata->cyclesize; ++l2)
          {
              if (max < fabs(floatdata[2 * l2 + l1])) max = fabs(floatdata[2 * l2 + l1]);
          }
          if (max > 0.9)
          {
              max = 0.9 / max;
              for (l2 = 0; l2 < synthdata->cyclesize; ++l2) floatdata[2 * l2 + l1] *= max;
          }
      }
  }
  if (doRecord > 0)
  {
      for (l2 = 0; l2 < 2; ++l2)
      {
          for (l1 = 0; l1 < synthdata->cyclesize; ++l1)
          {
              s = (short)(wavgain * floatdata[2 * l1 + l2]);
              wavdata[4*l1+2*l2] = (unsigned char)s;
              wavdata[4*l1+2*l2+1] = s >> 8;
          }
      }
      wavfile.write(wavdata, synthdata->cyclesize * 4);
      wavDataSize += synthdata->cyclesize * 4;
  }
}

void M_wavout::recordToggled(bool on) {

  int tmpint;

  if (!on) {
    tmpint = wavDataSize + 36;
    outbuf[3] = tmpint >> 24;  // ByteRate
    outbuf[2] = (tmpint >> 16) - ((tmpint >> 24) << 8);
    outbuf[1] = (tmpint >> 8) - ((tmpint >> 16) << 8);
    outbuf[0] = (unsigned char)tmpint;
    wavfile.seek(4);
    wavfile.write(outbuf, 4);
    tmpint = wavDataSize;
    outbuf[3] = tmpint >> 24;  // ByteRate
    outbuf[2] = (tmpint >> 16) - ((tmpint >> 24) << 8);
    outbuf[1] = (tmpint >> 8) - ((tmpint >> 16) << 8);
    outbuf[0] = (unsigned char)tmpint;
    wavfile.seek(40);
    wavfile.write(outbuf, 4);
    wavfile.seek(wavfile.size());
    wavfile.flush();
  } else {
    timer->setSingleShot(true);
    timer->start(200);
  }
}

void M_wavout::recordClicked() {

  doRecord = true;
  recordToggled(true);
}


void M_wavout::stopClicked() {

  doRecord = false;
  recordToggled(false);
}

void M_wavout::createWav() {

  int tmpint;

  wavname = configDialog->lineEditList.at(0)->text();
  wavfile.setFileName(wavname);
  if (wavfile.open(QIODevice::WriteOnly)) {
    wavDataSize = 0;
    outbuf[0] = 0x52; outbuf[1] = 0x49; outbuf[2] = 0x46; outbuf[3] = 0x46; // "RIFF"
    wavfile.write(outbuf, 4);
    outbuf[0] = 0x24; outbuf[1] = 0x00; outbuf[2] = 0xff; outbuf[3] = 0x00; // ChunkSize
    wavfile.write(outbuf, 4);
    outbuf[0] = 0x57; outbuf[1] = 0x41; outbuf[2] = 0x56; outbuf[3] = 0x45; // "WAVE"
    wavfile.write(outbuf, 4);
    outbuf[0] = 0x66; outbuf[1] = 0x6d; outbuf[2] = 0x74; outbuf[3] = 0x20; // "fmt "
    wavfile.write(outbuf, 4);
    outbuf[0] = 0x10; outbuf[1] = 0x00; outbuf[2] = 0x00; outbuf[3] = 0x00; // Subchunk1Size
    wavfile.write(outbuf, 4);
    outbuf[0] = 0x01; outbuf[1] = 0x00; // AudioFormat
    wavfile.write(outbuf, 2);
    outbuf[0] = 0x02; outbuf[1] = 0x00; // NumChannels
    wavfile.write(outbuf, 2);
    outbuf[3] = synthdata->rate >> 24;  // SampleRate
    outbuf[2] = (synthdata->rate >> 16) - ((synthdata->rate >> 24) << 8);
    outbuf[1] = (synthdata->rate >> 8) - ((synthdata->rate >> 16) << 8);
    outbuf[0] = (unsigned char)synthdata->rate;
    wavfile.write(outbuf, 4);
    tmpint = synthdata->rate * 2 * 2;
    outbuf[3] = tmpint >> 24;  // ByteRate
    outbuf[2] = (tmpint >> 16) - ((tmpint >> 24) << 8);
    outbuf[1] = (tmpint >> 8) - ((tmpint >> 16) << 8);
    outbuf[0] = (unsigned char)tmpint;
    wavfile.write(outbuf, 4);
    outbuf[0] = 0x04; outbuf[1] = 0x00; // BlockAlign
    wavfile.write(outbuf, 2);
    outbuf[0] = 0x10; outbuf[1] = 0x00; // BitsPerSample
    wavfile.write(outbuf, 2);
    outbuf[0] = 0x64; outbuf[1] = 0x61; outbuf[2] = 0x74; outbuf[3] = 0x61; // "data"
    wavfile.write(outbuf, 4);
    outbuf[0] = 0x00; outbuf[1] = 0x00; outbuf[2] = 0xff; outbuf[3] = 0x00; // Subchunk2Size
    wavfile.write(outbuf, 4);
    configDialog->midiPushButtonList.at(2)->pushButton->setEnabled(true);
    configDialog->midiPushButtonList.at(3)->pushButton->setEnabled(true);
    configDialog->labelList.at(0)->setText("Time: 0:00:00        ");
  } else {
    configDialog->midiPushButtonList.at(2)->pushButton->setEnabled(false);
    configDialog->midiPushButtonList.at(3)->pushButton->setEnabled(false);
  }
}

void M_wavout::openBrowser() {

    char buf[2048];
    char * result;

    result = getcwd(buf, 2048);
    if (result == NULL) {
        qWarning("Error in getcwd((): %d", errno);
        return;
    }
    wavname = QFileDialog::getSaveFileName(this, tr("Choose Wave File"),
            buf, tr("WAV files (*.wav)"));
    if (!wavname.isEmpty()) {
        configDialog->lineEditList.at(0)->setText(wavname);
        createWav();
    }
}

void M_wavout::timerProc() {

  QString qs1, qs2, qs3;
  int seconds, minutes, displaySeconds;

  if (doRecord) {
    timer->setSingleShot(true);
    timer->start(200);
    seconds = (wavDataSize >> 2) / synthdata->rate;
    minutes = (seconds % 3600) / 60;
    displaySeconds = seconds % 60;
    qs1.sprintf("%d", seconds / 3600);
    if (minutes < 10) {
      qs2.sprintf("0%d", minutes);
    } else {
      qs2.sprintf("%d", minutes);
    }
    if (displaySeconds < 10) {
      qs3.sprintf("0%d", displaySeconds);
    } else {
      qs3.sprintf("%d", displaySeconds);
    }
    configDialog->labelList.at(0)->setText(
            tr("Time: %1:%2:%3 ").arg(qs1).arg(qs2).arg(qs3));
  }
}
