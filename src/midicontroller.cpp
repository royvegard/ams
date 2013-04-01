#include "midicontroller.h"
#include "midiguicomponent.h"
#include "midicontrollable.h"


void MidiControllerContext::setMidiValueRT(int value)
{
    for (typeof(mcAbles.constBegin()) it = mcAbles.constBegin();
            it != mcAbles.constEnd(); it++) {
        if ((*it)->setMidiValueRT(value)) {
            synthdata->mcSet.put(*it);
            synthdata->pipeMessage |= 1;
        }
    }
}
