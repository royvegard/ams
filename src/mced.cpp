#include "module.h"
#include "mced.h"
#include "midicontrollable.h"


/**
  *@author Karsten Wiese
  */


MCedThing::~MCedThing()
{
  if (module)
    for (typeof(module->midiControllables.constBegin()) mcAble =
	   module->midiControllables.constBegin();
	 mcAble != module->midiControllables.constEnd(); ++mcAble)
      (*mcAble)->disconnect(this);
}

void MCedThing::listenTo(Module *m)
{
  module = m;
  for (typeof(module->midiControllables.constBegin()) mcAble =
	 module->midiControllables.constBegin();
       mcAble != module->midiControllables.constEnd(); ++mcAble)
    (*mcAble)->connectTo(this);
}

void MCedThing::listenTo(Module *m, int from)
{
  module = m;
  for (; from < module->midiControllables.count(); ++from)
    module->midiControllables.at(from)->connectTo(this);
}
