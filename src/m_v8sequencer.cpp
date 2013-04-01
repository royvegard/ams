/*
  V8 sequencer - derived from m_vcdelay.cpp

  Copyright (C) 2011 Bill Yerazunis <yerazunis@yahoo.com>

  This file is part of ams.

  ams is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2 as
  published by the Free Software Foundation.

  ams is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ams.  If not, see <http://www.gnu.org/licenses/>.
*/

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
#include "midicheckbox.h"
#include "midislider.h"
#include "m_v8sequencer.h"
#include "port.h"

M_v8sequencer::M_v8sequencer(QWidget* parent)
  : Module(M_type_v8sequencer, 18, parent, tr("V8 Seq"))
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_V8SEQUENCER_HEIGHT);

  port_M_in0 = new Port (tr("In 0"), PORT_IN, 0, this);
  port_M_in1 = new Port (tr("In 1"), PORT_IN, 1, this);
  port_M_in2 = new Port (tr("In 2"), PORT_IN, 2, this);
  port_M_in3 = new Port (tr("In 3"), PORT_IN, 3, this);
  port_M_seqstate = new Port(tr("A State"), PORT_IN, 4, this);
  port_M_seqtrig = new Port(tr("A Trig"), PORT_IN, 5, this);
  port_M_step = new Port(tr("Step"), PORT_IN, 6, this);
  port_M_direction = new Port(tr("Direction"), PORT_IN, 7, this);
  port_M_forward = new Port(tr("+ 1"), PORT_IN, 8, this);
  port_M_backward = new Port(tr("- 1"), PORT_IN, 9, this);
  port_M_s0 = new Port(tr("S 0"), PORT_IN, 10, this);
  port_M_s1 = new Port(tr("S 1"), PORT_IN, 11, this);
  port_M_s2 = new Port(tr("S 2"), PORT_IN, 12, this);
  port_M_s3 = new Port(tr("S 3"), PORT_IN, 13, this);
  port_M_s4 = new Port(tr("S 4"), PORT_IN, 14, this);
  port_M_s5 = new Port(tr("S 5"), PORT_IN, 15, this);
  port_M_s6 = new Port(tr("S 6"), PORT_IN, 16, this);
  port_M_s7 = new Port(tr("S 7"), PORT_IN, 17, this);

  cv.out_off = 35;
  port_out0 = new Port(tr("Out 0"), PORT_OUT, 0, this);
  port_out1 = new Port(tr("Out 1"), PORT_OUT, 1, this);
  port_out2 = new Port(tr("Out 2"), PORT_OUT, 2, this);
  port_out3 = new Port(tr("Out 3"), PORT_OUT, 3, this);
  port_seqstate = new Port(tr("A State"), PORT_OUT, 4, this);
  port_seqtrig = new Port(tr("A Trig"), PORT_OUT, 5, this);
  port_aux0 = new Port(tr("Aux 0"), PORT_OUT, 6, this);
  port_aux1 = new Port(tr("Aux 1"), PORT_OUT, 7, this);
  port_gate = new Port(tr("Gate"), PORT_OUT, 8, this);
  port_pulse = new Port(tr("Pulse"), PORT_OUT, 9, this);
  port_state0 = new Port(tr("S 0"), PORT_OUT, 10, this);
  port_state1 = new Port(tr("S 1"), PORT_OUT, 11, this);
  port_state2 = new Port(tr("S 2"), PORT_OUT, 12, this);
  port_state3 = new Port(tr("S 3"), PORT_OUT, 13, this);
  port_state4 = new Port(tr("S 4"), PORT_OUT, 14, this);
  port_state5 = new Port(tr("S 5"), PORT_OUT, 15, this);
  port_state6 = new Port(tr("S 6"), PORT_OUT, 16, this);
  port_state7 = new Port(tr("S 7"), PORT_OUT, 17, this);

  //    Now the joy of setting up a config screen.
  //    There is one line per sequencer state
  //
  //
  QHBoxLayout *hbox;
  int i;
  configDialog->setAddStretch(1);
  hbox = configDialog->addHBox();
  QStringList InternalExternal;
  InternalExternal << tr("Use step/direction and forward/backward");
  InternalExternal << tr("Use external analog input on StateIn input");
  int_ext_mode = 0;
  configDialog->addComboBox (tr("Internal vs. External State Control"),
				int_ext_mode, InternalExternal, hbox);
  maxstate = 8;
  configDialog->addIntSlider (tr("Max States"), maxstate, 1, 64, hbox);
  myfirststate = 0;
  configDialog->addIntSlider (tr("This Seq S0 ID"), myfirststate, 0, 56, hbox);

  hbox = configDialog->addHBox();
  QStringList AuxSrcs;
  AuxSrcs << tr("Out0");
  AuxSrcs << tr("Out1");
  AuxSrcs << tr("Out2");
  AuxSrcs << tr("Out3");
  //  AuxSrcs << tr("Out2 ignore hold");
  //  AuxSrcs << tr("Out3 ignore hold");
  QStringList AuxOffsets;
  AuxOffsets << tr("0");
  AuxOffsets << tr("1");
  AuxOffsets << tr("2");
  AuxOffsets << tr("3");
  AuxOffsets << tr("4");
  AuxOffsets << tr("5");
  AuxOffsets << tr("6");
  AuxOffsets << tr("7");
  QStringList AuxBehaviors;
  AuxBehaviors << tr("0,1,2,3,4,5,6,7");
  AuxBehaviors << tr("0,1,2,3,0,1,2,3");
  AuxBehaviors << tr("0,2,4,6,0,2,4,6");
  AuxBehaviors << tr("0,1,0,1,0,1,0,1");
  AuxBehaviors << tr("0,2,0,2,0,2,0,2");
  AuxBehaviors << tr("0,3,0,3,0,3,0,3");
  AuxBehaviors << tr("7,6,5,4,3,2,1,0");
  AuxBehaviors << tr("3,2,1,0,3,2,1,0");
  AuxBehaviors << tr("6,4,2,0,6,4,2,0");
  aux0src = 0;
  configDialog->addComboBox(tr("Aux 0 source"), aux0src, AuxSrcs, hbox);
  aux0offset = 0;
  configDialog->addComboBox(tr("Aux 0 offset"), aux0offset, AuxOffsets, hbox);
  aux0map = 0;
  configDialog->addComboBox(tr("Aux 0 mapping"), aux0map, AuxBehaviors, hbox);
  aux1src = 1;
  configDialog->addComboBox(tr("Aux 1 source"), aux1src, AuxSrcs, hbox);
  aux1offset = 0;
  configDialog->addComboBox(tr("Aux 1 offset"), aux1offset, AuxOffsets, hbox);
  aux1map = 0;
  configDialog->addComboBox(tr("Aux 1 mapping"), aux1map, AuxBehaviors, hbox);

  MidiSlider *slider;
  QStringList StateActions;
  StateActions << tr("Normal");
  StateActions << tr("Skip");
  StateActions << tr("Pause");
  StateActions << tr("Reset");
  for (i = 0; i < MODULE_V8SEQUENCER_STATES; i++)
    {
      hbox = configDialog->addHBox();
      qs = tr ("Step S%1").arg(i);
      stateaction[i] = 0;\
      configDialog->addComboBox(qs, stateaction[i], StateActions, hbox);
      qs = tr ("Out 0 S%1").arg(i);
      out0[i] = 0.0;
      slider = configDialog->addSlider
	(qs, out0[i], -2, 2, false, hbox);
      slider->setMinimumWidth (151);
      qs = tr ("Out 1 S%1").arg(i);
      out1[i] = 0.0;
      slider = configDialog->addSlider (qs, out1[i], -2, 2, false, hbox);
      slider->setMinimumWidth (151);
      qs = tr ("WrEna 2 S%1").arg(i);
      configDialog->addCheckBox (qs, sticky2[i], hbox);
      sticky2[i] = 0.0;
      qs = tr ("Out 2 S%1").arg(i);
      out2[i] = 0.0;
      slider = configDialog->addSlider (qs, out2[i], -8, 8, false, hbox);
      slider->setMinimumWidth (151);
      qs = tr ("WrEna 3 S%1").arg(i);
      configDialog->addCheckBox (qs, sticky3[i], hbox);
      sticky3[i] = 0.0;
      qs = tr ("Out 3 S%1").arg(i);
      out3[i] = 0.0;
      slider = configDialog->addSlider (qs, out3[i], -8, 8, false, hbox);
      slider->setMinimumWidth (151);
    };
  int l1;
  for (l1 = 0; l1 < synthdata->poly; l1++)
    state[l1] = 0;
}

M_v8sequencer::~M_v8sequencer() {

}

void M_v8sequencer::generateCycle() {

  //   For referencing into the output data array "data[l0][l1][l2]"
  //  int l0 // l0 is the output port number
  int l1;    //  l1 is for cyclesize
  unsigned int l2;  //  l2 is for polyphony

  int statepause;  //   are we in a "pause" state?

  //   Note that auxMap must be kept in synch with the combo box above!
  int auxMap[9][8] = {{0,1,2,3,4,5,6,7},
		      {0,1,2,3,0,1,2,3},
		      {0,2,4,5,0,2,4,6},
		      {0,1,0,1,0,1,0,1},
		      {0,2,0,2,0,2,0,2},
		      {0,3,0,3,0,3,0,3},
		      {7,6,5,4,3,2,1,0},
		      {3,2,1,0,3,2,1,0},
		      {6,4,2,0,6,4,2,0}};

  m_step = port_M_step->getinputdata();
  m_direction = port_M_direction->getinputdata();
  m_forward = port_M_forward->getinputdata();
  m_backward = port_M_backward->getinputdata();
  m_seqstate = port_M_seqstate->getinputdata();
  m_seqtrig = port_M_seqtrig->getinputdata();
  m_in0 = port_M_in0->getinputdata();
  m_in1 = port_M_in1->getinputdata();
  m_in2 = port_M_in2->getinputdata();
  m_in3 = port_M_in3->getinputdata();
  m_s0 = port_M_s0->getinputdata();
  m_s1 = port_M_s1->getinputdata();
  m_s2 = port_M_s2->getinputdata();
  m_s3 = port_M_s3->getinputdata();
  m_s4 = port_M_s4->getinputdata();
  m_s5 = port_M_s5->getinputdata();
  m_s6 = port_M_s6->getinputdata();
  m_s7 = port_M_s7->getinputdata();

  //    Big loop for cycling thru cyclesize, and thru polyphony
  for (l1 = 0; l1 < synthdata->poly; l1++)
    for (l2 = 0; l2 < synthdata->cyclesize; l2++)
      {
	//    Switch state according to the inputs, in the order given
	//    (first via step/direction, then back/forward, then
	//    seqtrig/forceload, then Sn.)
	//
	//    Note that step/direction and back/forward have
	//    hysteresis (i.e. must transition from <=0 to >0 to
	//    trigger a transition, while seqstate is merely
	//    level-sensitive on seqtrig, and the S(n) states are
	//    state-sensitive, and highest state number wins.
	//
	//    Further magic: if we're in "external" mode, then
	//    we do _nothing_ with respect to state except bound it to
	//    between 0 and maxstate.  In "internal" mode, the first
	//    sequencer in the chain (the one with myfirststate == 0)
	//    does all the state computation, including arbitration
	//    when a particular state is set to "skip", "pause", or "reset"

	int oldstate;
	oldstate = state[l1];
	
	if (int_ext_mode == 1)
	  {
	    state[l1] = int ((m_seqstate[l1][l2]+1.0)*maxstate*0.5);
	  }
	else
	  {

	    //   Do "normal (that is, Moog 960 style) motion.
	    //    Note that step and direction are only meaningful if
	    //     we are the first sequencer in the chain; otherwise S/D
	    //      and F/B are disregarded.
	    //
	    //    Past that, we _also_ disregard S/D/F/B
	    //     if we're in a "pause" state.  In pause, we obey only
	    //      "jump to" inputs.
	    //
	    //   if seqtrig > 0 (i.e. connected and active) then the
	    //     the first sequencer should switch to it, because that's
	    //      an update.
	    //   This leaves out how to relay pause/reset.  Pause is simple
	    //    enough... seqtrig < 0 should inhibit SDFB, and all sequencers
	    //     including the first should accept and pass thru seqstate
	    //      unmodified (unless they've got Jump-To-State input set,
	    //       in which case seqtrig > 0 and seqstate = state).
	    //   Reset on the other hand is handled by seqtrig > 0 and
	    //     seqstate = -1 (= -1 because we normalize state to
	    //      the range [-1.0, 1.0] and thus state 0 is always -1.0
	    //   Note that this means you can have the SEQSTATE direct-access
	    //    functionality OR the Jump-to functionality, but not both
	    //     without some other trickery.
	
	    //   Step/Direction and Forward/Backward for secondary
	    //    sequencers is ignored.
	    if (myfirststate > 0.0)
	      {
		state[l1] = int ((m_seqstate[l1][l2]+1.0)*maxstate*0.5);
	      }
	    else
	      {
		// == 0 means "allow F/B and S/D controls"
		if ( ( (state[l1] < MODULE_V8SEQUENCER_STATES )
		       && (stateaction[state[l1]] == 0))
		     || ( (state[l1] >= MODULE_V8SEQUENCER_STATES )
			  && (m_seqtrig[l1][l2] == 0)))
		  {
		    //   step and direction
		    if ((oldstep[l1] <= 0 ) && (m_step[l1][l2] > 0))
		      {
			if (m_direction[l1][l2] < 0)
			  { state[l1]--;}
			else
			  { state[l1]++;};
		      };
		
		    //   backward/forward
		    if ((oldforward[l1] <= 0.0) && (m_forward[l1][l2] > 0.0) )
		      {
			state[l1]++;
		      };
		
		    if ((oldbackward[l1] <= 0.0) && (m_backward[l1][l2] > 0.0))
		      {
			state[l1]--;
		      };
		  }
		else
		  {    //  seqtrig > 0 means load state, one way or another.
		    if (m_seqtrig [l1][l2] > 0)   //  load seqstate
		      state[l1] = int((m_seqstate[l1][l2]+1.0)*maxstate*0.5);
		  };	
		oldstep[l1] = m_step[l1][l2];
		oldforward[l1] = m_forward[l1][l2];
		oldbackward[l1] = m_backward[l1][l2];
	      }
	  }

	//   Does our current state do anything "funny", like reset,
	//   pause, etc?
	statepause = m_seqtrig[l1][l2];
	if (state[l1]-myfirststate > -0.5 && state[l1]-myfirststate < 7.5)
	  switch (stateaction[state[l1]-myfirststate])
	    {
	  case 0:    //  normal
	    {
	      statepause = 0;
	      break;
	    }
	  case 1:   //   skip
	    {
	      statepause = 1;
	      state[l1]++;
	      break;
	    }
	  case 2:    //   stop
	    {
	      //   Make things stop by setting m_seqtrig to -1 via pausestate
	      statepause = -1;
	      break;
	    }
	  case 3:    // reset to state 0
	    {
	      statepause = 1;
	      state[l1] = 0;
	      break;
	    }
	  }

	if (state[l1] >= maxstate - 0.5)
	  state[l1] = 0;
	if (state[l1] < -0.5)
	  state[l1] = maxstate-1;
	
	//    Direct state setting - overrides everything else assuming
	//    we're in "internal" mode.

	if (m_s0[l1][l2] > 0) state[l1] = 0 + myfirststate;
	if (m_s1[l1][l2] > 0) state[l1] = 1 + myfirststate;
	if (m_s2[l1][l2] > 0) state[l1] = 2 + myfirststate;
	if (m_s3[l1][l2] > 0) state[l1] = 3 + myfirststate;
	if (m_s4[l1][l2] > 0) state[l1] = 4 + myfirststate;
	if (m_s5[l1][l2] > 0) state[l1] = 5 + myfirststate;
	if (m_s6[l1][l2] > 0) state[l1] = 6 + myfirststate;
	if (m_s7[l1][l2] > 0) state[l1] = 7 + myfirststate;	

	//    Firewall against impossibility...
	if (state[l1] >= maxstate)
	  state[l1] = maxstate - 1;
	if (state[l1] < 0) state[l1] = 0;
	
	//     Global State is now set.  Determine our outputs.
	//
	//   state and trig - note we always pass accurate state output;
	//    but that trig has three states:
	//     > 0 == forced load in effect, from jump-to-S inputs
	//     = 0 == obey S/D, F/B.
	//     < 0 == paused, don't obey S/D, F/B.

	data[4][l1][l2] = (state[l1] * 2.0 / float(maxstate)) - 1.0;

	data[5][l1][l2] = statepause;
		
	//    Data 0...7 are out1, 2, 3, 4, aux1, 2, state, trig
	//
	//   Deal with daisychaining
	if ((state[l1] - myfirststate) > -0.5
	    && (state[l1] - myfirststate) < 7.5 )
	  {   //  yes, we're the active subblock
	    data[0][l1][l2] = oldout0[l1] = out0[state[l1] - myfirststate];
	    data[1][l1][l2] = oldout1[l1] = out1[state[l1] - myfirststate];
	    if (sticky2[state[l1] - myfirststate])
	      {
		data[2][l1][l2] = oldout2[l1] = out2[state[l1] - myfirststate];
	      }
	    else
	      data[2][l1][l2] = oldout2[l1];
	    if (sticky3[state[l1] - myfirststate])
	      {
		data[3][l1][l2] = oldout3[l1] = out3[state[l1] - myfirststate];
	      }
	    else
	      data[3][l1][l2] = oldout3[l1];
	  }
	else
	  {     //  No, we pass through what comes in on the m_in ports
	    data[0][l1][l2] = oldout0[l1] = m_in0[l1][l2];
	    data[1][l1][l2] = oldout1[l1] = m_in1[l1][l2];
	    data[2][l1][l2] = oldout2[l1] = m_in2[l1][l2];
	    data[3][l1][l2] = oldout3[l1] = m_in3[l1][l2];
	  };
	
	//
	//   aux0 and aux1 -
	int aux0state, aux1state;
	float aux0lcl = 0;
	float aux1lcl = 0;

       	aux0state = (auxMap[aux0map][ state[l1]-myfirststate]+aux0offset) % 8;
	switch (aux0src)
	  {
	  case 0:
	    aux0lcl = out0[aux0state];
	    break;
	  case 1:
	    aux0lcl = out1[aux0state];
	    break;
	  case 2:
	    aux0lcl = out2[aux0state];
	    break;
	  case 3:
	    aux0lcl = out3[aux0state];
	    break;
	  };

       	aux1state = (auxMap[aux1map][ state[l1]-myfirststate]+aux1offset) % 8;
	switch (aux1src)
	  {
	  case 0:
	    aux1lcl = out0[aux1state];
	    break;
	  case 1:
	    aux1lcl = out1[aux1state];
	    break;
	  case 2:
	    aux1lcl = out2[aux1state];
	    break;
	  case 3:
	    aux1lcl = out3[aux1state];
	    break;
	  };

	data[6][l1][l2] = aux0lcl;   // aux0
	data[7][l1][l2] = aux1lcl;   // aux1

	//    Gate out - did we take a step?
	data[8][l1][l2] = m_step[1l][l2]
	  + m_forward[l1][l2]
	  + m_backward[l1][l2];

	//    pulse out - did we change state?
	if (state[l1] == oldstate ) {
	  data[9][l1][l2] = 0;
	} else {
	  data[9][l1][l2] = 1;
	}
	//   data[8] thru data[15] are direct state outputs.
	data[10][l1][l2] = (state[l1] - myfirststate == 0) ? 1 : 0;
	data[11][l1][l2] = (state[l1] - myfirststate == 1) ? 1 : 0;
	data[12][l1][l2] = (state[l1] - myfirststate == 2) ? 1 : 0;
	data[13][l1][l2] = (state[l1] - myfirststate == 3) ? 1 : 0;
	data[14][l1][l2] = (state[l1] - myfirststate == 4) ? 1 : 0;
	data[15][l1][l2] = (state[l1] - myfirststate == 5) ? 1 : 0;
	data[16][l1][l2] = (state[l1] - myfirststate == 6) ? 1 : 0;
	data[17][l1][l2] = (state[l1] - myfirststate == 7) ? 1 : 0;
      }
}
