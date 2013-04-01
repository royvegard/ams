#ifndef MIDIPUSHBUTTON_H
#define MIDIPUSHBUTTON_H

#include <qpushbutton.h>
#include <qlabel.h>
#include "synthdata.h"
#include "midiguicomponent.h"

/**
  *@author Matthias Nagorni
  */

class MidiPushButton : public MidiGUIcomponent {

Q_OBJECT

private slots:
  void clicked();


public:
  QPushButton *pushButton;
    
public:
  MidiPushButton(class MidiControllableDoOnce &mcAble);

  virtual MidiGUIcomponent *createTwin();

  void mcAbleChanged() {};

};  
#endif
