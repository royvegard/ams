#ifndef MCED_H
#define MCED_H

/**
  *@author Karsten Wiese
  */


class MCed {
public:
  MCed() {}
  virtual ~MCed() {}

  virtual void mcAbleChanged() = 0;
};

class MCedThing: public MCed {
  class Module *module;

public:
  MCedThing() : module(NULL) {}
  virtual ~MCedThing();

  void listenTo(Module *);
  void listenTo(Module *, int from);
};
  
#endif
