#include <qlabel.h>
#include <qlabel.h>
#include <QHBoxLayout>
#include "midipushbutton.h"
#include "midicontrollable.h"

MidiPushButton::MidiPushButton(MidiControllableDoOnce &mcAble)
  : MidiGUIcomponent(mcAble)
{
  componentType = GUIcomponentType_pushbutton;

  QHBoxLayout *buttonBox = new QHBoxLayout(this);
  buttonBox->setMargin(5);
  buttonBox->addStretch(0);
  pushButton = new QPushButton(mcAble.getName());
  buttonBox->addWidget(pushButton);
  buttonBox->addStretch(0);
  QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(clicked()));
}

MidiGUIcomponent *MidiPushButton::createTwin()
{
  return new MidiPushButton(dynamic_cast<MidiControllableDoOnce &>(mcAble));
}

void MidiPushButton::clicked()
{
  static_cast<MidiControllableDoOnce &>(mcAble).trigger();
}
