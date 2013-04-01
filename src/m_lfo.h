#ifndef M_LFO_H
#define M_LFO_H

#include "module.h"


#define MODULE_LFO_WIDTH                 85
#define MODULE_LFO_HEIGHT               180

class M_lfo : public Module
{
  Q_OBJECT

  float freq, phi0;
  double wave_period;
  PolyArr<double>
    si, old_si,
    sa, old_sa,
    t, old_t,
    r, old_r,
    sh, old_sh,
    dt;
  PolyArr<int> state;
  PolyArr<bool> trigger;
  Port *port_M_trigger, *port_sine, *port_tri, *port_sawup, *port_sawdown, *port_rect, *port_sh;
                                
public:
  M_lfo(QWidget* parent=0);

  void generateCycle();
};
  
#endif
