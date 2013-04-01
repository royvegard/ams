#include "midicheckbox.h"


MidiCheckBox::MidiCheckBox(MidiControllable<float> &mcAble)
  : MidiGUIcomponent(mcAble)
{
  componentType = GUIcomponentType_checkbox;

  QHBoxLayout *checkFrame = new QHBoxLayout(this);
  checkFrame->setSpacing(5);
  checkFrame->setMargin(5);

  checkBox = new QCheckBox(nameLabel.text());
  checkFrame->addWidget(checkBox);
  checkFrame->addStretch();

  QObject::connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
  mcAbleChanged();
}

MidiGUIcomponent *MidiCheckBox::createTwin()
{
  return new MidiCheckBox(static_cast<MidiControllable<float> &>(mcAble));
}

void MidiCheckBox::toggled(bool checked)
{
  static_cast<MidiControllable<float> &>(mcAble).setVal(checked, this);
}

void MidiCheckBox::mcAbleChanged()
{
  checkBox->blockSignals(true);
  checkBox->setChecked(mcAble.getValue() > 0);
  checkBox->blockSignals(false);
}

