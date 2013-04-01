#ifndef M_PCMOUT_H
#define M_PCMOUT_H

#include "module.h"


#define MODULE_PCMOUT_HEIGHT                80


class M_pcmout : public Module
{
    Q_OBJECT

    float gain, polyroot;
    float mixer_gain[2];
    float ag, ag_displayed;
    int agc;

    Module *module_in[2];
    Port *port_in[2];
    
    virtual void mcAbleChanged(MidiControllableBase *);

  public: 
    float *pcmdata[2];
                            
  public:
    M_pcmout(QWidget* parent, int port);
    ~M_pcmout();

    void generateCycle();
};
  
#endif
