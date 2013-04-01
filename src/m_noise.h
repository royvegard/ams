#ifndef M_NOISE_H
#define M_NOISE_H

#include "module.h"


#define MODULE_NOISE_HEIGHT               100

class M_noise : public Module
{
    Q_OBJECT

    unsigned int count;
    float rate, level;
    float buf[3], r;
    Port *port_white, *port_pink, *port_random;
    double randmax;

  public:
    M_noise(QWidget* parent=0);

    void generateCycle();
};
  
#endif
