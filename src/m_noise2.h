#ifndef M_NOISE2_H
#define M_NOISE2_H

#include "module.h"


#define MODULE_NOISE2_HEIGHT               75

enum Noises {WHITE,RAND,PINK,PULSETRAIN};
class M_noise2 : public Module
{
    Q_OBJECT

    int NoiseType;
    unsigned int count;
    float rate, level;
    float buf[3], r;
    Port *port_white, *port_pink, *port_random, *port_pulsetrain;
    float randmax;

  public:
    M_noise2(QWidget* parent=0);

    void generateCycle();
};
  
#endif
