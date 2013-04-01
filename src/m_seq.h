#ifndef M_SEQ_H
#define M_SEQ_H

#include "module.h"


#define MODULE_SEQ_HEIGHT               140
#define MODULE_SEQ_MAX_LEN               32

class M_seq : public Module
{
    Q_OBJECT

  Port *port_trigger, *port_trigger_out, *port_note_out, *port_gate_out, *port_velocity_out;
  float seq_gate, seq_freq, seq_velocity;
  int seq_pos, tick, osc, note_len, triggerCount;
  int tickFrames, tickFramesRemain;

public: 
  int pitch[MODULE_SEQ_MAX_LEN], velocity[MODULE_SEQ_MAX_LEN];
  int bpm, pitch_ofs, seqLen;
  float gate[MODULE_SEQ_MAX_LEN];
  bool trigger, triggerOut;
  float **triggerData;

public:
  M_seq(int p_seqLen, QWidget* parent=0);

  void generateCycle();
  void nextStep();
};
  
#endif
