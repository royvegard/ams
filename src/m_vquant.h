#ifndef M_VQUANT_H
#define M_VQUANT_H

#include "module.h"

#define MODULE_VQUANT_HEIGHT                80

class M_vquant : public Module
{
    Q_OBJECT

    Port *port_M_in, *port_quant;
    
  public: 
    float **inData;       
    float gain;
                                
  public:
    M_vquant(QWidget* parent=0);

    void generateCycle();
};
  
#endif
