#include "intmidislider.h"
#include "midicontrollable.h"

IntMidiSlider::IntMidiSlider(GUIcomponentType componentType, MidiControllableBase &mcAble, Qt::Orientation orientation)
  : MidiSliderBase(mcAble, orientation)
{
  componentType = componentType;

  int ps = 0;
  if (mcAble.getMax() % 12 == 0)
    ps = 12;
  if (ps)
    slider.setPageStep(ps);

  slider.setTickInterval(1);
  slider.setTickPosition(QSlider::TicksBelow);
  slider.setFixedHeight(slider.sizeHint().height());
}

MidiGUIcomponent *IntMidiSlider::createTwin()
{
  return new IntMidiSlider(componentType, mcAble);
}

