#ifndef MIDIGUICOMPONENT_H
#define MIDIGUICOMPONENT_H

#include <QLabel>
#include <QWidget>
#include <QList>

#include "midicontroller.h"
#include "mced.h"

/**
  *@author Matthias Nagorni
  */

enum GUIcomponentType{
    GUIcomponentType_slider,
    GUIcomponentType_intslider, 
    GUIcomponentType_checkbox,
    GUIcomponentType_combobox, 
    GUIcomponentType_pushbutton,
    GUIcomponentType_floatintslider
};

class MidiGUIcomponent : public QWidget, public MCed {
Q_OBJECT

public:
  class MidiControllableBase &mcAble; 
  GUIcomponentType componentType;
  bool controllerOK;
  QLabel nameLabel;
        
public:
  MidiGUIcomponent(MidiControllableBase &mcAble);
  ~MidiGUIcomponent();

  virtual MidiGUIcomponent *createTwin() = 0;

  void invalidateController();

  virtual void updateMin() {}
  virtual void updateMax() {}

signals:
  void sigResetController();
  
public slots:
  void resetControllerOK();
};
  
#endif
