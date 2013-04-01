#include <math.h>

#include "guiwidget.h"
#include "midicontrollable.h"
#include "midiwidget.h"
#include "midislider.h"


QString MidiControllableBase::temp;

MidiControllableBase::MidiControllableBase(Module &module,
        const QString &aname)
    : name(aname), module(module), midiSign(true)
{
    midiControllableListIndex = module.midiControllables.count();
    module.midiControllables.append(this);
}

MidiControllableBase:: ~MidiControllableBase()
{
}

void MidiControllableBase::updateMGCs(MidiGUIcomponent *sender)
{
    module.mcAbleChanged(this);
    for (typeof(mcws.constBegin()) mcw = mcws.constBegin();
            mcw != mcws.constEnd();  mcw++)
        if (*mcw != sender)
            (*mcw)->mcAbleChanged();

    if (sender)
        synthdata->midiWidget->guiComponentTouched(*this);
}


void MidiControllableBase::connectToController(MidiControllerKey midiController)
{
    if (!midiControllerList.contains(midiController)) {
        midiControllerList.append(midiController);
        synthdata->midiWidget->addMidiControllable(midiController, this);
    }
}

void MidiControllableBase::disconnect(bool *updateActiveMidiControllers)
{
  synthdata->guiWidget->remove(this);
  while (midiControllerList.count())
    disconnectController(midiControllerList.at(0), updateActiveMidiControllers);
}

void MidiControllableBase::disconnectController(
        MidiControllerKey midiController, bool *updateActiveMidiControllers)
{
    midiControllerList.removeAll(midiController);
    synthdata->midiWidget->removeMidiControllable(midiController, this);
}

QString MidiControllableBase::getCleanName()
{
    QString cleanname = name;
    /*strip keyboard shortcut control character*/
    int idx = cleanname.indexOf('&');
    if (idx >= 0)
        cleanname.remove(idx, 1);
    return cleanname;
}


QString MidiControllableBase::getName()
{
    return name;
}


/*MidiControllableDoOnce*/
void MidiControllableDoOnce::updateMGCs(MidiGUIcomponent */*sender*/)
{
    trigger();
}

bool MidiControllableDoOnce::setMidiValueRT(int control14)
{
    if (!midiSign)
        control14 = CONTROL14_MAX - control14;

    if (control14 > std::max((CONTROL14_MAX * 3) / 4, lastVal)) {
        lastVal = CONTROL14_MAX;
        return true;
    }

    if (control14 < std::min(CONTROL14_MAX / 4, lastVal))
        lastVal = control14;

    return false;
}

int MidiControllableDoOnce::getMidiValue()
{
    return 0;
}

int MidiControllableFloat::scale(float v)
{
    float r;

    if (isLog) {
        if (v < 1e-4)
            v = 1e-4;
        r = logf(v);
    } else
        r = v;

    return (int)(r * SLIDER_SCALE);
}

int MidiControllableFloat::sliderMin()
{
    return scaledMin;
}

int MidiControllableFloat::sliderMax()
{
    return scaledMax;
}

int MidiControllableFloat::sliderVal()
{
    return scale(value);
}

int MidiControllableFloat::sliderStep()
{
    return 0;
}

void MidiControllableFloat::setValRT(int val)
{
    float v = (float)val / SLIDER_SCALE;

    if (isLog)
        v = expf(v);

    value = v;
}

void MidiControllableFloat::setVal(int val, MidiGUIcomponent *sender)
{
    setValRT(val);
    updateMGCs(sender);
}

void MidiControllableFloat::setLog(bool log)
{
    isLog = log;
    scaledMin = scale(varMin);
    scaledMax = scale(varMax);
    updateFloatMGCs();
    updateMGCs(NULL);
}

void MidiControllableFloat::setNewMin(int min)
{
    varMin = (float)min / SLIDER_SCALE;
    if (isLog)
        varMin = expf(varMin);
    scaledMin = scale(varMin);
    updateFloatMGCs();
    updateMGCs(NULL);
}

void MidiControllableFloat::setNewMax(int max)
{
    varMax = (float)max / SLIDER_SCALE;
    if (isLog)
        varMax = expf(varMax);
    scaledMax = scale(varMax);
    updateFloatMGCs();
    updateMGCs(NULL);
}

void MidiControllableFloat::setNewMin()
{
    varMin = value;
    scaledMin = scale(varMin);
    updateFloatMGCs();
    updateMGCs(NULL);
}

void MidiControllableFloat::setNewMax()
{
    varMax = value;
    scaledMax = scale(varMax);
    updateFloatMGCs();
    updateMGCs(NULL);
}

void MidiControllableFloat::resetMinMax()
{
    varMin = min;
    scaledMin = scale(varMin);
    varMax = max;
    scaledMax = scale(varMax);
    updateFloatMGCs();
    updateMGCs(NULL);
}

void MidiControllableFloat::updateFloatMGCs()
{
    for (typeof(mcws.constBegin()) mcw = mcws.constBegin();
            mcw != mcws.constEnd();  mcw++) {
        MidiSlider * s = dynamic_cast<MidiSlider *>(*mcw);
        if (s)
            s->minMaxChanged();
    }
}

bool MidiControllableFloat::setMidiValueRT(int control14)
{
    if (!midiSign)
        control14 = CONTROL14_MAX - control14;

    int scaledVal = scaledMin + ((long long)(scaledMax - scaledMin) * control14) / CONTROL14_MAX;
    setValRT(scaledVal);

    return true;
}

int MidiControllableFloat::getMidiValue()
{
    return 0;
}

const QString &MidiControllableFloat::minString()
{
    return temp.setNum(varMin, 'g', 3);
}

const QString &MidiControllableFloat::maxString()
{
    return temp.setNum(varMax, 'g', 3);
}

const QString &MidiControllableFloat::valString()
{
    return temp.setNum(value, 'g', 3);
}
