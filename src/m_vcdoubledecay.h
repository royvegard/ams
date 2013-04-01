#ifndef M_VCDOUBLEDECAY_H
#define M_VCDOUBLEDECAY_H

#include "module.h"


#define MODULE_VCDOUBLEDECAY_WIDTH                130
#define MODULE_VCDOUBLEDECAY_HEIGHT               200

class M_vcdoubledecay : public Module
{
    Q_OBJECT

    Port *port_M_gate, *port_M_retrigger, *port_M_attack, *port_M_decay, *port_M_sustain, *port_M_ratio, *port_M_release, *port_out;
    
  public: 
    float **gateData, **retriggerData, **attackData, **decayData, **sustainData, **releaseData, **ratioData;        
    float a0, d0, s0, r0, rl0, aGain, dGain, sGain, rGain, rlGain;
    PolyArr<float> e, e2, old_e, old_e2, s, old_s;
    PolyArr<int> state;
    PolyArr<bool> noteActive, gate, retrigger;
                                    
  public:
    M_vcdoubledecay(QWidget* parent=0);

    void generateCycle();
};
  
#endif
