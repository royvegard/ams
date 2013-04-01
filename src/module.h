#ifndef MODULE_H
#define MODULE_H

#include <QColor>
#include <QLabel>
#include <QList>
#include <QPaintEvent>
#include <QPoint>
#include <QString>
#include <QTextStream>
#include <alsa/asoundlib.h>

#include "box.h"
#include "synthdata.h"
#include "configdialog.h"


enum {
   MODULE_DEFAULT_WIDTH = 90,
   MODULE_DEFAULT_HEIGHT = 100, 
   MODULE_NEW_X = 50,
   MODULE_NEW_Y = 50
};

// types jackin and jackout kept to ensure existing patches will load. FA
enum M_typeEnum {
  M_type_custom,
  M_type_vco,
  M_type_vca,
  M_type_lfo,
  M_type_delay,
  M_type_ringmod,
  M_type_ladspa,
  M_type_pcmout,
  M_type_mix,
  M_type_vcf,
  M_type_mcv,		// 10
  M_type_env,
  M_type_seq,
  M_type_inv,
  M_type_noise,
  M_type_slew,
  M_type_quantizer,
  M_type_pcmin,
  M_type_cvs, 
  M_type_sh,
  M_type_vcorgan,	// 20
  M_type_dynamicwaves,
  M_type_advenv,
  M_type_wavout, 
  M_type_scope,
  M_type_spectrum,
  M_type_vcswitch,
  M_type_jackin,
  M_type_jackout,
  M_type_midiout,
  M_type_scmcv,		// 30
  M_type_scquantizer,
  M_type_stereomix,
  M_type_conv, 
  M_type_vcenv,
  M_type_advmcv,
  M_type_function,
  M_type_vcpanning,
  M_type_vcenv2,
  M_type_vcdoubledecay,
  M_type_vquant,        // 40
  M_type_amp,
  M_type_ad,
  M_type_mphlfo,
  M_type_noise2,
  M_type_vco2,
  M_type_v8sequencer,
  M_type_analogmemory,
  M_type_bitgrind,
  M_type_hysteresis,
  M_type_vcdelay,       // 50
  M_type_vocoder
};

/*forward declarations*/
class Port;
class MidiControllableBase;


class Module : public Box
{
  Q_OBJECT
  bool alive;
  int connections;
  QList<Port*> portList;

protected:
    virtual ~Module();

public: 
    ConfigDialog *configDialog;
    float ***data; 
    bool cycleReady;
    M_typeEnum M_type;  
    QList<MidiControllableBase*> midiControllables;
    int moduleID, outPortCount;
    QColor colorBackground, colorBorder, colorFont;
    static size_t portmemAllocated;

    Module(M_typeEnum M_type, int outPortCount, QWidget* parent,
	   const QString &name);
    void Delete();
    bool isAlive() {
	return alive;
    }
    void addPort(Port*); 
    int connected();
    void incConnections();
    void decConnections();
    void save(QTextStream&);
    void saveConnections(QTextStream&); 
    void saveParameters(QTextStream&);
    void saveBindings(QTextStream&); 
    void saveConfigDialog(QTextStream&); 
    void readConfigDialog(QString&);
    virtual void mcAbleChanged(MidiControllableBase *) {}
    void getColors(void);
    float **getData(int);
    void showConfigDialog(const QPoint&);
    bool hasModuleId(int);
    void setModuleId(int);
    MidiControllableBase * getMidiControlableBase(int);
    Port* getPortAt(int);
    Port* getPortWithIndex(int);
    Port* getInPortWithIndex(int);
    Port* getOutPortWithIndex(int);
    void paintCablesToConnectedPorts(QPainter&);
    void setJackColor(QColor& color);
    void setCableColor(QColor& color);

  protected:
    virtual void generateCycle() = 0;

    void portMemAlloc(int outPortCount, bool poly);
    virtual void paintEvent(QPaintEvent *ev);
    void paint(QPainter &);

    struct CtorVar {
        static const int step = 20;
        int in_index, out_index;
        int in_off, out_off;
        void reset() {
            in_index = out_index = 0;
            in_off = out_off = 35;
        }
    };
    static CtorVar cv;

  signals:
    void dragged(QPoint pos);
    void removeModule();
    void portSelected(Port*);
    void portDisconnected();

  public slots: 
    virtual void removeThisModule();
};


template <typename tn> class PolyArr {
  tn D[MAXPOLY];
public:
  PolyArr() {
    Set0();
  }
  PolyArr(tn v) {
    for (int i = 0; i < MAXPOLY; ++i)
      D[i] = v;
  }
  void Set0() {
    memset(D, 0, sizeof(D));
  }
  tn &operator [] (unsigned u) {
    return D[u];
  }
};
  
#endif
