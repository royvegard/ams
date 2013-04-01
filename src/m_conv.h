#ifndef M_CONV_H
#define M_CONV_H

#include "module.h"


#define MODULE_CONV_HEIGHT                75

class M_conv : public Module
{
    Q_OBJECT

    Port *port_M_in, *port_out;
    int convMode, octave;
    
  public: 
    float **inData;       
                            
  public:
    M_conv(QWidget* parent=0);

    void generateCycle();
};
  
#endif
