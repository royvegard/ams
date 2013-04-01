#ifndef M_ADVMCV_H
#define M_ADVMCV_H

#include "module.h"


#define MODULE_ADVMCV_WIDTH                140
#define MODULE_ADVMCV_HEIGHT               240
#define MODULE_ADVMCV_CONTROLLER_PORTS       4

class M_advmcv : public Module
{
    Q_OBJECT

    float pitchbend;
    PolyArr<float> freq;
    Port *port_note_out, *port_gate_out, *port_velocity_out, *port_trig_out;
    Port *port_aftertouch_out, *port_pitchbend_out, *port_controller_out[MODULE_ADVMCV_CONTROLLER_PORTS];

  public: 
    int  pitch, channel, controller_num[MODULE_ADVMCV_CONTROLLER_PORTS];
    float aftertouch_cv, pitchbend_cv;
    float controller_cv[MODULE_ADVMCV_CONTROLLER_PORTS];
                
  public:
    M_advmcv(QWidget* parent=0);
    ~M_advmcv();

    void aftertouchEvent(int value);
    void controllerEvent(int controlNum, int value);
    void pitchbendEvent(int value);
    
    void generateCycle();
};
  
#endif
