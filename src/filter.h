#ifndef FILTER_H
#define FILTER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qlabel.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>
#include "synthdata.h"

#define FILTER_MINIMUM_WIDTH        100
#define FILTER_MINIMUM_HEIGHT        50

class Filter : public QWidget
{
  Q_OBJECT

  private:
    QTimer *qtimer;
    float *cutoffRef, *resonanceRef, *risingRef, *fallingRef, *hwidthRef, *smoothnessRef;

  protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent (QResizeEvent* );            
    
  public:
    Filter(float *p_cutoffRef, float *p_resonanceRef, float *p_risingRef, float *p_fallingRef,
           float *p_hwidthRef, float *p_smoothnessRef, 
           QWidget* parent=0, const char *name=0);
    ~Filter();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    float logfilt(float logf, float cutoff, float resonance, float rising, float falling,
               float hwidth, float smoothness);
    float filt(float f, float cutoff, float resonance, float rising, float falling,
               float hwidth, float smoothness);

  public slots: 
    void updateFilter(int value);
    void repaintFilter();
};
  
#endif
