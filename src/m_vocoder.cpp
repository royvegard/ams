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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "midicheckbox.h"
#include "midislider.h"
// For FFTW to be happy...
#include <complex.h>
#include <fftw3.h>
#include "port.h"
#include "m_vocoder.h"

//   Window function - One way to make the FFT behave
//   and give more continuous results over edge steps.

float M_vocoder::windowcurve (int windowfunc, int len, int elem, float alpha)
{
  float out;
  out = 1.0;
  switch (windowfunc)
    {
    case 0:
      //  Rectangular window
      out = 1.0;
      break;
    case 1:
      //  Trapezoidal window.
      out = 1.0;
      if (elem < alpha * len)
	{
	  out = elem / (alpha * len);
	}
      if ( (len - elem) > len - (alpha * len) )
	{
	  out = (len - elem) / (alpha * len) ;
	}
      break;
    case 2:
      //  Hann window (raised cosine)
      out = 0.5 * (1.0 - cos (3.14159 * 2 * elem / (len - 1)));
      break;
    case 3:
      //  Hamming window
      out = 0.54 - 0.46 * cos (3.14159 * 2 * elem / (len - 1));
      break;
    case 4:
      //  Tukey window
      out = 1;
      if ( elem <= (alpha * len / 2) )
	out = (1 + cos (3.14159 * (((2 * elem) / (alpha * len))
				   - 1))) / 2;
      if (elem >= (len * (1 - alpha/2)))
	out = (1 + cos (3.14159 * (((2 * elem) / (alpha * len))
				   - (2 * alpha)
				   + 1))) / 2;
      break;
    case 5:
      //  Blackman-Nutall (least spillover)
      out =
	0.3635819
	- 0.4891775 * cos (2 * 3.14159 * elem / (len - 1))
	+ 0.1365995 * cos (4 * 3.14159 * elem / (len - 1))
	- 0.0106411 * cos (6 * 3.14159 * elem / (len - 1));
      break;
    }
  return (out);
}

M_vocoder::M_vocoder(QWidget* parent)
  : Module(M_type_vocoder, 5, parent, tr("FFT Vocoder"))
{
  QString  qs;
  int l1;
  unsigned int l2;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VOCODER_WIDTH, MODULE_VOCODER_HEIGHT);
  port_M_modulator = new Port(tr("Modulator"), PORT_IN, 0, this);
  port_M_pitchshift = new Port(tr("Pitch Shift"), PORT_IN, 1, this);
  port_M_freqshift = new Port(tr("Freq Shift"), PORT_IN, 2, this);
  port_M_channels = new Port(tr("Channels"), PORT_IN, 3, this);
  port_M_carrier = new Port(tr("Carrier"), PORT_IN, 4, this);
  cv.out_off = 65;
  port_altmodulator_out = new Port (tr("Altered Mod"), PORT_OUT, 0, this);
  port_vocoder_out = new Port(tr("Vocoder Out"), PORT_OUT, 1, this);
  port_modfft_out = new Port (tr("Modulator FFT"), PORT_OUT, 2, this);
  port_firstharmonic_out = new Port (tr("Mod 1st H"), PORT_OUT, 3, this);

  fftsize = synthdata->cyclesize * 4;
  //  fprintf (stderr, "FFTSize = %d\n", fftsize);

  channels = 16;
  configDialog->addSlider(tr("&Bins/Channel"), channels, 1,
			  (999 < synthdata->cyclesize
			   ? 999 : synthdata->cyclesize));
  vcchannels = 0;
  configDialog->addSlider(tr("&VC Bins/Channels"), vcchannels, -100, 100);
  attack = 0;
  configDialog->addSlider(tr("&Attack time"), attack, 0,1);
  release = 0;
  configDialog->addSlider(tr("&Release time"), release, 0, 1);
  pitchshift = 0;
  configDialog->addSlider(tr("&Pitch shift (octaves)"), pitchshift, -3, 3);
  vcpitch = 0;
  configDialog->addSlider(tr("V&C Pitch shift"), vcpitch, -5, 5);
  freqshift = 0;
  configDialog->addSlider(tr("&Frequency (Bode) shift"), freqshift, -999, 999);
  vcfreqshift = 0;
  configDialog->addSlider(tr("VC Fre&q shift"), vcfreqshift, -500, 500);
  phaseshift = 0;
  configDialog->addSlider(tr("P&hase shift"), phaseshift, -6.283, 6.283);
  myFFTWindowFunc = 0;
  QStringList windowFormats;
  windowFormats << tr("Rectangular");
  windowFormats << tr("Trapezoidal");
  windowFormats << tr("Hann (Cosine)");
  windowFormats << tr("Hamming (Cosine)");
  windowFormats << tr("Tukey (flattop cosine)");
  windowFormats << tr("Blackman-Nutall (minimum spill)");
  configDialog->addComboBox (tr("FFT &Window function"),
			     myFFTWindowFunc, windowFormats);
  dynsplice = 0;
  configDialog->addCheckBox(tr("Dynamic &splicing"), dynsplice);
  rtheta = 0;
  configDialog->addCheckBox(tr("R-&Theta modulator"), rtheta);

  modbuf = (float **)malloc(synthdata->poly * sizeof(float *));
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    modbuf[l1] = (float *)malloc( fftsize * sizeof(float));
    memset( modbuf[l1], 0, fftsize * sizeof(float));
  }
  carrbuf = (float **)malloc(synthdata->poly * sizeof(float *));
  for (l1 = 0; l1 < synthdata->poly; l1++) {
    carrbuf[l1] = (float *)malloc( fftsize * sizeof(float));
    memset( carrbuf[l1], 0, fftsize * sizeof(float));
  }

  modmap = (float *) malloc (sizeof (float) * fftsize);
  armodmap = (float *) malloc (sizeof (float) * fftsize);

  whichwin = 0;
  window = (float *) malloc (sizeof (float) * fftsize);
  for (l2 = 0; l2 < (unsigned int) fftsize; l2++)
    window[l2] = windowcurve (whichwin, fftsize, l2, 0.25);

  //  FFTW setup stuff
  carrinforward = (fftw_complex *) fftw_malloc (sizeof (fftw_complex)
					    * fftsize);
  carrinbackward = (fftw_complex *) fftw_malloc (sizeof (fftw_complex)
					     * fftsize);
  carroutforward = (fftw_complex *) fftw_malloc (sizeof (fftw_complex)
					     * fftsize);
  carroutbackward = (fftw_complex *) fftw_malloc (sizeof (fftw_complex)
					      * fftsize);
  modinforward = (fftw_complex *) fftw_malloc (sizeof (fftw_complex)
					    * fftsize);
  modinbackward = (fftw_complex *) fftw_malloc (sizeof (fftw_complex)
					     * fftsize);
  modoutforward = (fftw_complex *) fftw_malloc (sizeof (fftw_complex)
					     * fftsize);
  modoutbackward = (fftw_complex *) fftw_malloc (sizeof (fftw_complex)
					      * fftsize);
  fftw_set_timelimit (0.1);
  planmodforward = fftw_plan_dft_1d (fftsize, modinforward,
				  modoutforward, FFTW_FORWARD, FFTW_MEASURE);
  planmodbackward = fftw_plan_dft_1d (fftsize, modinbackward,
 				  modoutbackward, FFTW_BACKWARD, FFTW_MEASURE);
  plancarrforward = fftw_plan_dft_1d (fftsize, carrinforward,
				  carroutforward, FFTW_FORWARD, FFTW_MEASURE);
  plancarrbackward = fftw_plan_dft_1d (fftsize, carrinbackward,
 				  carroutbackward, FFTW_BACKWARD, FFTW_MEASURE);
}

M_vocoder::~M_vocoder() {

  int l1;

  for (l1 = 0; l1 < synthdata->poly; l1++) {
    free(modbuf[l1]);
    free(carrbuf[l1]);
  }
  free (modbuf);
  free (carrbuf);
  free (window);
  free (modmap);
  free (armodmap);

  //#define FFTW_CLEANUP
#ifdef FFTW_CLEANUP
  //    Clean up FFTW stuff.
  fftw_destroy_plan (plancarrforward);
  fftw_destroy_plan (plancarrbackward);
  fftw_destroy_plan (planmodforward);
  fftw_destroy_plan (planmodbackward);
  fftw_free (carrinforward);
  fftw_free (carrinbackward);
  fftw_free (carroutforward);
  fftw_free (carroutbackward);
  fftw_free (modinforward);
  fftw_free (modinbackward);
  fftw_free (modoutforward);
  fftw_free (modoutbackward);
#endif
}

void M_vocoder::generateCycle() {

  int l1;  //  l1 indexes along polyphony.
  unsigned int l2;  // l2 indexes along the cycle

  inModulator = port_M_modulator->getinputdata();
  inPitchShift = port_M_pitchshift->getinputdata();
  inFreqShift = port_M_freqshift->getinputdata();
  inChannels = port_M_channels->getinputdata();
  inCarrier = port_M_carrier->getinputdata();

  //       *** Insuring continuity across FFTs  ***
  //    One problem with shifting, etc. is to insure
  //    that the output is continuous in position
  //    across FFT frames.  If *all* you do is forward and
  //    reverse FFT, then there's no problem.  However, if you
  //    do any manipulation of the FFT-format data, then you can
  //    (and usually do) get steps in output at the frame boundaries
  //    when you transform back.  This is audible as a series of
  //    signal-dependent pulses at the frame rate (about 48 Hz for
  //    a cycle size of 1024 and a 48 KHz sample rate. )
  //
  //    The hack fix we use here is to keep a buffer of the last
  //    cyclesize samples we saw, and append them together, then
  //    pick a chunk with level- and gradient-matching to use as
  //    our output.  This is turned on and off by the "Dynamic
  //    splicing" checkbox.
  //
  //    To keep the CPU load down, we stay with a power-of-two FFT
  //    size.  That is, 2 * cyclesize; compared to something not a
  //    power of two, say 1280, 2048 is actually a lot faster to
  //    execute and so what looks like more work is actually
  //    considerably less work.


  //   Did the user change the FFT windowing function?
  if (myFFTWindowFunc != whichwin) {
    whichwin = myFFTWindowFunc;
    for (l2 = 0; l2 < (unsigned int) fftsize; l2++)
      window[l2] = windowcurve (whichwin, fftsize, l2, 0.25);
  }

  //  Our outside loop is the polyphony loop.
  for (l1 = 0; l1 < synthdata->poly; l1++) {

    //  copy the modulator input into inbuf

    for (l2 = 0; l2 < fftsize - synthdata->cyclesize; l2++) {
      modbuf[l1][l2] = modbuf [l1] [l2 + synthdata->cyclesize];
    }
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      modbuf[l1][l2 + fftsize - synthdata->cyclesize ] = inModulator[l1][l2];
    }

    //    Copy the input buffer to modinforward
    for (l2 = 0; l2 < (unsigned int)fftsize ; l2++) {
      modinforward[l2] = modbuf[l1][l2] * window[l2];
    }

//#define QUICKCHECK
#ifdef QUICKCHECK
    for (l2 = 0; l2 < (unsigned int) synthdata->cyclesize; l2++)
      data[0][l1][l2] = creal (modinforward[l2 + fftsize/4]);
    return;
#endif
    //   and forward FFT the modulator
    fftw_execute (planmodforward);

    //    copy the FFT of the modulator to modinbackward.
    for (l2 = 0; l2 < (unsigned int)fftsize; l2++)
      modinbackward[l2] = modoutforward[l2];

    //     Send the FFT of the modulator to the output for giggles
    //     and get an approximation of the first harmonic too.
    float firstharmonicval;
    int firstharmonicindex;
    firstharmonicval = 0.0;
    firstharmonicindex = 1.0;
    for (l2 = 1; l2 < (unsigned int) synthdata->cyclesize; l2++) {
      data[2][l1][l2] = logf(fabs (creal (modoutforward[l2])) + 1.0);
      if (data[2][l1][l2] > firstharmonicval) {
	firstharmonicindex = l2;
	firstharmonicval  = data[2][l1][l2] ;
      }
    };
    data[2][l1][0] = -10.0;
    for (l2 = 0; l2 < (unsigned int) synthdata->cyclesize; l2++) {
      data[3][l1][l2] = log2 (firstharmonicindex);
    };

    //   intermediate frequency-domain munging of modulator
    //   Frequency (additive, Bode-style) shifting first
    for (l2 = 0; l2 < (unsigned int)fftsize; l2++)
      modinbackward[l2] = 0;
    int lclfrq;
    for (l2 = 0; l2 < (unsigned int)fftsize/2; l2++) {
      //   positive frequencies (first half) of the FFT result
      lclfrq = l2 + (int)freqshift + vcfreqshift * inFreqShift[l1][0];
      lclfrq = lclfrq > 0 ? lclfrq : 0;
      lclfrq = lclfrq < ((fftsize/2)-1) ? lclfrq : (fftsize/2)-1;
      modinbackward[ lclfrq ] = modoutforward [l2];
      //   Negative frequencies (second half of the fft result)
      modinbackward[fftsize - lclfrq] = modoutforward [ fftsize - l2];
    }

    //    Pitchshifting (multiplicative, harmonic-retaining) shifting.
    //    Note that we reuse the modoutforward as working space
    for (l2 = 0; l2 < (unsigned int) fftsize; l2++) {
      modoutforward[l2] = modinbackward[l2];
    };
    for (l2 = 0; l2 < (unsigned int)fftsize; l2++)
      modinbackward[l2] = 0;

    float psmod, psfactor;
    psmod = (pitchshift + vcpitch * inPitchShift[l1][0]);
    psfactor = pow (2.0, psmod);
    for (l2 = 0; l2 < (unsigned int)fftsize/2; l2++) {
      //   positive frequencies (first half) of the FFT result
      lclfrq = l2 * psfactor;
      lclfrq = lclfrq > 0 ? lclfrq : 0;
      lclfrq = lclfrq < ((fftsize/2)-1) ? lclfrq : (fftsize/2)-1;
      //   Old way to pitch shift: just move the bucket.  But this puts
      //   nulls wherever the energy is split between two buckets with
      //   a 180 degree phase difference.
      if (rtheta == 0) {
            modinbackward[lclfrq] += modoutforward [l2];
            modinbackward[fftsize - lclfrq] += modoutforward [ fftsize - l2];
      }
      else
	{
	  //
	  //   Better way: move freq. bin, multiply angle by octave motion.
	  //
	  modinbackward[lclfrq] +=
	    cabs (modoutforward [l2])
	    * cexp (I * ( carg (modoutforward [l2])
			  + (l2 * phaseshift * psfactor)));
	  modinbackward[fftsize - lclfrq] +=
	    cabs (modoutforward [ fftsize - l2])
	    * cexp (I * ( carg (modoutforward [ fftsize - l2])
			  + (l2 * phaseshift * psfactor)));
	};
    }
    //     The munged modulator is now in "inbackward"
    //     so inverse-FFT it and output it as altered modulator.
    fftw_execute (planmodbackward);

    //   renormalize the time-domain modulator output
    for (l2 = 0; l2 < (unsigned)fftsize; l2++) {
      modoutbackward [l2] = modoutbackward[l2] / float (fftsize) ;
      modoutbackward [l2] = modoutbackward[l2] / window[l2];
    }

    unsigned int i;
    float residual;
    int clomatch_index;


    //     Splicing the new output to the results
    if (dynsplice == 0.0)
      {
	//   output it as the altered modulator.
	for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
	  data[0][l1][l2] = creal ( modoutbackward [l2 +
						    fftsize/2 -
						    synthdata->cyclesize/2 ]);
	}
	clomatch_index = fftsize - synthdata->cyclesize;
      }
    else
      {
	//   find where in our altered modulator output where a "close match"
	//   exists to our previous output value, then output one
	//   cyclesize worth starting there.  Note that since we start at the
	//   start, we get almost a complete fftsize worth of range to
	//   choose from.
	float clov_sofar;
	float tval, dtval;
	int searchstart;
	float spliceval, dspliceval;
	searchstart = fftsize/2 - synthdata->cyclesize;
	if (searchstart < 1) searchstart = 1;
	clomatch_index = searchstart;
	spliceval = data[0][l1][synthdata->cyclesize - 1];
	dspliceval = spliceval - data[0][l1][synthdata->cyclesize - 2];
	clov_sofar= fabs(creal(modoutbackward[clomatch_index])-spliceval );
	for (l2 = searchstart;
	     l2 < (searchstart + synthdata->cyclesize);
	     l2++)
	  {
	    tval = creal (modoutbackward[l2]);
	    dtval = tval - creal (modoutbackward [l2-1]);
	    if (
		((fabs (tval - spliceval )) < clov_sofar )
		&& ((dtval * dspliceval ) >= 0)
		 )
	      {
		clov_sofar= fabs (tval - spliceval );
		clomatch_index = l2;
		// fprintf (stderr, "%f %d ", clov_sofar, clomatch_index);
	      }
	  };
	//  fprintf (stderr, "%d %f %f ",
	//      clomatch_index, clov_sofar, clodv_sofar);
	
	//   What's our residual error, so that we can splice this
	//   with minimal "click"?
	residual = + spliceval - creal( modoutbackward[clomatch_index]);

	//  Move our wave, with the best match so far established, to
	//   the output buffer area.
	for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
	  data[0][l1][l2] = creal ( modoutbackward [ clomatch_index + l2])
	    + ((1.0 - (float(l2) / float(synthdata->cyclesize))) * residual);
	};

      };

    // fprintf (stderr, "%f %d  \n", residual, clomatch_index);

    //     Now it's time to do the carrier.
    //
    for (l2 = 0; l2 < fftsize - synthdata->cyclesize; l2++) {
      carrbuf [l1][l2] = carrbuf [l1][l2 + synthdata->cyclesize];
    }
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      carrbuf [l1][l2 + fftsize - synthdata->cyclesize] = inCarrier[l1][l2];
    }

    for (l2 = 0; l2 <  unsigned (fftsize); l2++) {
      carrinforward [l2] = carrbuf [l1][l2] * window[l2];
    }

    fftw_execute (plancarrforward);

    for (l2 = 0; l2 < (unsigned) fftsize; l2++) {
      carrinbackward[l2] = carroutforward[l2];
    };

    //   carroutforward now has the carrier,

    //      modoutforward now has the modulator.
    //   Group the modulator into channels, and multipy the channels
    //   over the carrier.

    int localchannels;
    localchannels = channels + vcchannels * inChannels[l1][0];
    if (localchannels < 1) localchannels = 1;
    if (localchannels > fftsize - 1) localchannels = fftsize - 1;
    for (l2 = 0; l2 < (unsigned) fftsize; l2++) {
      modmap[l2] = 0;
      //       initial conditions...
      if (l2 == 0)
	for (i = 0; i < channels; i++)
	  modmap[l2] += cabs (modoutforward[l2 + i]);
      else
	modmap [l2] = modmap[l2 - 1];

      //    add the heads, subtract the tails
      i = l2 + channels;
      if (l2 < (unsigned)fftsize - 2)
	modmap[l2] += cabs( modoutforward [i] );
      i = l2 - channels;
      if (l2 >= channels)
	modmap[l2] -= cabs( modoutforward [i] );
    }

    //   Normalize the modmap
    for (l2 = 0; l2 < (unsigned) fftsize; l2++)
      modmap[l2] = modmap[l2] / localchannels;

    //   Do attack/release
    for (l2 = 0; l2 < (unsigned) fftsize; l2++) {
      if (modmap [l2] > armodmap[l2])
	armodmap [l2] += (1 - attack) * (modmap[l2] - armodmap[l2]);
      if (modmap [l2] < armodmap[l2])
	armodmap [l2] += (1 - release) * (modmap[l2] - armodmap[l2]);
    }

    //   multiply the carrier by the modulation map.
    for (l2 = 0; l2 < (unsigned) fftsize; l2++) {
      carrinbackward[l2] = carroutforward[l2] * armodmap[l2];
    }

    //   reverse transform to final output, and renormalize by 1/fftsize.
    fftw_execute (plancarrbackward);

    int offset;
    for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
      offset = l2 + (fftsize/2) - (synthdata->cyclesize / 2);
      data[1][l1][l2]=
	(creal(carroutbackward[offset]/window[offset])) / (fftsize * 100);
    };
  };
}

