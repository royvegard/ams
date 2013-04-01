#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "midicheckbox.h"
#include "midislider.h"
#include "m_stereomix.h"
#include "port.h"


class M_stereomix::MCableMute: public MidiControllable<float> {
  M_stereomix &m() {
    return reinterpret_cast<M_stereomix &>(module);
  }

  void maybeUnSolo() {
    if (value != 0) {
      unsigned ix = &value - m().mute;
      if (m().solo[ix] != 0) {
	m().solo[ix] = 0;
	m().solo_index = -1;
      }
    }
  }

protected:
  void updateMGCs(MidiGUIcomponent *sender) {
    MidiControllable<float>::updateMGCs(sender);
    if (sender)
      maybeUnSolo();

    unsigned ix = &value - m().mute;
    reinterpret_cast<MidiControllable<float> *>(m().midiControllables.at(4 * ix + 2))->
      MidiControllable<float>::updateMGCs(NULL);
  }

public:
  MCableMute(M_stereomix &module, const QString &name, float &value)
    : MidiControllable<float>(module, name, value, 0, 1) {}

  bool setMidiValueRT(int control14) {
    if (!MidiControllable<float>::setMidiValueRT(control14))
      return false;

    maybeUnSolo();
    return true;
  }
};

class M_stereomix::MCableSolo: public MidiControllable<float> {
  M_stereomix &m() {
    return reinterpret_cast<M_stereomix &>(module);
  }

protected:
  void updateMGCs(MidiGUIcomponent *sender) {
    if (sender)
      m().soloed(reinterpret_cast<MCableSolo&>(sender->mcAble).value);

    m().updateSolos(sender);
  }

public:
  MCableSolo(M_stereomix &module, const QString &name, float &value)
    : MidiControllable<float>(module, name, value, 0, 1) {}

  bool setMidiValueRT(int control14) {
    if (!MidiControllable<float>::setMidiValueRT(control14))
      return false;

    m().soloed(value);
    return true;
  }
};

void M_stereomix::soloed(float &value)
{
  if (value != 0) {
    if (solo_index < in_channels)
      solo[solo_index] = 0;

    solo_index = &value - solo;
    mute[solo_index] = 0;
  } else
    solo_index = -1;
}

void M_stereomix::updateSolos(MidiGUIcomponent *sender)
{
  for (unsigned l1 = 0; l1 < in_channels; l1++) {
    MidiGUIcomponent *mgcSolo = configDialog->midiCheckBoxList.at(2 * l1 + 1);

    static_cast<MidiControllable<float> &>(mgcSolo->mcAble).
      MidiControllable<float>::updateMGCs(mgcSolo == sender ? sender : NULL);
    MidiGUIcomponent *mgcMute = configDialog->midiCheckBoxList.at(2 * l1);

    static_cast<MidiControllable<float> &>(mgcMute->mcAble).
      MidiControllable<float>::updateMGCs(NULL);
  }
}


M_stereomix::M_stereomix(int p_in_channels, QWidget* parent)
  : Module(M_type_stereomix, 2, parent, tr("Stereo Mixer %1")
          .arg(p_in_channels))
  , solo_index(-1)
{
  QString qs;
  QHBoxLayout *hbox;

  in_channels = p_in_channels;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_STEREOMIX_WIDTH,
              MODULE_STEREOMIX_HEIGHT + 40 + 20 * in_channels);
  gain = 1.0;
  configDialog->addSlider(tr("M&aster Volume"), gain, 0, 10, true);
  ignore_check = false;
  for (unsigned l1 = 0; l1 < in_channels; l1++) {
    qs = tr("In %1").arg(l1);
    Port *audio_in_port = new Port(qs, PORT_IN, in_port_list.count(), this);
    in_port_list.append(audio_in_port);
    hbox = configDialog->addHBox();
    mute[l1] = 0.0;
    qs = tr("&Mute %1").arg(l1);
    configDialog->addCheckBox(*new MCableMute(*this, qs, mute[l1]), hbox);
    solo[l1] = 0.0;
    qs = tr("&Solo %1").arg(l1);
    configDialog->addCheckBox(*new MCableSolo(*this, qs, solo[l1]), hbox);
    mixer_gain[l1] = 1.0;
    qs = tr("&Volume %1").arg(l1);
    MidiSlider *slider = configDialog->addSlider(qs, mixer_gain[l1],
            0, 2, true, hbox);
    slider->setMinimumWidth(200);
    hbox->setStretchFactor(slider, 100);
    pan[l1] = 0.0;
    qs = tr("&Pan %1").arg(l1);
    slider = configDialog->addSlider(qs, pan[l1], -1, 1, false, hbox);
    slider->setMinimumWidth(150);
    hbox->setStretchFactor(slider, 100);
  }
  cv.out_off += cv.step * in_channels;
  for (unsigned l1 = 0; l1 < 2; l1++) {
    qs.sprintf("Out %d", l1);
    port_out[l1] = new Port(qs, PORT_OUT, l1, this);
  }
}

void M_stereomix::generateCycle()
{
  int l1;
  unsigned int l2;
  float mixgain[2];

  for (unsigned l3 = 0; l3 < in_channels; l3++)
    if ((solo_index >= in_channels || solo_index == l3) &&
	mute[l3] == 0)
      inData[l3] = in_port_list.at(l3)->getinputdata();
    else
      inData[l3] = synthdata->zeroModuleData;


  mixgain[0] = gain * (1.0 - pan[0]) * mixer_gain[0];
  mixgain[1] = gain * (1.0 + pan[0]) * mixer_gain[0];
  for (l1 = 0; l1 < synthdata->poly; l1++)
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      data[0][l1][l2] = mixgain[0] * inData[0][l1][l2];
      data[1][l1][l2] = mixgain[1] * inData[0][l1][l2];
    }

  for (unsigned l3 = 0; l3 < in_channels; l3++) {
    mixgain[0] = gain * (1.0 - pan[l3]) * mixer_gain[l3];
    mixgain[1] = gain * (1.0 + pan[l3]) * mixer_gain[l3];
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
	data[0][l1][l2] += mixgain[0] * inData[l3][l1][l2];
	data[1][l1][l2] += mixgain[1] * inData[l3][l1][l2];
      }
    }
  }
}

/*
void M_stereomix::soloToggled(bool) {

  int l1;
  QCheckBox *checkbox;

  if (ignore_check) return;
  checkbox = (QCheckBox *)sender();
  ignore_check = true;
  for (l1 = 0; l1 < configDialog->midiCheckBoxList.count() >> 1; l1++) {
    if (configDialog->midiCheckBoxList.at(2 * l1 + 1)->checkBox != checkbox)
      configDialog->midiCheckBoxList.at(2 * l1 + 1)->checkBox->setChecked(false);
    else {
      if (checkbox->isChecked()) {
        solo_index = l1;
      } else {
        solo_index = -1;
      }
      //!!configDialog->midiCheckBoxList.at(2 * l1)->updateCheck(false);
    }
  }
  ignore_check = false;
}

void M_stereomix::muteToggled(bool) {

  int l1;
  QCheckBox *checkbox;

  if (ignore_check) return;
  checkbox = (QCheckBox *)sender();
  ignore_check = true;
  for (l1 = 0; l1 < configDialog->midiCheckBoxList.count() >> 1; l1++) {
    if ((configDialog->midiCheckBoxList.at(2 * l1)->checkBox == checkbox)
        && configDialog->midiCheckBoxList.at(2 * l1)->checkBox->isChecked()
        && configDialog->midiCheckBoxList.at(2 * l1 + 1)->checkBox->isChecked()) {
      //!!configDialog->midiCheckBoxList.at(2 * l1 + 1)->updateCheck(false);
      solo_index = -1;
    }
  }
  ignore_check = false;
}
*/
