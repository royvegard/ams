#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qbrush.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QPaintEvent>
#include <math.h>
#include "filter.h"


Filter::Filter(float *p_cutoffRef, float *p_resonanceRef, float *p_risingRef, float *p_fallingRef,
               float *p_hwidthRef, float *p_smoothnessRef,
               QWidget* parent, const char *name)
             : QWidget(parent)
{
  setObjectName(name);
  qtimer = new QTimer(this);
  connect(qtimer, SIGNAL(timeout()), this, SLOT(repaintFilter()));
  cutoffRef = p_cutoffRef;
  resonanceRef = p_resonanceRef;
  risingRef = p_risingRef;
  fallingRef = p_fallingRef;
  hwidthRef = p_hwidthRef;
  smoothnessRef = p_smoothnessRef;
  setPalette(QPalette(QColor(0, 20, 100), QColor(0, 20, 100)));
  setMinimumHeight(140);
}

Filter::~Filter()
{
}

float Filter::logfilt(float logf, float cutoff, float resonance, float rising,
                      float falling, float hwidth, float smoothness) {

  float response;
  float sr, sf, hw, sn, rn;
  float c1, c2, c3, c4, r1, r2, r3, r4;
  float logf1, logf2, logf3, logf4, logf5, logf6, logf7;

  sr = rising / 48.0;
  sf = falling / 48.0;
  sn = smoothness;
  rn = resonance;
  hw = hwidth;
  c1 = sr / (2.0 * sn);
  c2 = - sr / (2.0 * hw);
  c3 = - sf / (2.0 * hw);
  c4 = sf / (2.0 * sn);
  r1 = 1.0 - rn + c1 * sn * sn;
  if (r1 > 1.0 - rn / 2.0) {
    r1 = 1.0 - rn / 2.0;
    c1 = (r1 + rn - 1.0) / (sn * sn);
  }
  r2 = 1.0 + c2 * hw * hw;
  if (r2 < 1.0 - rn / 2.0) {
    r2 = 1.0 - rn / 2.0;
    c2 = (r2 - 1.0) / (hw * hw);
  }
  r3 = 1.0 + c3 * hw * hw;
  if (r3 < 0.5) {
    r3 = 0.5;
    c3 = (r3 - 1.0) / (hw * hw);
  }
  r4 = c4 * sn * sn;
  if (r4 > 0.5) {
    r4 = 0.5;
    c4 = r4 / (sn * sn);
  }
  logf4 = log(cutoff) / M_LN2;
  logf3 = logf4 - hw;
  logf5 = logf4 + hw;
  logf2 = logf3 + (r1 - r2) / sr;
  logf1 = logf2 - sn;
  logf6 = logf5 + (r3 - r4) / sf;
  logf7 = logf6 + sn;
  if (logf < logf1) {
    response = 1.0 - rn;
  }
  if ((logf >= logf1) && (logf < logf2)) {
    response = 1.0 - rn + c1 * (logf - logf1) * (logf - logf1);
  }
  if ((logf >= logf2) && (logf < logf3)) {
    response = r1 + sr * (logf - logf2);
  }
  if ((logf >= logf3) && (logf < logf4)) {
    response = 1.0 + c2 * (logf - logf4) * (logf - logf4);
  }
  if ((logf >= logf4) && (logf < logf5)) {
    response = 1.0 + c3 * (logf - logf4) * (logf - logf4);
  }
  if ((logf >= logf5) && (logf < logf6)) {
    response = r3 - sf * (logf - logf5);
  }
  if ((logf >= logf6) && (logf < logf7)) {
    response = c4 * (logf - logf7) * (logf - logf7);
  }
  if (logf >= logf7) {
    response = 0;
  }
  return(response);
}

float Filter::filt(float f, float cutoff, float resonance, float rising,
                   float falling, float hwidth, float smoothness) {

  float response;

  response = logfilt(log(f)/M_LN2, cutoff, resonance, rising, falling, hwidth, smoothness);
  return((exp(log(10.0)/10.0 * response)-1.0) / (exp(log(10.0)/10.0)-1.0));
//  return((exp(response)-1.0) / (exp(1.0)-1.0));
}

void Filter::paintEvent(QPaintEvent *)
{
  //  QPixmap pm(width(), height());
  QPainter p(this);
  int l1;
  float x1, y1, x2, y2, xscale, yscale;

  //  pm.fill(QColor(0, 20, 100));
  p.setViewport(0, 0, width(), height());
  p.setWindow(0, 0, width(), height());
  p.setPen(QColor(220, 100, 0));
  p.drawRect(0, 0, width(), height());
  xscale = (float)width() * M_LN2 / log((float)synthdata->rate * 0.5);
  yscale = (float)(height()-1);
  for (l1 = 0; l1 < width()-1; ++l1) {
    x1 = (float)l1 / xscale;
    x2 = (float)(l1 + 1) / xscale;
    y1 = yscale * (1.0 - logfilt(x1, *cutoffRef, *resonanceRef, *risingRef, *fallingRef, *hwidthRef, *smoothnessRef));
    y2 = yscale * (1.0 - logfilt(x2, *cutoffRef, *resonanceRef, *risingRef, *fallingRef, *hwidthRef, *smoothnessRef));
    p.drawLine(l1, y1, l1 + 1, y2);
  }
  xscale = (float)width() / (float)(synthdata->rate * 0.5);
  p.setPen(QColor(0, 200, 100));
  for (l1 = 0; l1 < width()-1; ++l1) {
    x1 = (float)l1 / xscale;
    x2 = (float)(l1 + 1) / xscale;
    y1 = yscale * (1.0 - filt(x1, *cutoffRef, *resonanceRef, *risingRef, *fallingRef, *hwidthRef, *smoothnessRef));
    y2 = yscale * (1.0 - filt(x2, *cutoffRef, *resonanceRef, *risingRef, *fallingRef, *hwidthRef, *smoothnessRef));
    p.drawLine(l1, y1, l1 + 1, y2);
  }
  //!!  bitBlt(this, 0, 0, &pm);
}

void Filter::repaintFilter() {

  update();
}

void Filter::updateFilter(int value) {
  qtimer->setSingleShot(true);
  qtimer->start(10);
}

QSize Filter::sizeHint() const {

  return QSize(FILTER_MINIMUM_WIDTH, FILTER_MINIMUM_HEIGHT);
}

QSizePolicy Filter::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void Filter::resizeEvent (QResizeEvent* )
{
  update();
}
