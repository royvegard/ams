#ifndef M_AD_H
#define M_AD_H

#include <QTimer>
#include "module.h"


enum {
    MODULE_AD_WIDTH = 140,
    MODULE_AD_HEIGHT = 40
};

class M_ad : public Module
{
  Q_OBJECT

  private:
    Port *port_in;
    Port *port_out[MAX_ANALOGUE_DRIVER_OUT];
    float detune_amp, detune_rate, drift_amp, drift_rate, detune_mod, drift_mod;
    float detune_a[MAXPOLY], detune_c[MAXPOLY];
    float drift_a[MAX_ANALOGUE_DRIVER_OUT][MAXPOLY],
          drift_c[MAX_ANALOGUE_DRIVER_OUT][MAXPOLY];
    float bypass;
    int detuneCount, driftCount, voice[2];
    QTimer *timer;
        
  public: 
    float **inData;
                            
  public:
    M_ad(int p_outCount, QWidget* parent=0);

    void generateCycle();

  public slots:
    void updateVoices(int n);
    void timerProc();
    void autoTune();
};
  
#endif
