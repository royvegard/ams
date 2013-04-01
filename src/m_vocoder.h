/* 
  Vocoder - derived from m_delay.cpp

  Copyright (C) 2011 Bill Yerazunis <yerazunis@yahoo.com>

  This file is part of ams.

  ams is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2 as
  published by the Free Software Foundation.

  ams is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ams.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef M_VOCODER_H
#define M_VOCODER_H

#include "module.h"
#include <complex.h>
#include <fftw3.h>

#define MODULE_VOCODER_WIDTH                 105
#define MODULE_VOCODER_HEIGHT                150

class M_vocoder : public Module
{
    Q_OBJECT 

    float channels, vcchannels;
    float attack, release;
    float pitchshift, vcpitch;
    float freqshift, vcfreqshift;
    float phaseshift, vcphaseshift;
    int whichwin, myFFTWindowFunc;
    float dynsplice, rtheta;

    Port *port_M_modulator, *port_M_pitchshift, *port_M_freqshift,
      *port_M_channels, *port_M_carrier;
    Port *port_modfft_out, *port_firstharmonic_out,
      *port_altmodulator_out, 
      *port_vocoder_out;

    fftw_plan planmodforward, planmodbackward, 
      plancarrforward, plancarrbackward;

    fftw_complex *carrinforward, *carroutforward, 
      *carrinbackward, *carroutbackward,
      *modinforward, *modoutforward, 
      *modinbackward, *modoutbackward;

  public: 
    int fftsize;
    float **inModulator, **inPitchShift, **inFreqShift, 
      **inChannels, **inCarrier;
    // the previous time-based samples, for overlapping
    float **modbuf, **carrbuf;
    // window for FFT; computed once and reused.
    float *window;
    // modulation map - result of FFT + channelize of modulator
    float *modmap;
    // modmap with attack/release filtering.
    float *armodmap;

  public:
    float windowcurve (int windowfunc, int len, int elem, float alpha );
    M_vocoder(QWidget* parent=0);
    ~M_vocoder();
    void generateCycle();
};
  
#endif
