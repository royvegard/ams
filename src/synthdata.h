#ifndef SYNTHDATA_H
#define SYNTHDATA_H

#include <stdlib.h>
#include <qobject.h>
#include <qthread.h>
#include <qstring.h>
#include <qcolor.h>
#include <QFont>
#include <qlist.h>
#include <QSemaphore>
#include <QTextStream>
#include <ladspa.h>
#include <jack/jack.h>

#include "config.h"

#ifdef HAVE_LIBZITA_ALSA_PCMI
#include <zita-alsa-pcmi.h>
#endif
#ifdef HAVE_LIBCLALSADRV
#include <clalsadrv.h>
#endif
#ifdef JACK_SESSION
#include <jack/session.h>
#endif

#include "macros.h"
#include "main.h"
#include "midicontroller.h"
#include "msoptions.h"
#include "ringbuffer.h"
#include "notelist.h"


extern QTextStream StdErr;
extern QTextStream StdOut;

struct LadspaLib {
  QString name;
  QList<const LADSPA_Descriptor *> desc;
};

#define EXP2_DEPTH 15
#define EXP2_BUF_LEN (1<<EXP2_DEPTH)
 
class SynthData: public QObject
{
    Q_OBJECT

    int  play_ports;
    int  capt_ports;
    void *play_mods [MAX_PLAY_PORTS / 2];
    void *capt_mods [MAX_CAPT_PORTS / 2];

#ifdef HAVE_LIBCLALSADRV
    Alsa_driver *alsa_handle;
#else
    Alsa_pcmi *alsa_handle;
#endif
    pthread_t    alsa_thread;
 
    jack_client_t *jack_handle;
    jack_port_t *jack_in  [MAX_CAPT_PORTS];
    jack_port_t *jack_out [MAX_PLAY_PORTS];
    bool setAllNotesOff;
    const ModularSynthOptions* synthoptions;
#ifdef JACK_SESSION
    jack_session_event_t *jsession_ev;
    QString js_filename;
#endif

  public:
    bool withAlsa;
    bool withJack;
    float *wave_sine;
    float *wave_saw;
    float *wave_saw2;
    float *wave_rect;
    float *wave_tri;
    float *exp_data;
    static float exp2_data[EXP2_BUF_LEN];
    float **zeroModuleData;
    float edge;

    int notes[MAXPOLY];               // Midi notes
    NoteList noteList;                // Stack for handling legato
    int velocity[MAXPOLY];            // Velocity
    int channel[MAXPOLY];
    int noteCounter[MAXPOLY];
    int sustainNote[MAXPOLY]; // Sustain Pedal Buffer
    bool sustainFlag;
    int midiControllerMode;
   
    int poly;
    unsigned int rate;
    unsigned int cyclesize;
    snd_pcm_uframes_t periodsize;
    int periods;
    int moduleCount;
    int moduleID;

    QList<class M_wavout *> wavoutModuleList;
    QList<class M_midiout *> midioutModuleList;
    QList<class M_scope *> scopeModuleList;
#ifdef OUTDATED_CODE
    QList<class M_spectrum *> spectrumModuleList;
#endif
    QList<class Module *> moduleList;
    QList<class M_advmcv *> listM_advmcv;

    snd_seq_t *seq_handle;
    bool doSynthesis;
    QSemaphore port_sem;
    QList<LadspaLib> ladspaLib;
  //    LADSPA_Descriptor_Function ladspa_dsc_func_list[MAX_SO];
  //    QString ladspa_lib_name[MAX_SO];
    QString colorPath, patchPath, rcPath;
    class MidiWidget *midiWidget;
    class GuiWidget *guiWidget;
    QString name;
    int midi_out_port[2];
    int midiChannel;
    QColor colorBackground;
    QColor colorModuleBackground;
    QColor colorModuleBorder;
    QColor colorModuleFont;
    QColor colorPortFont;
    QColor colorCable;
    QColor colorJack;
    QFont bigFont, smallFont;
    
    struct EditingFlags {
	enum {
	    CrossTopLeft = 1
	};
	int f;
	bool crossTopLeft() {
	    return f & CrossTopLeft;
	}
	void setCrossTopLeft(bool v) {
	    f &= ~CrossTopLeft;
	    if (v)
		f |= CrossTopLeft;
	}
    } editingFlags;

#ifdef JACK_SESSION
    enum jackSessionAction {
        jsaAppQuit = -1,
        jsaSave,
        jsaSaveAndQuit
    };
#endif

    pthread_mutex_t rtMutex;
    QList<MidiController> *activeMidiControllers;
    RingBuffer<MidiControllerKey> mckRead, mckDump;
    RingBuffer<MidiControllableBase *> mcSet;
    int pipeFd[2];
    char pipeMessage;
    unsigned framesDone;
    unsigned framesGUIPinged;

  private:
    void create_zero_data(void);
    static void *alsa_static_thr_main(void *arg);
    static int jack_static_callback(jack_nframes_t nframes, void *arg);
    void *alsa_thr_main(void);
    int  jack_callback(jack_nframes_t nframes);
    void call_modules(void);
    void readAlsaMidiEvents(void);
    void processAlsaMidiEvent(snd_seq_event_t*);
    void handleMidiEventNoteOn(snd_seq_event_t*);
    void handleMidiEventNoteOff(snd_seq_event_t*);
    void handleMidiEventPgmChange(snd_seq_event_t*);
    void handleMidiEventController(snd_seq_event_t*);
    void handleMidiEventPitchbend(snd_seq_event_t*);
    void handleMidiEventChanPress(snd_seq_event_t*);
    void handleMidiEventControll14(snd_seq_event_t*);
    MidiControllerContext* getMidiControllerContext(snd_seq_event_t*);

public:
  SynthData(QObject* parent, const ModularSynthOptions& mso);
  void stopPCM();
  ~SynthData();

  void initVoices();
  void allNotesOff() {
    setAllNotesOff = true;
  }

  float exp_table(float x);
  float exp2_table(float ) FATTR_FLOAT_ARG;
  int incModuleCount();
  int decModuleCount();
  int getModuleCount();
  int getModuleID();
  int getLadspaIDs(QString setName, QString pluginName, int *index, int *n);

  int find_capt_mod(void *);
  int find_play_mod(void *);
  void set_capt_mod(unsigned int k, void *M);
  void set_play_mod(unsigned int k, void *M);

  int initAlsa (const QString& cname, const QString& pname,
          unsigned int fsamp, snd_pcm_uframes_t frsize,
          unsigned int nfrags, int ncapt, int nplay);
  int closeAlsa();

  int initJack (int ncapt, int nplay);
  int closeJack();

#ifdef JACK_SESSION
    static void jack_session_static_callback(jack_session_event_t *ev,
            void *arg);
    void jack_session_callback(jack_session_event_t *ev);
    bool jack_session_event();
    QString getJackSessionFilename() const;
signals:
    void jackSessionEvent(int);
#endif
};

extern SynthData *synthdata;
 
#endif
 
