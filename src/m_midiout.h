#ifndef M_MIDIOUT_H
#define M_MIDIOUT_H

#include "module.h"


#define MODULE_MIDIOUT_HEIGHT               100

class M_midiout : public Module
{
    Q_OBJECT

    float mixer_gain[2], triggerLevel; 
    int midiMode, offset[2], controller[2], channel;
    Port *port_in[2], *port_M_trigger;
    PolyArr<bool> trigger;    
    int triggeredNote[2][MAXPOLY], lastmididata[2][MAXPOLY];
    
  public: 
    float **inData[2], **triggerData;
                            
  public:
    M_midiout(QWidget* parent=0);
    ~M_midiout();

    void generateCycle();
};
  
#endif
