#ifndef M_VCSWITCH_H
#define M_VCSWITCH_H

#include "module.h"

#define MODULE_VCSWITCH_WIDTH                115
#define MODULE_VCSWITCH_HEIGHT               100

class M_vcswitch : public Module
{
    Q_OBJECT

    float switchLevel;
    Port *port_M_in[2], *port_M_cv, *port_out[2], *port_mix;
    
  public:
    M_vcswitch(QWidget* parent=0);

    void generateCycle();
};
  
#endif
