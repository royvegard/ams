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
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_midiout.h"
#include "module.h"
#include "port.h"


M_midiout::M_midiout(QWidget* parent)
  : Module(M_type_midiout, 0, parent, tr("Midi Out"))
{
  QString qs;
  int l1, l2;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_MIDIOUT_HEIGHT);
  mixer_gain[0] = 1.0;
  mixer_gain[1] = 1.0;
  midiMode = 0;
  offset[0] = 0;
  offset[1] = 0;
  controller[0] = 24;
  controller[1] = 25;
  triggerLevel = 0.5;
  port_in[0] = new Port(tr("In 0"), PORT_IN, 0, this);
  port_in[1] = new Port(tr("In 1"), PORT_IN, 1, this);
  port_M_trigger = new Port(tr("Trigger"), PORT_IN, 2, this);

  configDialog->initTabWidget();
  QVBoxLayout *gainTab = configDialog->addVBoxTab(
          tr("&Gain/Offset/Trigger level"));
  QVBoxLayout *midiTab = configDialog->addVBoxTab(tr("MIDI &Settings"));
  QStringList channelNames;
  for (l1 = 0; l1 < 16; l1++) {
    qs.sprintf("%4d", l1);
    channelNames << qs;
  }
  channel = 0;
  configDialog->addComboBox(tr("MIDI &channel"), channel, channelNames, midiTab);
  configDialog->addSlider(tr("G&ain 0"), mixer_gain[0], 0, 10, false, gainTab);
  configDialog->addSlider(tr("Ga&in 1"), mixer_gain[1], 0, 10, false, gainTab);
  configDialog->addIntSlider(tr("&Offset 0"), offset[0], 0, 127, gainTab);
  configDialog->addIntSlider(tr("O&ffset 1"), offset[1], 0, 127, gainTab);
  QStringList midiNames;
  midiNames <<
    tr("In 0/1: Controller") <<
    tr("In 0: Controller In 1: Pitchbend") <<
    tr("In 0/1: Note") <<
    tr("In 0: Note, In 1: Velocity");
  configDialog->addComboBox(tr("MIDI &event type"), midiMode, midiNames, midiTab);
  configDialog->addIntSlider(tr("C&ontroller 0"), controller[0], 0, 127, midiTab);
  configDialog->addIntSlider(tr("Co&ntroller 1"), controller[1], 0, 127, midiTab);
  configDialog->addSlider(tr("Tri&gger level"), triggerLevel, 0, 10, false, gainTab);
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    for (l2 = 0; l2 < 2; l2++) {
      triggeredNote[l2][l1] = 0;
      lastmididata[l2][l1] = 0;
    }
  }
}

M_midiout::~M_midiout()
{
}

void M_midiout::generateCycle()
{
  if (!synthdata->seq_handle)
    return;

  int l1, l3, mididata, velocitydata;
  unsigned int l2;
  snd_seq_event_t ev;

  triggerData = port_M_trigger->getinputdata();
  for (l1 = 0; l1 < 2; l1++) inData [l1] = port_in [l1]->getinputdata();

  switch (midiMode) {
    case 0:
      if (triggerData == synthdata->zeroModuleData) {
        for (l1 = 0; l1 < 2; l1++) {
          if (mixer_gain[l1] > 0.01) {
            for (l3 = 0; l3 < synthdata->poly; l3++) {
              for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
                mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.8) - 1000;
                if (mididata < 0) mididata = 0;
                else if (mididata > 127) mididata = 127;
                if (mididata != lastmididata[l1][l3]) {
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_CONTROLLER;
                  ev.data.control.channel = channel;
                  ev.data.control.param = controller[l1];
                  ev.data.control.value = mididata;
                  triggeredNote[l1][l3] = 0;
                  lastmididata[l1][l3] = mididata;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              }
            }
          }
        }
      } else {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (!trigger[l3] && (triggerData[l3][l2] > triggerLevel)) {
              trigger[l3] = true;
              for (l1 = 0; l1 < 2; l1++) {
                if (mixer_gain[l1] > 0.01) {
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_CONTROLLER;
                  mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.8) - 1000;
                  if (mididata < 0) mididata = 0;
                  else if (mididata > 127) mididata = 127;
                  ev.data.control.channel = channel;
                  ev.data.control.param = controller[l1];
                  ev.data.control.value = mididata;
                  triggeredNote[l1][l3] = 0;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              }
            } else {
              if (trigger[l3] && (triggerData[l3][l2] < triggerLevel)) {
                trigger[l3] = false;
              }
            }
          }
        }
      }
      break;
    case 1:
      if (triggerData == synthdata->zeroModuleData ) {
        for (l1 = 0; l1 < 2; l1++) {
          if (mixer_gain[l1] > 0.01) {
            for (l3 = 0; l3 < synthdata->poly; l3++) {
              for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
               if (l1) {
                 mididata = (int)(128.0 * offset[l1] + (int)(mixer_gain[l1] * inData[l1][l3][l2] * 16384.0) - 8192.0);
                 if (mididata < -8191) mididata = -8191;
                 else if (mididata > 8191) mididata = 8191;
                } else {
                  mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.8) - 1000;
                  if (mididata < 0) mididata = 0;
                  else if (mididata > 127) mididata = 127;
                }
                if (mididata != lastmididata[l1][l3]) {
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  if (l1) {
                    ev.type = SND_SEQ_EVENT_PITCHBEND;
                    ev.data.control.param = 0;
                  } else {
                    ev.type = SND_SEQ_EVENT_CONTROLLER;
                    ev.data.control.param = controller[l1];
                  }
                  ev.data.control.channel = channel;
                  ev.data.control.value = mididata;
                  triggeredNote[l1][l3] = 0;
                  lastmididata[l1][l3] = mididata;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              }
            }
          }
        }
      } else {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (!trigger[l3] && (triggerData[l3][l2] > triggerLevel)) {
              trigger[l3] = true;
              for (l1 = 0; l1 < 2; l1++) {
                if (mixer_gain[l1] > 0.01) {
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  if (l1) {
                    ev.type = SND_SEQ_EVENT_PITCHBEND;
                    ev.data.control.param = 0;
                    mididata = (int)(128.0 * offset[l1] + (int)(mixer_gain[l1] * inData[l1][l3][l2] * 16384.0) - 8192.0);
                    if (mididata < -8191) mididata = -8191;
                    else if (mididata > 8191) mididata = 8191;
                  } else {
                    ev.type = SND_SEQ_EVENT_CONTROLLER;
                    ev.data.control.param = controller[l1];
                    mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.8) - 1000;
                    if (mididata < 0) mididata = 0;
                    else if (mididata > 127) mididata = 127;
                  }
                  ev.data.control.channel = channel;
                  ev.data.control.value = mididata;
                  triggeredNote[l1][l3] = 0;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              }
            } else {
              if (trigger[l3] && (triggerData[l3][l2] < triggerLevel)) {
                trigger[l3] = false;
              }
            }
          }
        }
      }
      break;
    case 2:
      if (triggerData != synthdata->zeroModuleData ) {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (!trigger[l3] && (triggerData[l3][l2] > triggerLevel)) {
              trigger[l3] = true;
              for (l1 = 0; l1 < 2; l1++) {
                if (mixer_gain[l1] > 0.01) {
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_NOTEON;
                  ev.data.control.channel = channel;
                  mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.0) - 1000;
                  if (mididata < 0) mididata = 0;
                  else if (mididata > 127) mididata = 127;
                  ev.data.note.note = mididata;
                  ev.data.note.velocity = 127;
                  triggeredNote[l1][l3] = mididata;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              }
            } else {
              if (trigger[l3] && (triggerData[l3][l2] < triggerLevel)) {
                trigger[l3] = false;
                for (l1 = 0; l1 < 2; l1++) {
                  if (mixer_gain[l1] > 0.01) {
                    snd_seq_ev_clear(&ev);
                    snd_seq_ev_set_subs(&ev);
                    snd_seq_ev_set_direct(&ev);
                    ev.type = SND_SEQ_EVENT_NOTEOFF;
                    ev.data.control.channel = channel;
                    ev.data.note.velocity = 0;
                    ev.data.note.note = triggeredNote[l1][l3];
                    snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                    snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                  }
                }
              }
            }
          }
        }
      } else {
        for (l1 = 0; l1 < 2; l1++) {
          if (mixer_gain[l1] > 0.01) {
            for (l3 = 0; l3 < synthdata->poly; l3++) {
              for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
                mididata = offset[l1] + (int)(1000.0 + mixer_gain[l1] * inData[l1][l3][l2] * 12.0) - 1000;
                if (mididata < 0) mididata = 0;
                else if (mididata > 127) mididata = 127;
                if (mididata != lastmididata[l1][l3]) {
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_NOTEOFF;
                  ev.data.control.channel = channel;
                  ev.data.note.note = triggeredNote[l1][l3];
                  ev.data.note.velocity = 0;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_NOTEON;
                  ev.data.control.channel = channel;
                  ev.data.note.note = mididata;
                  ev.data.note.velocity = 127;
                  triggeredNote[l1][l3] = mididata;
                  lastmididata[l1][l3] = mididata;
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[l1]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              }
            }
          }
        }
      }
      break;
    case 3:
      if (triggerData != synthdata->zeroModuleData ) {
        for (l3 = 0; l3 < synthdata->poly; l3++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (!trigger[l3] && (triggerData[l3][l2] > triggerLevel)) {
              trigger[l3] = true;
              if (mixer_gain[0] > 0.01) {
                snd_seq_ev_clear(&ev);
                snd_seq_ev_set_subs(&ev);
                snd_seq_ev_set_direct(&ev);
                ev.type = SND_SEQ_EVENT_NOTEON;
                ev.data.control.channel = channel;
                mididata = offset[0] + (int)(1000.0 + mixer_gain[0] * inData[0][l3][l2] * 12.0) - 1000;
                velocitydata = offset[1] + (int)(1000.0 + mixer_gain[1] * inData[1][l3][l2] * 12.8) - 1000;
                if (mididata < 0) mididata = 0;
                else if (mididata > 127) mididata = 127;
                if (velocitydata < 0) velocitydata = 0;
                else if (velocitydata > 127) velocitydata = 127;
                ev.data.note.note = mididata;
                ev.data.note.velocity = velocitydata;
                triggeredNote[0][l3] = mididata;
                snd_seq_ev_set_source(&ev, synthdata->midi_out_port[0]);
                snd_seq_event_output_direct(synthdata->seq_handle, &ev);
              }
            } else {
              if (trigger[l3] && (triggerData[l3][l2] < triggerLevel)) {
                trigger[l3] = false;
                if (mixer_gain[0] > 0.01) {
                  snd_seq_ev_clear(&ev);
                  snd_seq_ev_set_subs(&ev);
                  snd_seq_ev_set_direct(&ev);
                  ev.type = SND_SEQ_EVENT_NOTEOFF;
                  ev.data.control.channel = channel;
                  ev.data.note.velocity = 0;
                  ev.data.note.note = triggeredNote[0][l3];
                  snd_seq_ev_set_source(&ev, synthdata->midi_out_port[0]);
                  snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                }
              }
            }
          }
        }
      } else {
        if (mixer_gain[0] > 0.01) {
          for (l3 = 0; l3 < synthdata->poly; l3++) {
            for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
              mididata = offset[0] + (int)(1000.0 + mixer_gain[0] * inData[0][l3][l2] * 12.0) - 1000;
              if (mididata < 0) mididata = 0;
              else if (mididata > 127) mididata = 127;
              if (mididata != lastmididata[0][l3]) {
                snd_seq_ev_clear(&ev);
                snd_seq_ev_set_subs(&ev);
                snd_seq_ev_set_direct(&ev);
                ev.type = SND_SEQ_EVENT_NOTEOFF;
                ev.data.control.channel = channel;
                ev.data.note.note = triggeredNote[0][l3];
                ev.data.note.velocity = 0;
                snd_seq_ev_set_source(&ev, synthdata->midi_out_port[0]);
                snd_seq_event_output_direct(synthdata->seq_handle, &ev);
                snd_seq_ev_clear(&ev);
                snd_seq_ev_set_subs(&ev);
                snd_seq_ev_set_direct(&ev);
                ev.type = SND_SEQ_EVENT_NOTEON;
                ev.data.control.channel = channel;
                velocitydata = offset[1] + (int)(1000.0 + mixer_gain[1] * inData[1][l3][l2] * 12.8) - 1000;
                if (velocitydata < 0) velocitydata = 0;
                else if (velocitydata > 127) velocitydata = 127;
                ev.data.note.note = mididata;
                ev.data.note.velocity = velocitydata;
                triggeredNote[0][l3] = mididata;
                lastmididata[0][l3] = mididata;
                lastmididata[1][l3] = velocitydata;
                snd_seq_ev_set_source(&ev, synthdata->midi_out_port[0]);
                snd_seq_event_output_direct(synthdata->seq_handle, &ev);
              }
            }
          }
        }
      }
      break;
  }
}

