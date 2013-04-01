#ifndef M_RINGMOD_H
#define M_RINGMOD_H

#include "module.h"


#define MODULE_RINGMOD_WIDTH                100
#define MODULE_RINGMOD_HEIGHT                95

class M_ringmod : public Module
{
    Q_OBJECT

    float gain;
    Port *port_M_vco1, *port_M_vco2, *port_out;
    
  public: 
    float **vcoData1, **vcoData2;       
                            
  public:
    M_ringmod(QWidget* parent=0);

    int setGain(float p_gain);
    float getGain();

    void generateCycle();
};
  
#endif
