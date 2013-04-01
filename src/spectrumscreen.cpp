#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qevent.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QMouseEvent>
#include "synthdata.h"
#include "spectrumscreen.h"
#include <rfftw.h>
#include <math.h>

SpectrumScreen::SpectrumScreen(QWidget* parent, const char *name) : QWidget (parent, name)
{

  spectrumdata= (float *)malloc(SPECTRUM_BUFSIZE * sizeof(float));
  spectrumbuf_ch1 = (fftw_real *)malloc((SPECTRUM_BUFSIZE >> 1) * sizeof(fftw_real));
  spectrumbuf_ch2 = (fftw_real *)malloc((SPECTRUM_BUFSIZE >> 1) * sizeof(fftw_real));
  spectrumbuf_sum = (fftw_real *)malloc((SPECTRUM_BUFSIZE >> 1) * sizeof(fftw_real));
  mode = SPECTRUM_MODE_NORMAL;
  fftMode = FFT_MODE_ABS;
  viewMode = VIEW_MODE_FIF;
  normMode = NORM_MODE_EACH;
  triggerMode = SPECTRUM_TRIGGERMODE_CONTINUOUS;
  window = WINDOW_HANNING;
  ch1 = 0;
  ch2 = 1;
  zoom = 1;
  f_min = 0;
  f_max = (double)synthdata->rate / 2.0;
  f = f_max / 2.0;
  max1 = 0;
  max2 = 0;
  maxsum = 0;
  fftFrames = 1024;
  freqZoom = false;
  enableMouse = false;
  setPalette(QPalette(QColor(50, 50, 100), QColor(50, 50, 100)));
  initPalette(0);
  initPalette(1);
  plan = rfftw_create_plan(fftFrames, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
  fftPixmap1 = new QPixmap(SPECTRUM_WIDTH, SPECTRUM_HEIGHT);
  fftPixmap2 = new QPixmap(SPECTRUM_WIDTH, SPECTRUM_HEIGHT);
  fftPixmap1->fill(QColor(50, 50, 100));
  fftPixmap2->fill(QColor(50, 50, 100));
}

SpectrumScreen::~SpectrumScreen()
{
  free(spectrumdata);
  free(spectrumbuf_ch1);
  free(spectrumbuf_ch2);
}

void SpectrumScreen::paintEvent(QPaintEvent *) {

  int l1, l2;
  double xscale, xscale1, xscale2, yscale, max, min, ych1Max, ych2Max, fch1Max, fch2Max;
  int x1, x2, y1ch1, y1ch2, y2ch1, y2ch2, tmp, r, g, y, b, f1, f2, frame1, frame2, xch1Max, xch2Max;
  QPixmap pm(width(), height());
  QPainter p(&pm);
  QString qs;

  pm.fill(QColor(50, 50, 100));

  if ((ch1 < 0) || (ch2 < 0)) max = (ch1 < 0) ? max2 : max1;
  else max = (max1 > max2) ? max1 : max2;

  f1 = int(2.0 * f_min / (double)synthdata->rate * width());
  f2 = int(2.0 * f_max / (double)synthdata->rate * width());

  if (!freqZoom)
  {
    p.setPen(QColor(255, 255, 0));
    p.drawLine(f1, 0, f1, height());
    p.drawLine(f2, 0, f2, height());
    f1 = 0;
    f2 = width();
  }
  frame1 = int((double)f1 / width() * (double)fftFrames / 2.0);
  frame2 = int((double)f2 / width() * (double)fftFrames / 2.0);

  min = (fftMode == FFT_MODE_LOG) ? max - (MAX_SPECTRUM_DB - MIN_SPECTRUM_DB) : 0;

  if (viewMode == VIEW_MODE_FIF)
  {
    xscale = (double)width() / (double)(frame2 - frame1);
    if (fftMode == FFT_MODE_LOG) yscale = (height() - 20.0) / (MIN_SPECTRUM_DB - MAX_SPECTRUM_DB);
    else yscale = zoom * (20.0 - height()) / max;

    if (fftMode == FFT_MODE_LOG && normMode != NORM_MODE_EACH)
    {
      g = 10 * (int)(ceil(0.1 * min + 0.001));
      p.setPen(QColor(0, 200, 200));
      while (g <= MAX_SPECTRUM_DB)
      {
        y = height() + int(yscale * (g - min));
        p.drawLine(0, y, width(), y);
        qs.sprintf("%3d", g);
        p.drawText(width() - 27, y - 2, qs);
        g += 10;
      }
    }

    ych1Max = 0;
    ych2Max = 0;
    fch1Max = 0;
    fch2Max = 0;
    xch1Max = 0;
    xch2Max = 0;

    for (l1 = frame1; l1 < frame2 - 2; l1++)
    {
      x1 = int((double)(l1 - frame1) * xscale);
      x2 = int((double)(l1 + 1 - frame1) * xscale);

      switch (mode)
      {
      case SPECTRUM_MODE_NORMAL:
        y1ch1 = int(yscale * (spectrumbuf_ch1[l1] - min));
        y1ch2 = int(yscale * (spectrumbuf_ch2[l1] - min));
        break;
      case SPECTRUM_MODE_SUM:
        y1ch1 = int(yscale * (spectrumbuf_sum[l1] - min));
        break;
      }

      switch (mode)
      {
      case SPECTRUM_MODE_NORMAL:
        y2ch1 = int(yscale * (spectrumbuf_ch1[l1 + 1] - min));
        y2ch2 = int(yscale * (spectrumbuf_ch2[l1 + 1] - min));
        if (ch1 >= 0)
        {
          p.setPen(QColor(0, 220, 0));
          p.drawLine(x1, height() + y1ch1, x2, height() + y2ch1);
        }
        if (ch2 >= 0)
        {
          p.setPen(QColor(255, 255, 0));
          p.drawLine(x1, height() + y1ch2, x2, height() + y2ch2);
        }
        break;
      case SPECTRUM_MODE_SUM:
        y2ch1 = int(yscale * (spectrumbuf_sum[l1 + 1] - min));
        if ((ch1 >= 0) && (ch2 >= 0))
        {
          p.setPen(QColor(0, 220, 0));
          p.drawLine(x1, height() + y1ch1, x2, height() + y2ch1);
        }
        break;
      }

      if (-y2ch1 > ych1Max)
      {
        ych1Max = -y2ch1;
        fch1Max = (double)(l1 + 1) / (double)fftFrames * (double)synthdata->rate;
        xch1Max = x2;
      }
      if (-y2ch2 > ych2Max)
      {
        ych2Max = -y2ch2;
        fch2Max = (double)(l1 + 1) / (double)fftFrames * (double)synthdata->rate;
        xch2Max = x2;
       }
    }

    if ((ch1 >= 0) && fch1Max > 0)
    {
      p.setPen(QColor(0, 220, 0));
      p.drawText(xch1Max, height() - (int)ych1Max, QString::number(fch1Max));
    }
    if ((ch2 >= 0) && (fch2Max > 0))
    {
      p.setPen(QColor(255, 255, 0));
      p.drawText(xch2Max, height() - (int)ych2Max, QString::number(fch2Max));
    }

    bitBlt(this, 0, 0, &pm);


  } else {
    bitBlt(fftPixmap1, 0, -1, fftPixmap1);
    bitBlt(fftPixmap2, 0, -1, fftPixmap2);
    QPainter p1(fftPixmap1);
    QPainter p2(fftPixmap2);
    yscale = 764.0 / (max - min);
    xscale1 = (double)fftPixmap1->width() / (double)(frame2 - frame1);
    xscale2 = (double)fftPixmap2->width() / (double)(frame2 - frame1);
    switch (mode) {
    case SPECTRUM_MODE_NORMAL:
      for (l1 = 0; l1 < fftPixmap1->width(); l1++) {
        x1 = int((double)l1 / xscale1) + frame1;
        x2 = int((double)(l1 + 1) / xscale1) + frame1;
        y1ch1 = 0;
        y1ch2 = 0;
        if (x2 > x1) {
          for (l2 = x1; l2 < x2; l2++) {
            tmp = int(0.9 * zoom * yscale * (spectrumbuf_ch1[l2] - min));
            if (tmp > y1ch1) y1ch1 = tmp;
            tmp = int(0.9 * zoom * yscale * (spectrumbuf_ch2[l2] - min));
            if (tmp > y1ch2) y1ch2 = tmp;
          }
        } else {
          y1ch1 = int(0.9 * zoom * yscale * (spectrumbuf_ch1[x1] - min));
          y1ch2 = int(0.9 * zoom * yscale * (spectrumbuf_ch2[x1] - min));
        }
        if (y1ch1 < 0) y1ch1 = 0;
        if (y1ch2 < 0) y1ch2 = 0;
        if (y1ch1 > 764) y1ch1 = 764;
        if (y1ch2 > 764) y1ch2 = 764;
        r = palette1[y1ch1].red() + palette2[y1ch2].red();
        g = palette1[y1ch1].green() + palette2[y1ch2].green();
        b = palette1[y1ch1].blue() + palette2[y1ch2].blue();
        if (r > 255) r = 255;
        if (g > 255) g = 255;
        if (b > 255) b = 255;
        p1.setPen(QColor(r, g, b));
        p1.drawPoint(l1, fftPixmap1->height() - 1);
      }
      for (l1 = 0; l1 < fftPixmap2->width(); l1++) {
        x1 = int((double)l1 / xscale2) + frame1;
        x2 = int((double)(l1 + 1) / xscale2) + frame1;
        y1ch1 = 0;
        y1ch2 = 0;
        if (x2 > x1) {
          for (l2 = x1; l2 < x2; l2++) {
            tmp = int(0.9 * zoom * yscale * (spectrumbuf_ch1[l2] - min));
            if (tmp > y1ch1) y1ch1 = tmp;
            tmp = int(0.9 * zoom * yscale * (spectrumbuf_ch2[l2] - min));
            if (tmp > y1ch2) y1ch2 = tmp;
          }
        } else {
          y1ch1 = int(0.9 * zoom * yscale * (spectrumbuf_ch1[x1] - min));
          y1ch2 = int(0.9 * zoom * yscale * (spectrumbuf_ch2[x1] - min));
        }
        if (y1ch1 < 0) y1ch1 = 0;
        if (y1ch2 < 0) y1ch2 = 0;
        if (y1ch1 > 764) y1ch1 = 764;
        if (y1ch2 > 764) y1ch2 = 764;
        r = palette1[y1ch1].red() + palette2[y1ch2].red();
        g = palette1[y1ch1].green() + palette2[y1ch2].green();
        b = palette1[y1ch1].blue() + palette2[y1ch2].blue();
        if (r > 255) r = 255;
        if (g > 255) g = 255;
        if (b > 255) b = 255;
        p2.setPen(QColor(r, g, b));
        p2.drawPoint(l1, fftPixmap2->height() - 1);
      }
      break;
    case SPECTRUM_MODE_SUM:
      yscale = 764.0 / maxsum;
      for (l1 = 0; l1 < fftPixmap1->width(); l1++) {
        x1 = int((double)l1 / xscale1) + frame1;
        x2 = int((double)(l1 + 1) / xscale1) + frame1;
        y1ch1 = 0;
        if (x2 > x1) {
          for (l2 = x1; l2 < x2; l2++) {
            tmp = int(0.9 * zoom * yscale * (spectrumbuf_sum[l2] - min));
            if (tmp > y1ch1) y1ch1 = tmp;
          }
        } else {
          y1ch1 = int(0.9 * zoom * yscale * (spectrumbuf_sum[x1] - min));
        }
        if ((ch1 < 0) || (ch2 < 0)) y1ch1 = 0;
        if (y1ch1 > 764) y1ch1 = 764;
        p1.setPen(palette1[y1ch1]);
        p1.drawPoint(l1, fftPixmap1->height() - 1);
      }
      for (l1 = 0; l1 < fftPixmap2->width(); l1++) {
        x1 = int((double)l1 / xscale2) + frame1;
        x2 = int((double)(l1 + 1) / xscale2) + frame1;
        y1ch1 = 0;
        if (x2 > x1) {
          for (l2 = x1; l2 < x2; l2++) {
            tmp = int(0.9 * zoom * yscale * (spectrumbuf_sum[l2] - min));
            if (tmp > y1ch1) y1ch1 = tmp;
          }
        } else {
          y1ch1 = int(0.9 * zoom * yscale * (spectrumbuf_sum[x1] - min));
        }
        if ((ch1 < 0) || (ch2 < 0)) y1ch1 = 0;
        if (y1ch1 > 764) y1ch1 = 764;
        p2.setPen(palette1[y1ch1]);
        p2.drawPoint(l1, fftPixmap2->height() - 1);
      }
      break;
    }
    bitBlt(this, 0, 0, fftPixmap1);
    if (!freqZoom) {
      f1 = int(2.0 * f_min / (double)synthdata->rate * width());
      f2 = int(2.0 * f_max / (double)synthdata->rate * width());
      p.setPen(QColor(255, 255, 0));
      p.drawLine(f1, 0, f1, height());
      p.drawLine(f2, 0, f2, height());
    }
  }
}

inline float sqr(float x) {
  return(x*x);
}

void SpectrumScreen::refreshSpectrum() {

  int l1, ofs, n;
  float p1, p2, sum;
  fftw_real *tmpbuf_ch1, *tmpbuf_ch2;
  double w;

  tmpbuf_ch1 = (fftw_real *)malloc((SPECTRUM_BUFSIZE >> 1) * sizeof(fftw_real));
  tmpbuf_ch2 = (fftw_real *)malloc((SPECTRUM_BUFSIZE >> 1) * sizeof(fftw_real));
  readofs = writeofs - synthdata->cyclesize - fftFrames;
  if (readofs < 0 ) readofs += SPECTRUM_BUFSIZE >> 1;

  ofs = readofs;
  n = fftFrames >> 1;
  for (l1 = 0; l1 < fftFrames; l1++)
  {
    switch (window)
    {
    case WINDOW_HAMMING:
      w = 1.0 - 0.85185 * cos(M_PI * l1 / n);
      break;
    case WINDOW_BARTLETT:
      w = 2.0 * (1.0 - fabs(l1 - n) / n);
      break;
    case WINDOW_HANNING:
      w = 1.0 - cos(M_PI * l1 / n);
      break;
    case WINDOW_WELCH:
      w = 1.5 * (1.0 - sqr(float(l1 - n) / n));
      break;
    default:
      w = 1.0;
    }
    w /= fftFrames;

    spectrumbuf_ch1[l1] = spectrumdata[2 * ofs + ch1] * w;
    spectrumbuf_ch2[l1] = spectrumdata[2 * ofs + ch2] * w;
    ofs++;
    if (ofs >= SPECTRUM_BUFSIZE >> 1) {
        ofs -= SPECTRUM_BUFSIZE >> 1;
    }
  }

  rfftw_one(plan, spectrumbuf_ch1, tmpbuf_ch1);
  rfftw_one(plan, spectrumbuf_ch2, tmpbuf_ch2);

  switch (normMode)
  {
  case NORM_MODE_EACH:
    if (fftMode == FFT_MODE_LOG) max1 = max2 = maxsum = -100;
    else max1 = max2 =  maxsum = 1e-10;
    break;
  case NORM_MODE_GLOBAL:
    if (fftMode == FFT_MODE_LOG)
    {
      if (maxsum > -100)
      {
        w = 1e-5 * fftFrames;
        max1 -= w;
        max2 -= w;
        maxsum -= w;
      }
    }
    else
    {
      if (maxsum > 0.001)
      {
        max1 *= 0.98;
        max2 *= 0.98;
        maxsum *= 0.98;
      }
    }
    break;
  case NORM_MODE_FIXED:
    if (fftMode == FFT_MODE_LOG) max1 = max2 = maxsum = MAX_SPECTRUM_DB;
    else max1 = max2 = maxsum = 1.0;
    break;
  default:
    ;
  }

  n = fftFrames;
  sum = 0;
  for (l1 = 1; l1 < (n+1)/2 - 1; l1++)
  {
    p1 = 4 * (sqr(tmpbuf_ch1[l1]) + sqr(tmpbuf_ch1[n - l1]));
    p2 = 4 * (sqr(tmpbuf_ch2[l1]) + sqr(tmpbuf_ch2[n - l1]));
    switch (fftMode) {
    case FFT_MODE_POW:
      sum = p1 + p2;
      break;
    case FFT_MODE_ABS:
      sum = sqrt(p1 + p2);
      p1 = sqrt(p1);
      p2 = sqrt(p2);
      break;
    case FFT_MODE_LOG:
      sum = 10 * log10(p1 + p2);
      p1 = 10 * log10(p1);
      p2 = 10 * log10(p2);
      break;
    }

    if (normMode != NORM_MODE_FIXED)
    {
      if (p1 > max1) max1 = p1;
      if (p2 > max2) max2 = p2;
      if (sum > maxsum) maxsum = sum;
    }

    spectrumbuf_ch1[l1] = p1;
    spectrumbuf_ch2[l1] = p2;
    spectrumbuf_sum[l1] = sum;
  }

  free(tmpbuf_ch1);
  free(tmpbuf_ch2);
  update();
}

void SpectrumScreen::singleShot() {

  refreshSpectrum();
}

QSize SpectrumScreen::sizeHint() const {

  return QSize(MINIMUM_WIDTH, MINIMUM_HEIGHT);
}

QSizePolicy SpectrumScreen::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

spectrumModeType SpectrumScreen::setMode(spectrumModeType p_mode) {

  mode = p_mode;
  update();
  return(mode);
}

fftModeType SpectrumScreen::setFFTMode(fftModeType p_fftMode) {

  fftMode = p_fftMode;
  return(fftMode);
}

viewModeType SpectrumScreen::setViewMode(viewModeType p_viewMode) {

  viewMode = p_viewMode;
  return(viewMode);
}

normModeType SpectrumScreen::setNormMode(normModeType p_normMode) {

  normMode = p_normMode;
  if (fftMode == FFT_MODE_LOG) max1 = max2 = maxsum = -100;
  else max1 = max2 =  maxsum = 1e-10;
  return(normMode);
}

spectrumTriggerModeType SpectrumScreen::setTriggerMode(spectrumTriggerModeType p_triggerMode) {

  triggerMode = p_triggerMode;
  return(triggerMode);
}

int SpectrumScreen::setCh1(int p_ch1) {

  ch1 = p_ch1;
  update();
  return(ch1);
}

int SpectrumScreen::setCh2(int p_ch2) {

  ch2 = p_ch2;
  update();
  return(ch2);
}

fftWindowType SpectrumScreen::setWindow(fftWindowType p_window) {

  window = p_window;
  return(window);
}

double SpectrumScreen::setZoom(double p_zoom) {

  zoom = p_zoom;
  update();
  return(zoom);
}

bool SpectrumScreen::toggleFreqZoom(bool p_freqZoom) {

  freqZoom = p_freqZoom;
  return(freqZoom);
}

bool SpectrumScreen::setEnableMouse(bool p_enableMouse) {

  enableMouse = p_enableMouse;
  return(enableMouse);
}

float SpectrumScreen::set_f_min(float p_f_min) {

  f_min = p_f_min;
  return(f_min);
}

float SpectrumScreen::set_f_max(float p_f_max) {

  f_max = p_f_max;
  return(f_max);
}

int SpectrumScreen::setFFTFrames(int p_fftFrames) {

  fftFrames = p_fftFrames;
  rfftw_destroy_plan(plan);
  plan = rfftw_create_plan(fftFrames, FFTW_REAL_TO_COMPLEX, FFTW_IN_PLACE);
  return(fftFrames);
}

spectrumModeType SpectrumScreen::getMode() {

  return(mode);
}

fftModeType SpectrumScreen::getFFTMode() {

  return(fftMode);
}


viewModeType SpectrumScreen::getViewMode() {

  return(viewMode);
}

normModeType SpectrumScreen::getNormMode() {

  return(normMode);
}

spectrumTriggerModeType SpectrumScreen::getTriggerMode() {

  return(triggerMode);
}

int SpectrumScreen::getCh1() {

  return(ch1);
}

int SpectrumScreen::getCh2() {

  return(ch2);
}

fftWindowType SpectrumScreen::getWindow() {

  return(window);
}

double SpectrumScreen::get_f_min() {

  if (freqZoom)
    return(f_min);
  else
    return(0);
}

double SpectrumScreen::get_f_max() {

  if (freqZoom)
    return(f_max);
  else
    return((double)synthdata->rate / 2.0);
}

double SpectrumScreen::get_f() {

  return(f);
}

double SpectrumScreen::getZoom() {

  return(zoom);
}

int SpectrumScreen::getFFTFrames() {

  return(fftFrames);
}

void SpectrumScreen::resizeEvent (QResizeEvent* )
{
  fftPixmap1->resize(width(), height());
  QPainter p(fftPixmap1);
  p.setWindow(0, 0, fftPixmap2->width(), fftPixmap2->height());
  p.drawPixmap(0, 0, *fftPixmap2);
  update();
}

void SpectrumScreen::initPalette (int index) {
  int l1;

  switch (index == 0) {
  case 0:
    for (l1 = 0; l1 < 256; l1++) {
      palette1[l1].setRgb(l1, 0, 0);
    }
    for (l1 = 1; l1 < 256; l1++) {
      palette1[l1+255].setRgb(255, l1, 0);
    }
    for (l1 = 1; l1 < 256; l1++) {
      palette1[l1+510].setRgb(255, 255, l1);
    }
    break;
  case 1:
    for (l1 = 0; l1 < 256; l1++) {
      palette2[l1].setRgb(0, l1, 0);
    }
    for (l1 = 1; l1 < 256; l1++) {
      palette2[l1+255].setRgb(0, 255 - l1, l1);
    }
    for (l1 = 1; l1 < 256; l1++) {
      palette2[l1+510].setRgb(l1, l1, 255);
    }
    break;
  }
}

void SpectrumScreen::mousePressEvent(QMouseEvent *ev) {

  if (!enableMouse) {
    return;
  }
  if (ev->button() == Qt::LeftButton) {
    if (triggerMode == SPECTRUM_TRIGGERMODE_CONTINUOUS) {
      triggerMode = SPECTRUM_TRIGGERMODE_SINGLE;
    } else {
      triggerMode = SPECTRUM_TRIGGERMODE_CONTINUOUS;
      emit(runSpectrum());
    }
  }
}

