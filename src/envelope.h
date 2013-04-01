#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <QWidget>
#include <qsizepolicy.h>
#include <qsize.h>
#include <QPaintEvent>
#include "synthdata.h"
#include "mced.h"

#define ENVELOPE_MINIMUM_WIDTH        100
#define ENVELOPE_MINIMUM_HEIGHT        50
#define SUSTAIN_LEN                   0.5

class Envelope : public QWidget, public MCed
{
  class MidiControllableFloat &delayRef, &attackRef, &holdRef, &decayRef, &sustainRef, &releaseRef;

protected:
  virtual void paintEvent(QPaintEvent *);
    
public:
  Envelope(MidiControllableFloat &p_delayRef, MidiControllableFloat &p_attackRef,
	   MidiControllableFloat &p_holdRef,  MidiControllableFloat &p_decayRef,
	   MidiControllableFloat &p_sustainRef, MidiControllableFloat &p_releaseRef);
  ~Envelope();
  virtual QSize sizeHint() const;
  virtual QSizePolicy sizePolicy() const;
  int setDelay(float p_delay);

  void mcAbleChanged();
};
  
#endif
