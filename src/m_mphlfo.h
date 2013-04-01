#ifndef M_MPHLFO_H
#define M_MPHLFO_H

#include "module.h"


#define MODULE_MPHLFO_WIDTH                115
#define MODULE_MPHLFO_HEIGHT               360

class M_mphlfo : public Module
{
    Q_OBJECT

    Port *port_out[16];
    float freq, gain_saw, gain_tri;
    double tri, saw, d_tri, d_saw;
    double o[16];
    int state;
    int mode;
        
  public:
    M_mphlfo(QWidget* parent=0);

    void generateCycle();
};
  
#endif
