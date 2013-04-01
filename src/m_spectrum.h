#ifndef M_SPECTRUM_H
#define M_SPECTRUM_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>


#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qtimer.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"


#ifndef OUTDATED_CODE

#define MODULE_SPECTRUM_HEIGHT                80

class M_spectrum : public Module
{
  Q_OBJECT

  Port *port_in[2];

public:
  M_spectrum(QWidget* parent=0);

  void generateCycle() {}
};

#else  // OUTDATED_CODE

#define MODULE_SPECTRUM_HEIGHT                80

class M_spectrum : public Module
{
  Q_OBJECT

  private:
    float zoom;
    float gain;
    float mixer_gain[2]; 
    int agc;
    Port *port_in[2];
    float *floatdata;
    QTimer *timer;
    int viewMode, fftFrames, normMode, fftMode, window, refreshMode;
    float freqZoom, f_min, f_max;
    QLabel *minLabel, *maxLabel;
    
  public: 
    float **inData[2];
                            
  public:
    M_spectrum(QWidget* parent=0, const char *name=0);
    ~M_spectrum();
    int setGain(float p_gain);
    float getGain();

  public slots:
    void generateCycle();
    void showConfigDialog();
    void timerProc();
    void updateFFTFrames(int val);
    void updateViewMode(int val);
    void updateZoom(int val);
    void update_f_min(int val);
    void update_f_max(int val);
    void updateNormMode(int val);
    void updateWindow(int val);
    void updateFFTMode(int val);
    void updateRefreshMode(int val);
    void freqZoomToggled(bool on);
    void startSpectrum();
};

#endif // OUTDATED_CODE
  
#endif
