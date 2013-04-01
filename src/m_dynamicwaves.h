#ifndef M_DYNAMICWAVES_H
#define M_DYNAMICWAVES_H

#include "module.h"


#define MODULE_DYNAMICWAVES_WIDTH                125
#define MODULE_DYNAMICWAVES_HEIGHT               160
#define MODULE_DYNAMICWAVES_MAX_OSC                8
#define DYNAMICWAVES_EXP_TABLE_LEN             32768
#define DYNAMICWAVES_ENVELOPE_RESPONSE           256

enum dynamicWaveFormType { DYNAMICWAVE_SINE, DYNAMICWAVE_SAW, DYNAMICWAVE_TRI, DYNAMICWAVE_RECT, DYNAMICWAVE_SAW2 };

class M_dynamicwaves : public Module
{
    Q_OBJECT

    float attack[8][MODULE_DYNAMICWAVES_MAX_OSC], sustain[MODULE_DYNAMICWAVES_MAX_OSC];
    float release[5][MODULE_DYNAMICWAVES_MAX_OSC];
    float timeScale;
    PolyArr<float[MODULE_DYNAMICWAVES_MAX_OSC]> e_noteOff, de, de_release;
    float tune, osc_tune[MODULE_DYNAMICWAVES_MAX_OSC], gain[MODULE_DYNAMICWAVES_MAX_OSC];
    int octave, osc_octave[MODULE_DYNAMICWAVES_MAX_OSC];
    int harmonic[MODULE_DYNAMICWAVES_MAX_OSC], subharmonic[MODULE_DYNAMICWAVES_MAX_OSC];
    int waveForm[MODULE_DYNAMICWAVES_MAX_OSC];
    float expFMGain, linFMGain;
    float phi0[MODULE_DYNAMICWAVES_MAX_OSC];
    PolyArr<float[MODULE_DYNAMICWAVES_MAX_OSC]> phi;
    float wave_period;
    bool allEnvTerminated;
    Port *port_M_freq, *port_M_exp, *port_M_lin, *port_gate, *port_retrigger;
    Port *port_out;
                    
  public: 
    int oscCount;
    PolyArr<float[MODULE_DYNAMICWAVES_MAX_OSC]> e; 
    PolyArr<bool> noteActive, gate, retrigger;
    PolyArr<bool[MODULE_DYNAMICWAVES_MAX_OSC]> oscActive;
    PolyArr<long[MODULE_DYNAMICWAVES_MAX_OSC]> noteOnOfs, noteOffOfs;
    float **gateData;
    float **retriggerData;
    float **freqData;        
    float **expFMData;        // Frequency modulation exp characteristic
    float **linFMData;        // Frequency modulation lin characteristic
                            
  public:
    M_dynamicwaves(int p_oscCount, QWidget* parent=0);

    void generateCycle();
};
  
#endif
