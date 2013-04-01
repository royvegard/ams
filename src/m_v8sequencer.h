/* 
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

#ifndef M_V8SEQUENCER_H
#define M_V8SEQUENCER_H

#include "module.h"


#define MODULE_V8SEQUENCER_HEIGHT                400
#define MODULE_V8SEQUENCER_STATES                8

class M_v8sequencer : public Module
{
  Q_OBJECT 

  int state[MAXPOLY];
  int stateaction[MODULE_V8SEQUENCER_STATES];
  int maxstate;
  int myfirststate;
  int aux0src, aux1src, aux0offset, aux1offset, aux0map, aux1map;
  int int_ext_mode;
  float oldstep[MAXPOLY], oldforward[MAXPOLY], 
    oldbackward[MAXPOLY], oldseqtrig[MAXPOLY];
  float oldout0[MAXPOLY], oldout1[MAXPOLY], 
    oldout2[MAXPOLY], oldout3[MAXPOLY];
   

  
    //   Total input ports: 4 + 2 + 8 = 14
    //      Input ports for stepping control  
  Port *port_M_step, *port_M_direction, *port_M_forward, *port_M_backward;
  //        Input port - direct jump-to
  Port *port_M_seqstate, *port_M_seqtrig;
  //        Input ports for daisy-chaining sequencer blocks
  Port *port_M_in0, *port_M_in1, *port_M_in2, *port_M_in3;
  //        Input ports for jump-to control
  Port *port_M_s0, *port_M_s1, *port_M_s2, *port_M_s3,
       *port_M_s4, *port_M_s5, *port_M_s6, *port_M_s7;
  //        Output ports for linear sequencer outputs  (total 16)
  Port *port_out0, *port_out1, *port_out2, *port_out3;
  Port *port_seqstate, *port_seqtrig;
  Port *port_aux0, *port_aux1;
  Port *port_gate, *port_pulse;
  Port *port_state0, *port_state1, *port_state2, *port_state3,
       *port_state4, *port_state5, *port_state6, *port_state7;

  //    The input vectors (these get pointed to the senders)
  public: 
     float **m_step, **m_direction, **m_forward, **m_backward, 
       **m_seqstate, **m_seqtrig;
     float **m_in0, **m_in1, **m_in2, **m_in3;
     float **m_s0, **m_s1, **m_s2, **m_s3, **m_s4, **m_s5, **m_s6, **m_s7;
    
     //     the per-state sliders and checkboxes
    float 
      out0[MODULE_V8SEQUENCER_STATES], out1[MODULE_V8SEQUENCER_STATES], 
      out2[MODULE_V8SEQUENCER_STATES], sticky2[MODULE_V8SEQUENCER_STATES], 
      out3[MODULE_V8SEQUENCER_STATES], sticky3[MODULE_V8SEQUENCER_STATES];
                            
  public:
    M_v8sequencer(QWidget* parent=0);
    ~M_v8sequencer();

    void generateCycle();
};
  
#endif
