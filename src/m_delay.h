#ifndef M_DELAY_H
#define M_DELAY_H

#include "module.h"


#define MODULE_DELAY_HEIGHT                75
#define MAX_DELAY_FRAMES                16384

class M_delay : public Module
{
    Q_OBJECT 

    float delay;
    int read_ofs;
    Port *port_M_in, *port_out;
    
  public: 
    float **inData, **buf;       
                            
  public:
    M_delay(QWidget* parent=0);
    ~M_delay();

    void generateCycle();
};
  
#endif
