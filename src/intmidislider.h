#ifndef INTMIDISLIDER_H
#define INTMIDISLIDER_H

#include "midisliderbase.h"



class IntMidiSlider : public MidiSliderBase {
public:
  IntMidiSlider(GUIcomponentType componentType,
		MidiControllableBase &mcAble, Qt::Orientation = Qt::Horizontal);

  virtual MidiGUIcomponent *createTwin();
};

#endif
