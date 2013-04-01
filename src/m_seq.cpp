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
#include "intmidislider.h"
#include "m_seq.h"
#include "port.h"

M_seq::M_seq(int p_seqLen, QWidget* parent)
  : Module(M_type_seq, 4, parent, tr("SEQ"))
  , tickFrames(0)
  , tickFramesRemain(0)
{
  QString qs;
  char str[1024];
  int l1;
  QVBoxLayout *pitchTab[4], *gateTab[4], *velocityTab[4];

  seqLen = p_seqLen;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_SEQ_HEIGHT);
  port_trigger = new Port(tr("Trigger"), PORT_IN, 0, this);
  cv.out_off = 55;
  port_gate_out = new Port(tr("Gate"), PORT_OUT, 0, this);
  port_note_out = new Port(tr("Freq"), PORT_OUT, 1, this);
  port_velocity_out = new Port(tr("Velocity"), PORT_OUT, 2, this);
  port_trigger_out = new Port(tr("Trigger"), PORT_OUT, 3, this);

  configDialog->initTabWidget();
  QVBoxLayout *generalTab = configDialog->addVBoxTab(
          tr("Pitch offset/&Tempo/Gate time"));
  int seqLen_8 = (seqLen + 7) / 8;
  for (l1 = 0; l1 < seqLen_8; l1++) {
    sprintf(str, "&Pitch %d", l1);
    pitchTab[l1] = configDialog->addVBoxTab(str);
    sprintf(str, "&Gate %d", l1);
    gateTab[l1] = configDialog->addVBoxTab(str);
    sprintf(str, "&Velocity %d", l1);
    velocityTab[l1] = configDialog->addVBoxTab(str);
  }
  seq_gate = 0;
  seq_freq = 0;
  seq_velocity = 0;
  seq_pos = 0;
  tick = 0;
  osc = 0;
  trigger = false;
  triggerCount = 0;
  triggerOut = false;
  bpm = 120;
  pitch_ofs = 32;
  configDialog->addIntSlider("&Pitch offset", pitch_ofs, 0, 63, generalTab);
  configDialog->addIntSlider("&Beats per minute", bpm, 3, 300, generalTab);
  QStringList noteLenNames;
  noteLenNames << "1" << "3/4" << "1/2" << "1/4";
  configDialog->addComboBox("&Gate time", note_len, noteLenNames, generalTab);
  for (l1 = 0; l1 < seqLen; l1++) {
    pitch[l1] = 31;
    velocity[l1] = 63;
    gate[l1] = 0;
  }
  for (l1 = 0; l1 < seqLen; l1++) {
    sprintf(str, "Gate &%d", l1);
    configDialog->addCheckBox(str, gate[l1], gateTab[l1 / 8]);
    sprintf(str, "Pitch &%d", l1);
    configDialog->addIntSlider(str, pitch[l1], 0, 64, pitchTab[l1 / 8]);
    sprintf(str, "Velocity &%d", l1);
    configDialog->addIntSlider(str, velocity[l1], 0, 127, velocityTab[l1 / 8]);
  }
}


void M_seq::generateCycle() {

  int l1;
  unsigned int l2;

    triggerData = port_trigger->getinputdata ();

    if (triggerCount) {
      triggerCount--;
    } else {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (!trigger && (triggerData[0][l2] > 0.5)) {
          trigger = true;
          triggerCount = 32;
        }
      }
    }

    tickFrames -= synthdata->cyclesize;
    if (tickFrames <= 0)
      nextStep();

    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      data[3][0][l2] = (triggerOut) ? 1.0 : 0;
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
        if (l1 == osc) {
          data[0][l1][l2] = seq_gate;
          data[1][l1][l2] = seq_freq;
          data[2][l1][l2] = seq_velocity;
        } else {
          data[0][l1][l2] = 0;
        }
      }
    }
}

void M_seq::nextStep()
{
  int len, l2;
  long noteCount;
  int minuteFrames = synthdata->rate * 60 + tickFramesRemain;
  tickFrames += minuteFrames / (bpm << 4);
  tickFramesRemain = minuteFrames % (bpm << 4);

  if (trigger) {
    tick = 0;
    seq_pos = 0;
    trigger = false;
  }
  if (seq_pos == 0)
    triggerOut = true;
  else
    triggerOut = false;

  len = 4 - note_len;
  if (tick == 0) {
    seq_freq = float(pitch[seq_pos] + pitch_ofs) / 12.0;

// Search for next free voice
    osc = -1;
    noteCount = 0;
    for (l2 = 0; l2 < synthdata->poly; l2++) {
      if (synthdata->noteCounter[l2] > noteCount) {
        noteCount = synthdata->noteCounter[l2];
        osc = l2;
      }
    }

    seq_gate = (osc < 0) ? 0 : (float)gate[seq_pos];
    seq_velocity = float(velocity[seq_pos]) / 127.0;
    seq_pos++;
    if (seq_pos >= seqLen)
      seq_pos = 0;

  }
  if (tick == len)
    seq_gate = 0;

  tick++;
  if (tick >= 4)
    tick = 0;

}
