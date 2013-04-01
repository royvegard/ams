#ifndef MIDISLIDERBASE_H
#define MIDISLIDERBASE_H

#include <qslider.h>
#include <qlabel.h>
#include "synthdata.h"
#include "midiguicomponent.h"

/**
  *@author Karsten Wiese
  */

#define SLIDER_SCALE 16384.0

class MidiSliderBase: public MidiGUIcomponent {
  Q_OBJECT

  QLabel valueLabel;
  QLabel minLabel, maxLabel;

public:
  QSlider slider;

  MidiSliderBase(class MidiControllableBase &mcAble, Qt::Orientation orientation);

  virtual void updateMin();
  virtual void updateMax();
  virtual void mcAbleChanged();

private slots:
  void valueChanged(int value);

};

#endif
