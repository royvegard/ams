#ifndef M_INV_H
#define M_INV_H

#include "module.h"


#define MODULE_INV_HEIGHT                80

class M_inv : public Module
{
    Q_OBJECT

    Port *port_M_in, *port_out;
    
  public: 
    float **inData;       
                            
  public:
    M_inv(QWidget* parent=0);

    void generateCycle();
};
  
#endif
