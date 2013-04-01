#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qevent.h>
#include <qpixmap.h>
#include <QPaintEvent>
#include <QResizeEvent>
#include "synthdata.h"
#include "scopescreen.h"

#include <iostream>


ScopeScreen::ScopeScreen(float &timeScale, int &mode, int &edge,
			 int &triggerMode, float &triggerThrs, float &zoom)
  : timeScale(timeScale)
  , mode(mode)
  , edge(edge)
  , triggerMode(triggerMode)
  , triggerThrs(triggerThrs)
  , zoom(zoom)
{

  scopebuf = (float *)malloc(SCOPE_BUFSIZE * sizeof(float));
  scopedata= (float *)malloc(SCOPE_BUFSIZE * sizeof(float));
  scopebufValidFrames = 0;
  mode = MODE_NORMAL;
  edge = EDGE_RISING;
  triggerMode = TRIGGERMODE_CONTINUOUS;
  triggerThrs = 0;
  triggered = false;
  ch1 = 0;
  ch2 = 1;
  zoom = 1;

  readofs = 0;
  writeofs = 0;
  setPalette(QPalette(QColor(0, 80, 0), QColor(0, 80, 0)));
}

ScopeScreen::~ScopeScreen()
{
  free(scopebuf);
  free(scopedata);
}


void ScopeScreen::calcY(int offset)
{
    x2 = int((float)offset * xscale);
    if (ch1 < 0) {
      s1 = 0;
    } else {
      s1 = scopebuf[2 * offset + ch1];
    }
    if (ch2 < 0) {
      s2 = 0;
    } else {
      s2 = scopebuf[2 * offset + ch2];
    }
    switch (mode) {
    case MODE_NORMAL:
      y2ch1 = int(yscale * s1);
      y2ch2 = int(yscale * s2);
      break;
    case MODE_SUM:
      y2ch1 = int(yscale * (s1+s2));
      break;
    case MODE_DIFF:
      y2ch1 = int(yscale * (s1-s2));
      break;
    }
}

void ScopeScreen::paintEvent(QPaintEvent *) {
  //  std::cout << __PRETTY_FUNCTION__ << std::endl;
  //  return;
  int l1, thrs, vw, vh, vh_2;

  QPainter p(this);

  vw = width();
  vh = height();
  vh_2 = vh >> 1;
  xscale = (float)vw / (float)timeScaleFrames();
  yscale = zoom * (float)height() / 65536.0;
  thrs = int(yscale * triggerThrs * 32767.0);
  if (triggerMode == TRIGGERMODE_TRIGGERED) {
    p.setPen(QColor(0, 255, 255));
    p.drawLine(0, vh_2 - thrs, 10, vh_2 - thrs);
    p.setPen(QColor(0, 220, 0));
  }

  calcY(0);
  x1 = x2;
  y1ch1 = y2ch1;
  y1ch2 = y2ch2;

  for (l1 = 1; l1 < timeScaleFrames(); l1++) {
    calcY(l1);
    if (x2 == x1)
      continue;

    switch (mode) {
    case MODE_NORMAL:
      if (ch1 >= 0) {
        p.setPen(QColor(0, 220, 0));
        p.drawLine(x1, vh_2 - y1ch1, x2, vh_2 - y2ch1);
      }
      if (ch2 >= 0) {
        p.setPen(QColor(255, 255, 0));
        p.drawLine(x1, vh_2 - y1ch2, x2, vh_2 - y2ch2);
      }
      break;
    case MODE_SUM:
      if ((ch1 >= 0) && (ch2 >= 0)) {
        p.setPen(QColor(0, 220, 0));
        p.drawLine(x1, vh_2 - y1ch1, x2, vh_2 - y2ch1);
      }
      break;
    case MODE_DIFF:
      if ((ch1 >= 0) && (ch2 >= 0)) {
        p.setPen(QColor(0, 220, 0));
        p.drawLine(x1, vh_2 - y1ch1, x2, vh_2 - y2ch1);
      }
      break;
    }

    x1 = x2;
    y1ch1 = y2ch1;
    y1ch2 = y2ch2;
  }
  p.setPen(QColor(0, 220, 0));
  //  p.drawRect(0, 1, vw-1, vh-1);
  p.drawLine(0, vh_2, vw - 1, vh_2);
  p.drawLine(vw >> 1, 0, vw >> 1, vh);
  p.drawText(5, 20, QString::number((int)timeScale)+" ms");
}

void ScopeScreen::refreshScope() {

  int l1, ofs;
  float s1, s2;

  readofs = writeofs - synthdata->cyclesize - timeScaleFrames();
  if (readofs < 0 ) {
    readofs+=SCOPE_BUFSIZE >> 1;
  }
  ofs = readofs;
  if ((triggerMode == TRIGGERMODE_TRIGGERED) && (ch1 >=0)) {
    l1 = 0;
    triggered = false;
    while(l1 < TRIGGER_RANGE) {
      s1 = scopedata[2 * ofs + ch1];
      ofs--;
      if (ofs < 0 ) {
        ofs += SCOPE_BUFSIZE >> 1;
      }
      s2 = scopedata[2 * ofs +ch1];
      if (edge == EDGE_FALLING) {
        if ((s1 < triggerThrs * 32767.0) && (s2 > triggerThrs * 32767.0)) {
          readofs = ofs;
          triggered = true;
          break;
        }
      } else {
        if ((s1 > triggerThrs * 32767.0) && (s2 < triggerThrs * 32767.0)) {
          readofs = ofs;
          triggered = true;
          break;
        }
      }
      ofs--;
      if (ofs < 0 ) {
        ofs += SCOPE_BUFSIZE >> 1;
      }
      l1++;
    }
  }
  if ((triggerMode != TRIGGERMODE_TRIGGERED) || triggered) {
//    fprintf(stderr, "M1\n");
    for (l1 = 0; l1 < timeScaleFrames(); l1++) {
//      fprintf(stderr, "l1: %d ofs: %d\n", l1, ofs);
      scopebuf[2 * l1] = scopedata[2 * ofs];
      scopebuf[2 * l1 + 1] = scopedata[2 * ofs + 1];
      ofs++;
      if (ofs >= SCOPE_BUFSIZE >> 1) {
        ofs -= SCOPE_BUFSIZE >> 1;
      }
    }
    scopebufValidFrames = timeScaleFrames();
//    fprintf(stderr, "M2\n");
  }
  update();	//  repaint();
}

void ScopeScreen::singleShot() {

  refreshScope();
}

QSize ScopeScreen::sizeHint() const {

  return QSize(MINIMUM_WIDTH, MINIMUM_HEIGHT);
}

QSizePolicy ScopeScreen::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

int ScopeScreen::setCh1(int p_ch1) {

  ch1 = p_ch1;
  update();
  return(ch1);
}

int ScopeScreen::setCh2(int p_ch2) {

  ch2 = p_ch2;
  update();
  return(ch2);
}

void ScopeScreen::mcAbleChanged()
{
    update();
}
