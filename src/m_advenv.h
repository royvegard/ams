#ifndef M_ADVENV_H
#define M_ADVENV_H

#include "module.h"


#define MODULE_ADVENV_WIDTH                120
#define MODULE_ADVENV_HEIGHT               120
#define ADVENVELOPE_RESPONSE               256

class M_advenv : public Module
{
    Q_OBJECT

    float attack[8], sustain, release[5];
    PolyArr<bool> gate, retrigger, noteActive;
    PolyArr<int> noteOnOfs, noteOffOfs;
    PolyArr<float> e_noteOff, de, e, de_release;
    Port *port_gate, *port_retrigger, *port_inverse_out, *port_gain_out;

  public: 
    float timeScale;
    float **gateData, **retriggerData;
                
  public:
    M_advenv(QWidget* parent=0);

    void generateCycle();
};
  
#endif
