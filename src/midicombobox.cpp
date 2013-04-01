#include <math.h>

#include "midicombobox.h"
#include "midicontrollable.h"


MidiComboBox::MidiComboBox(MidiControllableNames &mcAble)
  : MidiGUIcomponent(mcAble)
{
  QString qs;

  componentType = GUIcomponentType_combobox;

  QVBoxLayout *comboFrame = new QVBoxLayout(this);
  comboFrame->setSpacing(5);
  comboFrame->setMargin(5);

  comboFrame->addWidget(&nameLabel);
  //  nameLabel->setFixedHeight(nameLabel->sizeHint().height());
  comboBox = new QComboBox();
  comboBox->addItems(mcAble.itemNames);
  comboBox->setFixedSize(comboBox->sizeHint());
  comboFrame->addWidget(comboBox);
  mcAbleChanged();
  QObject::connect(comboBox, SIGNAL(currentIndexChanged(int)),
		   this, SLOT(valueChanged(int)));
  nameLabel.setBuddy(comboBox);
}

MidiGUIcomponent *MidiComboBox::createTwin()
{
  return new MidiComboBox(*dynamic_cast<MidiControllableNames *>(&mcAble));
}

MidiComboBox::~MidiComboBox(){
}

void MidiComboBox::mcAbleChanged()
{
  comboBox->blockSignals(true);
  comboBox->setCurrentIndex(mcAble.getValue());
  comboBox->blockSignals(false);
}

void MidiComboBox::valueChanged(int val)
{
  ((MidiControllableNames &)mcAble).setVal(val, this);
}
/*
void MidiComboBox::setMidiValueRT(int value)
{
}

void MidiComboBox::setMidiValue(int value) {

  if (!controllerOK)
    controllerOK = abs(getMidiValue() - value) < 4;
  else
    if (controllerOK)
      if (midiSign == 1)
	comboBox->setCurrentIndex(int((float)(comboBox->count()-1) / 127.0 * (float)value));
      else
	comboBox->setCurrentIndex(int((float)(comboBox->count()-1) / 127.0 * (float)(127-value)));
}

void MidiComboBox::updateValue(int value) {

  *valueRef = value;
  comboBox->setCurrentIndex(value);
  emit guiComponentTouched();
}

int MidiComboBox::getMidiValue() {

  int x;

  x = (int)rint(127.0 * comboBox->currentIndex() / (comboBox->count()-1));
  return(x);
}
*/
