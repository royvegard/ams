#ifndef MIDICOMBOBOX_H
#define MIDICOMBOBOX_H

#include <QComboBox>
#include <QLabel>

#include "midiguicomponent.h"

/**
  *@author Matthias Nagorni
  */

class MidiComboBox : public MidiGUIcomponent {

Q_OBJECT

public:
  QComboBox *comboBox;
  QLabel *valueLabel;
    
public:
  MidiComboBox(class MidiControllableNames &mcAble);
  virtual MidiGUIcomponent *createTwin();
  ~MidiComboBox();

  void mcAbleChanged();
     
private slots:
  void valueChanged(int value);

};

#endif
