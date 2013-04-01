#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <QPolygon>
#include "envelope.h"
#include "midicontrollable.h"


Envelope::Envelope(MidiControllableFloat &delayRef,
        MidiControllableFloat &attackRef, MidiControllableFloat &holdRef,
        MidiControllableFloat &decayRef, MidiControllableFloat &sustainRef,
        MidiControllableFloat &releaseRef)
  : delayRef(delayRef)
  , attackRef(attackRef)
  , holdRef(holdRef)
  , decayRef(decayRef)
  , sustainRef(sustainRef)
  , releaseRef(releaseRef)
{
  delayRef.connectTo(this);
  attackRef.connectTo(this);
  holdRef.connectTo(this);
  decayRef.connectTo(this);
  sustainRef.connectTo(this);
  releaseRef.connectTo(this);

  setObjectName("Envelope");
  setAutoFillBackground(true);
  setPalette(QPalette(QColor(0, 20, 100), QColor(10, 50, 10)));
  setMinimumHeight(140);
}

Envelope::~Envelope()
{
  delayRef.disconnect(this);
  attackRef.disconnect(this);
  holdRef.disconnect(this);
  decayRef.disconnect(this);
  sustainRef.disconnect(this);
  releaseRef.disconnect(this);
}

void Envelope::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  QPolygon points(7);
  QPen pen;
  float len, x, y, xscale, yscale;

  p.setViewport(0, 0, width(), height());
  p.setWindow(0, 0, width(), height());
  len = delayRef + attackRef + holdRef + decayRef + SUSTAIN_LEN + releaseRef;
  xscale = (float)width() / len;
  yscale = (float)(height()-6);
  x = delayRef * xscale;
  points.setPoint(0, (int)x, height());
  x += attackRef * xscale;
  points.setPoint(1, (int)x, 6);
  x += holdRef * xscale;
  points.setPoint(2, (int)x, 6);
  x += decayRef * xscale;
  y = sustainRef * yscale;
  points.setPoint(3, (int)x, height() - (int)y);
  x += SUSTAIN_LEN * xscale;
  points.setPoint(4, (int)x, height() - (int)y);
  x += releaseRef * xscale;
  points.setPoint(5, (int)x, height() - 1);
  x = delayRef * xscale;
  points.setPoint(6, (int)x, height() - 1);
  p.setBrush(QBrush(QColor(10, 80, 10)));
  p.drawPolygon(points);
  pen.setColor(QColor(10, 110, 10));
  pen.setWidth(3);
  p.setPen(pen);
  p.drawPolyline(points);
  pen.setColor(QColor(20, 160, 20));
  pen.setWidth(1);
  p.setPen(pen);
  p.drawPolyline(points);
}

void Envelope::mcAbleChanged()
{
  update();
}

QSize Envelope::sizeHint() const {

  return QSize(ENVELOPE_MINIMUM_WIDTH, ENVELOPE_MINIMUM_HEIGHT);
}

QSizePolicy Envelope::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

