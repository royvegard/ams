#ifndef M_VCPANNING_H
#define M_VCPANNING_H

#include "module.h"


#define MODULE_VCPANNING_WIDTH                110
#define MODULE_VCPANNING_HEIGHT               120

class M_vcpanning : public Module
{
    Q_OBJECT

    Port *port_M_in, *port_M_pan, *port_out[2];
    
  public: 
    float **inData, **panData;        
    float panGain, panOffset;
    float panPos[MAXPOLY], pan[2][MAXPOLY], oldpan[2][MAXPOLY];
    int panMode;
                                    
  public:
    M_vcpanning(QWidget* parent=0);

    void generateCycle();
};
  
#endif
