#include <QHBoxLayout>
#include <QVBoxLayout>
#include "midisliderbase.h"
#include "midicontrollable.h"

/**
  *@author Karsten Wiese
  */

MidiSliderBase::MidiSliderBase(MidiControllableBase &mcAble, Qt::Orientation orientation)
  : MidiGUIcomponent(mcAble)
  , slider(orientation)
{
  QVBoxLayout *sliderBox = new QVBoxLayout(this);
  sliderBox->setSpacing(0);
  sliderBox->setMargin(0);

  sliderBox->addWidget(&nameLabel, 0, Qt::AlignHCenter);
  QHBoxLayout *sliderLabels = new QHBoxLayout();
  sliderBox->addLayout(sliderLabels);
  sliderLabels->addWidget(&minLabel, 0);
  sliderLabels->addWidget(&valueLabel, 0, Qt::AlignHCenter);
  sliderLabels->addWidget(&maxLabel, 0, Qt::AlignRight);

  updateMin();
  updateMax();
  slider.setPageStep(mcAble.sliderStep());
  mcAbleChanged();
  QObject::connect(&slider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
  sliderBox->addWidget(&slider);

  nameLabel.setBuddy(&slider);
}

void MidiSliderBase::valueChanged(int value)
{
  mcAble.setVal(value, this);
  valueLabel.setText(mcAble.valString());
}

void MidiSliderBase::updateMin()
{
  minLabel.setText(mcAble.minString());
  slider.blockSignals(true);
  slider.setMinimum(mcAble.sliderMin());
  slider.blockSignals(false);
}

void MidiSliderBase::updateMax()
{
  maxLabel.setText(mcAble.maxString());
  slider.blockSignals(true);
  slider.setMaximum(mcAble.sliderMax());
  slider.blockSignals(false);
}

void MidiSliderBase::mcAbleChanged()
{
  slider.blockSignals(true);
  slider.setValue(mcAble.sliderVal());
  slider.blockSignals(false);
  valueLabel.setText(mcAble.valString());
}
