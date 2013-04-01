#ifndef M_MCV_H
#define M_MCV_H

#include "module.h"


#define MODULE_MCV_HEIGHT               120

class M_mcv : public Module
{
    Q_OBJECT

    float pitchbend;
    PolyArr<float> freq;
    Port *port_note_out, *port_gate_out, *port_velocity_out, *port_trig_out;

  public: 
    int  pitch, channel;
                
  public:
    M_mcv(QWidget* parent=0);

    void generateCycle();
};
  
#endif
