#ifndef MULTI_ENVELOPE_H
#define MULTI_ENVELOPE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <QWidget>
#include <qstring.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <QPaintEvent>
#include <QResizeEvent>
#include "synthdata.h"
#include "mced.h"

#define MULTI_ENVELOPE_MINIMUM_WIDTH        100
#define MULTI_ENVELOPE_MINIMUM_HEIGHT        50
#define SUSTAIN_LEN                         0.5

class MultiEnvelope : public QWidget, public MCedThing
{
    Q_OBJECT

    int envCount;
    float *timeScaleRef, *attackRef, *sustainRef, *releaseRef;
    QColor colorTable[8];
    
  protected:
    virtual void paintEvent(QPaintEvent *);
    
  public:
    MultiEnvelope(int p_envCount, float *timeScaleRef, float *attackRef, float *sustainRef, float *releaseRef);
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

    void mcAbleChanged();
};
  
#endif
