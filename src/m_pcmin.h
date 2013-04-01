#ifndef M_PCMIN_H
#define M_PCMIN_H

#include "module.h"


#define MODULE_PCMIN_HEIGHT                80

class M_pcmin : public Module
{
    Q_OBJECT

    float gain;
    float mixer_gain[2]; 
    Port *port_out[2];
    
  public: 
    float *pcmdata[2];
                            
  public:
    M_pcmin(QWidget* parent, int port);
    ~M_pcmin();

    void generateCycle();
};
  
#endif
