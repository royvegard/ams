#ifndef MIDISLIDER_H
#define MIDISLIDER_H

#include "midisliderbase.h"

/**
  *@author Karsten Wiese
  */

class MidiSlider : public MidiSliderBase {
public:
  MidiSlider(class MidiControllableFloat &mcAble, Qt::Orientation orientation = Qt::Horizontal);
  virtual MidiGUIcomponent *createTwin();

  void minMaxChanged();
};

#endif
