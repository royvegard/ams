#include "midislider.h"
#include "midicontrollable.h"


MidiSlider::MidiSlider(MidiControllableFloat &mcAble, Qt::Orientation orientation)
  : MidiSliderBase(mcAble, orientation)
{
  componentType = GUIcomponentType_slider;
}

MidiGUIcomponent *MidiSlider::createTwin()
{
  return new MidiSlider(dynamic_cast<MidiControllableFloat &>(mcAble));
}

void MidiSlider::minMaxChanged()
{
  updateMin();
  updateMax();
}
