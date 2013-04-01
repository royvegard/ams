#ifndef KSCOPESCREEN_H
#define KSCOPESCREEN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qlabel.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qevent.h>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QtOpenGL/QGLWidget>
#include "mced.h"
#include "synthdata.h"

#define MINIMUM_WIDTH                 100
#define MINIMUM_HEIGHT                 50
#define TRIGGER_RANGE               16384
#define SCOPE_BUFSIZE              128000 

enum modeType { MODE_NORMAL, MODE_SUM, MODE_DIFF };
enum edgeType { EDGE_RISING, EDGE_FALLING };
enum triggerModeType { TRIGGERMODE_CONTINUOUS, TRIGGERMODE_TRIGGERED, TRIGGERMODE_SINGLE, TRIGGERMODE_MIDI };

class ScopeScreen : public QGLWidget, public MCedThing
{
  Q_OBJECT

  int ch1, ch2;
  bool triggered;
  float &timeScale;
  int &mode;
  int &edge;
  int &triggerMode;
  float &triggerThrs;
  float &zoom;
  float *scopebuf;
  int scopebufValidFrames;
  float xscale, yscale;
  int x1, x2, y1ch1, y1ch2, y2ch1, y2ch2;
  float s1, s2;

public:
  int timeScaleFrames() {
    return (int )((float )synthdata->rate * timeScale * 0.001);
  }
  float *scopedata;
  int readofs, writeofs;

protected:
  virtual void paintEvent(QPaintEvent *);

  void calcY(int offset);
    
  public:
  ScopeScreen(float &timeScale, int &mode, int &edge, int &triggerMode, float &, float &);
    ~ScopeScreen();
  void mcAbleChanged();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

    int setCh1(int p_ch1);
    int setCh2(int p_ch2);

  public slots: 
    void refreshScope();
    void singleShot();
};
  
#endif
      
