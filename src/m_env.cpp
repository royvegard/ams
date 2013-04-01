#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <alsa/asoundlib.h>
#include "envelope.h"
#include "synthdata.h"
#include "midislider.h"
#include "m_env.h"
#include "port.h"
#include "midicontrollable.h"

M_env::M_env(QWidget* parent)
  : Module(M_type_env, 2, parent, tr("ENV"))
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_ENV_HEIGHT);
  delay = 0;
  attack = 0.05;
  hold = 0.02;
  decay = 0.1;
  sustain = 0.7;
  release = 0.05;
  timeScale = 1.0;
  port_gate = new Port(tr("Gate"), PORT_IN, 0, this);
  port_retrigger = new Port(tr("Retrigger"), PORT_IN, 1, this);
  cv.out_off = 75;
  port_gain_out = new Port(tr("Out"), PORT_OUT, 0, this);
  port_inverse_out = new Port(tr("Inverse Out"), PORT_OUT, 1, this);

  configDialog->initTabWidget();
  QVBoxLayout *adsrTab = configDialog->addVBoxTab(tr("&ADSR"));
  QVBoxLayout *delayTab = configDialog->addVBoxTab(
          tr("&Delay/Hold/Time Scale"));
  configDialog->addSlider(tr("De&lay"), delay, 0, 1, false, delayTab);
  configDialog->addSlider(tr("At&tack"), attack, 0, 1, false, adsrTab);
  configDialog->addSlider(tr("H&old"), hold, 0, 1, false, delayTab);
  configDialog->addSlider(tr("De&cay"), decay, 0, 1, false, adsrTab);
  configDialog->addSlider(tr("&Sustain"), sustain, 0, 1, false, adsrTab);
  configDialog->addSlider(tr("&Release"), release, 0, 1, false, adsrTab);
  configDialog->addSlider(tr("T&ime Scale"), timeScale, 0.1, 10, false, delayTab);

  configDialog->addEnvelope(*dynamic_cast<MidiControllableFloat *>(midiControllables.at(0)),
			    *dynamic_cast<MidiControllableFloat *>(midiControllables.at(1)),
			    *dynamic_cast<MidiControllableFloat *>(midiControllables.at(2)),
			    *dynamic_cast<MidiControllableFloat *>(midiControllables.at(3)),
			    *dynamic_cast<MidiControllableFloat *>(midiControllables.at(4)),
			    *dynamic_cast<MidiControllableFloat *>(midiControllables.at(5)));
}

void M_env::generateCycle()
{
  int l1, status;
  unsigned int l2;
  float tscale, de_attack, de_decay, de_release;
  float a, dl, dc, h;
  int idl, idla, idlah, idlahdc;


  gateData = port_gate->getinputdata();
  retriggerData = port_retrigger->getinputdata();

  tscale = timeScale * (float)synthdata->rate;
  de_attack = (attack > 0) ? 1.0 / (attack * tscale) : 0;
  de_decay = (decay > 0) ? (1.0 - sustain) / (decay * tscale) : 0;
  a = tscale * attack;
  dl = tscale * delay;
  idl = (int)dl;
  h = tscale * hold;
  dc = tscale * decay;
  idla = (int)(dl + a);
  idlah = idla + (int)h;
  if (idlah == idla)
    ++idlah;
  idlahdc = idlah + (int)dc;
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      if (!gate[l1] && (gateData[l1][l2] > 0.5)) {
	gate[l1] = true;
	if (e[l1] > 0) {
	  noteOnOfs[l1] = -ENVELOPE_RESPONSE;
	  de[l1] = e[l1] / (float)ENVELOPE_RESPONSE;
	} else
	  noteOnOfs[l1] = 0;
      }
      if (gate[l1] && (gateData[l1][l2] < 0.5)) {
	gate[l1] = false;
	e_noteOff[l1] = e[l1];
      }
      if (!retrigger[l1] && (retriggerData[l1][l2] > 0.5)) {
	retrigger[l1] = true;
	if (e[l1] > 0)
	  noteOnOfs[l1] = (de_attack > 0) ? (int)(e[l1] / de_attack) : 0;
	else
	  noteOnOfs[l1] = 0;
      }
      if (retrigger[l1] && (retriggerData[l1][l2] < 0.5))
	retrigger[l1] = false;

      if (gate[l1]) {
	status = 1;
	if (noteOnOfs[l1] < 0)
	  status = 0;
	if (noteOnOfs[l1] >= idl)
	  status = 2;
	if (noteOnOfs[l1] >= idla)
	  status = 3;
	if (noteOnOfs[l1] >= idlah)
	  status = 4;
	if (noteOnOfs[l1] >= idlahdc)
	  status = 5;
	switch (status) {
	case 0: e[l1] -= de[l1];
	  break;
	case 1: e[l1] = 0;
	  break;
	case 2: e[l1] += de_attack;
	  break;
	case 3: e[l1] = 1.0;
	  break;
	case 4: e[l1] -= de_decay;
	  break;
	case 5: e[l1] = sustain;
	  break;
	default: e[l1] = 0;
	  break;
	}
	if (e[l1] < 0)
	  e[l1] = 0;
	data[0][l1][l2] = e[l1];
	data[1][l1][l2] = -e[l1];
	noteOnOfs[l1]++;
      } else {                          // Release
	de_release = (release > 0) ? e_noteOff[l1] / (release * tscale) : 0;
	e[l1] -= de_release;

	if ((release == 0) || (e[l1] < 0))
	  e[l1] = 0;

	data[0][l1][l2] = e[l1];
	data[1][l1][l2] = -e[l1];
      }
    }
  }
}

