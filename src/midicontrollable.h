#ifndef MIDICONTROLLABLE_H
#define MIDICONTROLLABLE_H

#include <QString>

#include "midisliderbase.h"
#include "midicontroller.h"
#include "module.h"

#define SLIDER_SCALE 16384.0

/*forward declarations*/
class MidiControllerKey;
class MCed;


class MidiControllableBase {
    protected:
        static QString temp;
        QString name;

    public:
        Module &module; 
        int midiControllableListIndex;
        QList<MidiControllerKey> midiControllerList;
        QList<MCed*> mcws;
        bool midiSign; 

        MidiControllableBase(Module &module, const QString &name);
        virtual ~MidiControllableBase();
	void disconnect(bool *updateActiveMidiControllers);
        QString getName();
        QString getCleanName();

        virtual void updateMGCs(MidiGUIcomponent *sender);

        virtual int getValue() { return *(int*)0; }
        virtual int getMin() { return *(int*)0; }
        virtual int getMax() { return *(int*)0; }

        virtual bool setMidiValueRT(int ) = 0;
        virtual int getMidiValue() = 0;

        virtual int sliderMin() { return 0;}
        virtual int sliderMax() { return 0;}
        virtual int sliderVal() { return 0;}
        virtual int sliderStep() { return 0;}

        virtual void setValRT(int ) {};
        virtual void setVal(int , MidiGUIcomponent *) {};

        virtual const QString &minString() { return *(QString*)NULL;}
        virtual const QString &maxString() { return *(QString*)NULL;}
        virtual const QString &valString() { return *(QString*)NULL;}

        void connectTo(MCed *mcw) {
            mcws.append(mcw);
        }

        void disconnect(MCed *mcw) {
            mcws.removeAll(mcw);
        }

        void connectToController(MidiControllerKey midiController);
        void disconnectController(MidiControllerKey midiController,
				  bool *updateActiveMidiControllers = NULL);
};


class MidiControllableDoOnce: public QObject, public MidiControllableBase {
    Q_OBJECT

        int lastVal;

    public:
    MidiControllableDoOnce(Module &module, const QString &name)
        : MidiControllableBase(module, name)
          , lastVal(64)
    {}

    virtual void updateMGCs(MidiGUIcomponent *sender);

    virtual bool setMidiValueRT(int );
    virtual int getMidiValue();

    void trigger() {
        emit triggered();
    }

    signals:
        void triggered();
};



static const int CONTROL14_MAX = (1 << 14) - 1;

template <typename t> class MidiControllable: public MidiControllableBase {
    protected:
        t &value;
        const t min, max;

        static t round(float f);

    public:
        MidiControllable(Module &module, const QString &name, t &value,
                t min, t max)
            : MidiControllableBase(module, name)
              , value(value)
              , min(min)
              , max(max)
        {}

        void setValRT(int val) {
            value = val;
        };

        void setVal(int val, MidiGUIcomponent *sender) {
            setValRT(val);
            updateMGCs(sender);
        }

        int toInt(t _t) {return (int)_t;}
        t toType(float i) {return (t)i;}

        virtual int getValue() { return toInt(value); }
        virtual int getMin() { return toInt(min); }
        virtual int getMax() { return toInt(max); }

        virtual bool setMidiValueRT(int control14) {
            float tick = (float)(max - min) / CONTROL14_MAX;
            if (!midiSign)
                control14 = CONTROL14_MAX - control14;
            t old = value;
            value = min + (int)(0.5 + tick * control14);

            return old != value;
        }

        virtual int getMidiValue() { return 0;}

        virtual int sliderMin() { return getMin(); }
        virtual int sliderMax() { return getMax(); }
        virtual int sliderVal() { return getValue(); }
        virtual int sliderStep() { return 1; }

        virtual const QString &minString() { return temp.setNum(toInt(min)); }
        virtual const QString &maxString() { return temp.setNum(toInt(max)); }
        virtual const QString &valString() { return temp.setNum(toInt(value)); }
};


template <>
inline float MidiControllable<float>::round(float f) {
    return f;
}


template <>
inline int MidiControllable<int>::round(float f) {
    return (int)(0.5 + f);
}


class MidiControllableFloat: public MidiControllable<float> {
    bool isLog;
    float varMin, varMax;
    int scaledMin, scaledMax;

    public:
    MidiControllableFloat(Module &module, const QString &name,
            float &value, float min, float max, bool isLog = false)
        : MidiControllable<float>(module, name, value, min, max)
          , isLog(isLog)
    {
        resetMinMax();
    }

    operator float() {return value;}

    int scale(float );

    bool getLog() {
        return isLog;
    }
    void setLog(bool );
    void setNewMin(int min);
    void setNewMax(int max);
    void setNewMin();
    void setNewMax();
    void resetMinMax();
    void updateFloatMGCs();

    virtual bool setMidiValueRT(int );
    virtual int getMidiValue();

    virtual int sliderMin();
    virtual int sliderMax();
    virtual int sliderVal();
    virtual int sliderStep();

    virtual void setValRT(int );
    virtual void setVal(int , MidiGUIcomponent *);

    virtual const QString &minString();
    virtual const QString &maxString();
    virtual const QString &valString();
};


class MidiControllableNames: public MidiControllable<int> {
    public:
        QStringList itemNames;

        MidiControllableNames(Module &module, const QString &name,
                int &value, const QStringList &itemNames)
            : MidiControllable<int>(module, name, value, 0,
                    itemNames.count() - 1)
            , itemNames(itemNames)
        {}
};

#endif
