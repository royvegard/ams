#ifndef M_SLEW_H
#define M_SLEW_H

#include "module.h"


#define MODULE_SLEW_HEIGHT                75

class M_slew : public Module
{
    Q_OBJECT

    Port *port_M_in, *port_out;
    float timeUp, timeDown;
    PolyArr<float> lastData;
    
  public: 
    float **inData;       
                            
  public:
    M_slew(QWidget* parent=0);

    void generateCycle();
};
  
#endif
