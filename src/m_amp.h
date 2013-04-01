#ifndef M_AMP_H
#define M_AMP_H

#include "module.h"


#define MODULE_AMP_HEIGHT                80

class M_amp : public Module
{
    Q_OBJECT

    Port *port_M_in, *port_out;
    float gain;
    
  public: 
    float **inData;       
                            
  public:
    M_amp(QWidget* parent=0);

    void generateCycle();
};
  
#endif
