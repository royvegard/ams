#ifdef OUTDATED_CODE

#ifndef KSPECTRUMSCREEN_H
#define KSPECTRUMSCREEN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qlabel.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qpixmap.h>
#include <qevent.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include "synthdata.h"
#include <rfftw.h>

#define MINIMUM_WIDTH                 100
#define MINIMUM_HEIGHT                 50
#define SPECTRUM_WIDTH               1280
#define SPECTRUM_HEIGHT               800
#define SPECTRUM_BUFSIZE           128000
#define MIN_SPECTRUM_DB               -60   
#define MAX_SPECTRUM_DB                10   


enum spectrumModeType { SPECTRUM_MODE_NORMAL, SPECTRUM_MODE_SUM };
enum fftModeType { FFT_MODE_POW, FFT_MODE_ABS, FFT_MODE_LOG };
enum viewModeType { VIEW_MODE_FIF, VIEW_MODE_FIFT };
enum normModeType { NORM_MODE_EACH, NORM_MODE_GLOBAL, NORM_MODE_FIXED };
enum spectrumTriggerModeType { SPECTRUM_TRIGGERMODE_CONTINUOUS, SPECTRUM_TRIGGERMODE_SINGLE };
enum fftWindowType { WINDOW_HAMMING, WINDOW_BARTLETT, WINDOW_HANNING, WINDOW_WELCH };

class SpectrumScreen : public QWidget
{
  Q_OBJECT

  signals:
    void f_minmaxChanged();
    void freqChanged();
    void runSpectrum();
    
  private:
    SynthData *synthdata;
    spectrumModeType mode;
    fftModeType fftMode;
    viewModeType viewMode;
    spectrumTriggerModeType triggerMode;
    normModeType normMode;
    fftWindowType window;
    int ch1, ch2;
    double zoom, f_min, f_max, f;
    int fftFrames;
    fftw_real max1, max2, maxsum;
    fftw_real *spectrumbuf_ch1;
    fftw_real *spectrumbuf_ch2;
    fftw_real *spectrumbuf_sum;
    rfftw_plan plan;
    bool freqZoom, enableMouse;
    QPixmap *fftPixmap1, *fftPixmap2;
    QColor palette1[768], palette2[768]; 

  public:
    int readofs, writeofs;
    float *spectrumdata;
    
  private:
    void initPalette(int index);
    
  protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent (QResizeEvent* );            

    virtual void mousePressEvent (QMouseEvent* );            
//    virtual void mouseMoveEvent (QMouseEvent* );    
    
  public:
    SpectrumScreen(QWidget* parent=0, const char *name=0);
    ~SpectrumScreen();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    spectrumModeType setMode(spectrumModeType p_mode);
    fftModeType setFFTMode(fftModeType p_fftMode);
    viewModeType setViewMode(viewModeType p_viewMode);
    normModeType setNormMode(normModeType p_normMode);
    spectrumTriggerModeType setTriggerMode(spectrumTriggerModeType p_triggerMode);
    int setCh1(int p_ch1);
    int setCh2(int p_ch2);
    bool toggleFreqZoom(bool p_freqZoom);
    bool setEnableMouse(bool p_enableMouse);
    float set_f_min(float p_f_min);
    float set_f_max(float p_f_max);
    fftWindowType setWindow(fftWindowType p_window);
    double setZoom(double p_zoom);
    int setFFTFrames(int p_fftFrames);
    spectrumModeType getMode();
    fftModeType getFFTMode();
    viewModeType getViewMode();
    normModeType getNormMode();
    spectrumTriggerModeType getTriggerMode();
    int getCh1();
    int getCh2();
    fftWindowType getWindow();
    double get_f_min();
    double get_f_max();
    double get_f();
    double getZoom();
    int getFFTFrames();

  public slots: 
    void refreshSpectrum();
    void singleShot();
};
  
#endif

#endif
