#ifndef M_SH_H
#define M_SH_H

#include "module.h"


#define MODULE_SH_WIDTH                115
#define MODULE_SH_HEIGHT                80

class M_sh : public Module
{
    Q_OBJECT

    float triggerLevel, sample;
    bool gate;
    Module *in_M_in, *in_M_trig; 
    Port *port_M_in, *port_M_trig, *port_out, *port_gate;
    
  public: 
    float **inData, **trigData;       
                            
  public:
    M_sh(QWidget* parent=0);

    void generateCycle();
};
  
#endif
