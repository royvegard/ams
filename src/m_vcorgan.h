#ifndef M_VCORGAN_H
#define M_VCORGAN_H

#include "module.h"


#define MODULE_VCORGAN_WIDTH                 85
#define MODULE_VCORGAN_HEIGHT               120
#define MODULE_VCORGAN_MAX_OSC                8
#define VCORGAN_EXP_TABLE_LEN             32768

enum waveFormType { ORGAN_SINE, ORGAN_SAW, ORGAN_TRI, ORGAN_RECT, ORGAN_SAW2 };

class M_vcorgan : public Module
{
    Q_OBJECT

    float tune, osc_tune[MODULE_VCORGAN_MAX_OSC], gain[MODULE_VCORGAN_MAX_OSC];
    int octave, osc_octave[MODULE_VCORGAN_MAX_OSC];
    int harmonic[MODULE_VCORGAN_MAX_OSC], subharmonic[MODULE_VCORGAN_MAX_OSC];
    int waveForm[MODULE_VCORGAN_MAX_OSC];
    float expFMGain, linFMGain;
    float phi0[MODULE_VCORGAN_MAX_OSC], phi[MAXPOLY][MODULE_VCORGAN_MAX_OSC];
    float wave_period;
    Port *port_M_freq, *port_M_exp, *port_M_lin;
    Port *port_out;
                    
  public: 
    int oscCount;
    float **freqData;        
    float **expFMData;        // Frequency modulation exp characteristic
    float **linFMData;        // Frequency modulation lin characteristic
                            
  public:
    M_vcorgan(int p_oscCount, QWidget* parent=0);

    void generateCycle();
};
  
#endif
