#ifndef MIDICHECKBOX_H
#define MIDICHECKBOX_H

#include <qcheckbox.h>
#include <qlabel.h>
#include "synthdata.h"
#include "midiguicomponent.h"
#include "midicontrollable.h"

/**
  *@author Matthias Nagorni
  */

class MidiCheckBox : public MidiGUIcomponent {

Q_OBJECT

private slots:
  void toggled(bool);

private:
  QLabel *valueLabel;

public:
  QCheckBox *checkBox;
    
public:
  MidiCheckBox(MidiControllable<float> &mcAble);

  virtual MidiGUIcomponent *createTwin();

  void mcAbleChanged();
};

#endif
