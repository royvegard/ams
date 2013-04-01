#ifndef M_SCOPE_H
#define M_SCOPE_H

#include <qtimer.h>
#include "module.h"


#define MODULE_SCOPE_HEIGHT                80

class M_scope : public Module
{
  Q_OBJECT

  private:
    int mode, edge, triggerMode;
    float zoom, timeScale, triggerThrs;
    long wavDataSize;
    float gain;
    float mixer_gain[2]; 

    Port *port_in[2];
    float *floatdata;
    QTimer *timer;
    
  public: 
    float **inData[2];
                            
  public:
    M_scope(QWidget* parent=0);
    ~M_scope();

    void generateCycle();
    int setGain(float p_gain);
    float getGain();

  public slots:
    void timerProc();
    void updateTriggerMode(int val);
};
  
#endif
