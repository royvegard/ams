#ifndef M_MIX_H
#define M_MIX_H

#include "module.h"


#define MODULE_MIX_WIDTH                 85
#define MODULE_MIX_HEIGHT                40
#define MAX_MIX_IN                       16 
       
class M_mix : public Module
{
    Q_OBJECT

    QList<Port*> in_port_list;
    Port *port_out;
    float gain;
    float mixer_gain[MAX_MIX_IN];
        
  public: 
    float **inData[MAX_MIX_IN];
    int in_channels;
                            
  public:
    M_mix(int p_in_channels, QWidget* parent=0);

    void generateCycle();
};
  
#endif
