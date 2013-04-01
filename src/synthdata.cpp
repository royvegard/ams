#include <sys/time.h>
#include <sys/resource.h>
#include <math.h>
#include <pthread.h>
#include <QApplication>
#include <qobject.h>
#include <qstring.h>
#include <QFont>
#include <QObject>

#include "guiwidget.h"
#include "midiwidget.h"
#include "synthdata.h"
#include "m_advmcv.h"
#include "m_env.h"
#include "m_vcenv.h"
#include "m_vcenv2.h"
#include "m_pcmout.h"
#include "m_pcmin.h"
#include "m_advenv.h"
#include "m_dynamicwaves.h"
#include "m_wavout.h"
#include "m_midiout.h"
#include "m_scope.h"


union uf {
  struct {
    unsigned int mant:	23;
    unsigned int exp:	8;
    unsigned int sign:	1;
  } p;
  float f;
  unsigned u;
};

float SynthData::exp2_data[EXP2_BUF_LEN];

#ifdef JACK_SESSION
static const char JSFILENAME[] = "file.ams";
#endif


SynthData::SynthData(QObject* parent, const ModularSynthOptions& mso)
  : QObject(parent)
  , setAllNotesOff(false)
  , synthoptions(&mso)
  , edge(mso.edge)
  , poly(mso.poly)
  , port_sem(1)
  , name(mso.synthName)
  , bigFont("Helvetica", 10)
  , smallFont("Helvetica", 8)
  , activeMidiControllers(NULL)
  , framesDone(0)
  , framesGUIPinged(0)
{
  setObjectName("SynthData");
  if (pthread_mutex_init(&rtMutex, NULL) < 0) {
    StdErr << __PRETTY_FUNCTION__ << ": pthread_mutex_init() failed" << endl;
    exit(-1);
  }
  if (pipe(pipeFd) < 0) {
    StdErr << __PRETTY_FUNCTION__ << ": pipe() failed" << endl;
    exit(-1);
  }

  int l1, l2;
  double dphi, phi, dy, dyd;
  int decaytime;

  decaytime = (int)((float)WAVE_PERIOD / 16.0);
  initVoices();
  rate = 0;
  periods = 0;
  periodsize = 0;
  cyclesize = 0;
  moduleCount = 0;
  moduleID = 0;
  doSynthesis = false;
  sustainFlag = false;
  midiChannel = -1;
  midiControllerMode = 0;
  colorPath = "";
  patchPath = "";
  rcPath = "";

  exp_data = (float *)malloc(EXP_TABLE_LEN * sizeof(float));
  wave_sine = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_saw = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_saw2 = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_rect = (float *)malloc(WAVE_PERIOD * sizeof(float));
  wave_tri = (float *)malloc(WAVE_PERIOD * sizeof(float));

  dphi = 2.0 * M_PI / WAVE_PERIOD;
  phi = 0;
  for (l1 = 0; l1 < WAVE_PERIOD; l1++) {
    wave_sine[l1] = sin(phi);
    phi += dphi;
  }
  for (l1 = 0; l1 < EXP_TABLE_LEN; l1++)
    exp_data[l1] = exp(l1 / 1000.0 - 16.0);
  {
    unsigned u = 0;
    for (float f = 0; f < 1; f += 1.0/EXP2_BUF_LEN) {
      union uf e;
      e.f = exp2f(f);
      e.p.exp = 0;
      exp2_data[u++] = e.f;
    }
    if (u != EXP2_BUF_LEN) {
      StdErr << __PRETTY_FUNCTION__ << ": exp2_data initialisation failed" << endl;
      exit(-1);
    }
  }
  dy = 2.0 / (float)(WAVE_PERIOD - decaytime);
  dyd = 2.0 / decaytime;
  l2 = 0;
  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++)
    wave_saw[l2++] = (float)l1 * dy;

  for (l1 = 0; l1 < decaytime; l1++)
    wave_saw[l2++] = 1.0 - (float)l1 * dyd;

  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++)
    wave_saw[l2++] = -1.0 + (float)l1 * dy;

  l2 = WAVE_PERIOD - 1;
  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++)
    wave_saw2[l2--] = (float)l1 * dy;

  for (l1 = 0; l1 < decaytime; l1++)
    wave_saw2[l2--] = 1.0 - (float)l1 * dyd;

  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++)
    wave_saw2[l2--] = -1.0 + (float)l1 * dy;

  l2 = 0;
  dyd = 4.0 / decaytime;
  for (l1 = 0; l1 < decaytime>>2; l1++)
    wave_rect[l2++] = (float)l1 * dyd;

  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++)
    wave_rect[l2++] = 1.0;

  for (l1 = 0; l1 < decaytime>>1; l1++)
    wave_rect[l2++] = 1.0 - (float)l1 * dyd;

  for (l1 = 0; l1 < (WAVE_PERIOD - decaytime)>>1; l1++)
    wave_rect[l2++] = -1.0;

  for (l1 = 0; l1 < decaytime>>2; l1++)
    wave_rect[l2++] = -1.0 + (float)l1 * dyd;

  dy = 4.0 / (float)WAVE_PERIOD;
  for (l1 = 0; l1 < (WAVE_PERIOD>>2); l1++)
    wave_tri[l1] = (float)l1 * dy;

  for (l1 = (WAVE_PERIOD>>2); l1 < (WAVE_PERIOD >> 1) + (WAVE_PERIOD>>2); l1++)
    wave_tri[l1] = 1.0 - (float)(l1 - (WAVE_PERIOD>>2)) * dy;

  for (l1 = (WAVE_PERIOD >> 1) + (WAVE_PERIOD>>2); l1 < WAVE_PERIOD; l1++)
    wave_tri[l1] = -1.0 + (float)(l1 - (WAVE_PERIOD >> 1) - (WAVE_PERIOD>>2)) * dy;

  play_ports = 0;
  capt_ports = 0;
  withJack = false;
  withAlsa = false;
  for (int i = 0; i < MAX_PLAY_PORTS / 2; i++) play_mods [i] = 0;
  for (int i = 0; i < MAX_CAPT_PORTS / 2; i++) capt_mods [i] = 0;
  colorBackground = QColor(COLOR_MAINWIN_BG);
  colorModuleBackground = QColor(COLOR_MODULE_BG);
  colorModuleBorder = QColor(195, 195, 195);
  colorModuleFont = QColor(255, 255, 255);
  colorPortFont = QColor(255, 255, 255);
  colorCable = QColor(180, 180, 180);
  colorJack = QColor(250, 200, 50);

#ifdef JACK_SESSION
    jsession_ev = NULL;
    js_filename = "";
#endif
}

void SynthData::initVoices()
{
  for (int l1 = 0; l1 < poly; ++l1) {
    notes[l1] = 0;
    velocity[l1] = 0;
    noteCounter[l1] = 1000000;
    sustainNote[l1] = false;
    channel[l1] = -1;
  }
}

void SynthData::stopPCM()
{
    if (withJack)
      closeJack();
    if (withAlsa)
      closeAlsa();
}

SynthData::~SynthData()
{
    free (exp_data);
    free (wave_sine);
    free (wave_saw);
    free (wave_saw2);
    free (wave_rect);
    free (wave_tri);
    if (poly > 0)
      free(zeroModuleData[0]);
    free (zeroModuleData);
    delete (midiWidget);
    delete (guiWidget);
}

int SynthData::incModuleCount() {

  moduleCount++;
  moduleID++;
  return(0);
}

int SynthData::decModuleCount() {

  moduleCount--;
  return(0);
}

int SynthData::getModuleCount() {

  return(moduleCount);
}

int SynthData::getModuleID() {

  return moduleID;
}

int SynthData::getLadspaIDs(QString setName, QString pluginName, int *index, int *n)
{
  int subID1, subID2Name = -1, subID2Label = -1;
  QString qsn, qsl;

  setName = setName.trimmed();
  pluginName = pluginName.trimmed();
  subID1 = -1;
  QList<LadspaLib>::const_iterator li = ladspaLib.constBegin();
  for (int l1 = 0;
       li < ladspaLib.constEnd(); ++li, ++l1)
    if (setName == li->name.trimmed()) {
      subID1 = l1;

      QList<const LADSPA_Descriptor *>::const_iterator di =
	li->desc.constBegin();
      for (int l2 = 0; di < li->desc.constEnd(); ++di, ++l2) {
        qsl.sprintf("%s", (*di)->Label);
        qsn.sprintf("%s", (*di)->Name);
        if (pluginName == qsl.trimmed()) {
          subID2Label = l2;
          break;
        }
        if (pluginName == qsn.trimmed()) {
          subID2Name = l2;                            // No break to give the priority to "Label"
        }
      }
      break;
    }

  *index = subID1;
  *n = (subID2Label < 0) ? subID2Name : subID2Label; // Use "Name" only if no match for "Label"
  return( (subID1 >= 0) && ( (subID2Name >= 0) || (subID2Label >= 0) ) );
}

float SynthData::exp_table(float x) {

  int index;

  index = (int)((x + 16.0) * 1000.0);
  if (index >= EXP_TABLE_LEN) index = EXP_TABLE_LEN - 1;
  else if (index < 0) index = 0;
  return(exp_data[index]);
}

// float SynthData::exp_table_ln2(float x)
// { older version, less precise
//   int index = (int)(x * (float)(M_LN2 * 1000.0) + (float)(16.0 * 1000.0));
//   if (index >= EXP_TABLE_LEN)
//     index = EXP_TABLE_LEN - 1;
//   else
//     if (index < 0)
//       index = 0;
//   return exp_data[index];
// }
float SynthData::exp2_table(float f)
// { ultimate precision, slower
//   return __builtin_exp2f(f);
// }
{
  if (f < -16)
    return 0;

  union uf uf, uexp2;
  uf.f = f + 17;

  unsigned exp = (uf.u >> 23) - 0x7f;
  unsigned mant = uf.p.mant;

  unsigned e = exp;
  exp = 1 << e;
  exp += mant >> (23 - e);

  mant <<= e;
  mant &= (1<<23) - 1;
  mant >>= 23 - EXP2_DEPTH;

  uexp2.f = exp2_data[mant];
  uexp2.u |= (exp + 0x7F - 17) << 23;

  return uexp2.f;
}

void SynthData::create_zero_data(void)
{
  zeroModuleData = (float **) malloc(poly * sizeof(float *));
  for (int i = 0; i < poly; i++)
    if (i == 0) {
      zeroModuleData[0] = (float *) malloc(periodsize * sizeof(float));
      memset(zeroModuleData[0], 0, periodsize * sizeof(float));
    } else
      zeroModuleData[i] = zeroModuleData[0];
}


int SynthData::find_play_mod(void *M)
{
    for (int i = 0; i < play_ports / 2; i++)
        if (play_mods[i] == M)
            return i;
    return -1;
}

int SynthData::find_capt_mod(void *M)
{
    for (int i = 0; i < capt_ports / 2; i++)
        if (capt_mods[i] == M)
            return i;
    return -1;
}

void SynthData::set_capt_mod(unsigned int k, void *M)
{
  if (k < MAX_CAPT_PORTS / 2)
    capt_mods[k] = M;
}

void SynthData::set_play_mod(unsigned int k, void *M)
{
  if (k < MAX_PLAY_PORTS / 2)
    play_mods[k] = M;
}

int SynthData::initAlsa(const QString& cname, const QString& pname,
        unsigned int fsamp, snd_pcm_uframes_t frsize,
        unsigned int nfrags, int ncapt, int nplay)
{
    pthread_attr_t attr;

    withAlsa = true;
    ncapt &= ~1;
    nplay &= ~1;

#ifdef HAVE_LIBCLALSADRV
  #ifdef HAVE_CLALSADRV_API2
    alsa_handle = new Alsa_driver(
            (nplay > 0 && !pname.isEmpty()) ? pname.toLocal8Bit().constData() : 0,
            (ncapt > 0 && !cname.isEmpty()) ? cname.toLocal8Bit().constData() : 0,
            0, fsamp, frsize, nfrags);
  #else
    alsa_handle = new Alsa_driver(pname.toLocal8Bit().constData(), fsamp,
            frsize, nfrags, nplay > 0, ncapt > 0, false);
  #endif
    if (alsa_handle->stat () < 0)
#else
    /*libzita-alsa-pcmi*/
    alsa_handle = new Alsa_pcmi(
            (nplay > 0 && !pname.isEmpty()) ? pname.toLocal8Bit().constData() : 0,
            (ncapt > 0 && !cname.isEmpty()) ? cname.toLocal8Bit().constData() : 0,
            0, fsamp, frsize, nfrags, 0);
    if (alsa_handle->state() < 0)
#endif
    {
        qCritical("Can't connect to ALSA");
        return -ENODEV;
    }
    capt_ports = alsa_handle->ncapt();
    play_ports = alsa_handle->nplay();
    if (capt_ports > ncapt)
        capt_ports = ncapt;
    if (play_ports > nplay)
        play_ports = nplay;
    if (capt_ports > MAX_CAPT_PORTS)
        capt_ports = MAX_CAPT_PORTS;
    if (play_ports > MAX_PLAY_PORTS)
        play_ports = MAX_PLAY_PORTS;

#if defined (HAVE_CLALSADRV_API2) || defined (HAVE_LIBZITA_ALSA_PCMI)
    if (capt_ports > 0)
        qWarning("ALSA capture device \"%s\" opened with %d inputs",
                cname.toUtf8().constData(), capt_ports);
    if (play_ports > 0)
        qWarning("ALSA playback device \"%s\" opened with %d outputs",
                pname.toUtf8().constData(), play_ports);
#else
    qWarning("ALSA device \"%s\" opened with %d inputs "
            "and %d outputs",
            pname.toUtf8().constData(), capt_ports, play_ports);
#endif

    rate = fsamp;
    periodsize = frsize;
    cyclesize  = frsize;
    create_zero_data ();

    rlimit rlim;
    sched_param parm;
    if (getrlimit(RLIMIT_RTPRIO, &rlim))
	parm.sched_priority = sched_get_priority_max(SCHED_FIFO);
    else
	parm.sched_priority = rlim.rlim_cur;

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
    pthread_attr_setschedparam (&attr, &parm);
    pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create (&alsa_thread, &attr, alsa_static_thr_main, this))
    {
        qWarning("%s", QObject::tr("Can't create ALSA thread with RT priority")
                .toUtf8().constData());
        pthread_attr_setschedpolicy (&attr, SCHED_OTHER);
        parm.sched_priority = sched_get_priority_max (SCHED_OTHER);
        pthread_attr_setschedparam (&attr, &parm);
        if (pthread_create (&alsa_thread, &attr, alsa_static_thr_main, this))
        {
            qWarning("%s", QObject::tr("Can't create ALSA thread").toUtf8().constData());
            exit (1);
	}
    }
    return 0;
}

int SynthData::closeAlsa ()
{
    qWarning("%s", QObject::tr("Closing ALSA...").toUtf8().constData());
    withAlsa = false;
    //TODO: check pthread_join(alsa_thread, &thr_result);
    sleep (1);
    delete alsa_handle;
    return 0;
}

void *SynthData::alsa_static_thr_main (void *arg)
{
    return ((SynthData *) arg)->alsa_thr_main ();
}

void *SynthData::alsa_thr_main (void)
{
    int		i;
    unsigned int k;
    M_pcmin     *C;
    M_pcmout    *P;

    alsa_handle->pcm_start ();

    while (withAlsa)
    {
	k = alsa_handle->pcm_wait();
	pthread_mutex_lock(&rtMutex);

        readAlsaMidiEvents();

        while (k >= cyclesize)
       	{
            if (capt_ports)
	    {
                alsa_handle->capt_init (cyclesize);
                for (i = 0; i < capt_ports; i += 2)
                {
                    C = doSynthesis ? (M_pcmin *)(capt_mods [i / 2]) : 0;
                    if (C)
                    {
                        alsa_handle->capt_chan(i,     C->pcmdata[0], cyclesize);
                        alsa_handle->capt_chan(i + 1, C->pcmdata[1], cyclesize);
		    }
		}
                alsa_handle->capt_done(cyclesize);
	    }

            if (play_ports)
	    {
                alsa_handle->play_init(cyclesize);
                for (i = 0; i < play_ports; i += 2)
                {
                    P = doSynthesis ? (M_pcmout *)(play_mods [i / 2]) : 0;
                    if (P)
                    {
                        P->generateCycle();
                        alsa_handle->play_chan(i,     P->pcmdata[0], cyclesize);
                        alsa_handle->play_chan(i + 1, P->pcmdata[1], cyclesize);
		    }
	            else
                    {
                        alsa_handle->clear_chan(i, cyclesize);
                        alsa_handle->clear_chan(i + 1, cyclesize);
		    }
		}
                alsa_handle->play_done(cyclesize);
	    }

            if (doSynthesis) call_modules();

            k -= cyclesize;
	}
	pthread_mutex_unlock(&rtMutex);
    }

    alsa_handle->pcm_stop ();

    return 0;
}



int SynthData::initJack (int ncapt, int nplay)
{
    QString qs;

    withJack = true;

    play_ports = nplay & ~1;
    capt_ports = ncapt & ~1;
    if (capt_ports > MAX_CAPT_PORTS) capt_ports = MAX_CAPT_PORTS;
    if (play_ports > MAX_PLAY_PORTS) play_ports = MAX_PLAY_PORTS;

#ifdef JACK_SESSION
    if (synthoptions->global_jack_session_uuid.isEmpty())
        jack_handle = jack_client_open(name.toLatin1().constData(),
                JackNullOption, NULL);
    else
        jack_handle = jack_client_open(name.toLatin1().constData(),
                JackSessionID, NULL,
                synthoptions->global_jack_session_uuid.constData());
#else
    jack_handle = jack_client_open(name.toLatin1().constData(),
            JackNullOption, NULL);

#endif

    if (!jack_handle) {
        qWarning("%s", QObject::tr("Can't connect to JACK")
                .toUtf8().constData());
        return -ENODEV;
    }
    jack_set_process_callback (jack_handle, jack_static_callback, (void *)this);

    rate = jack_get_sample_rate (jack_handle);
    periodsize = MAXIMUM_PERIODSIZE;
    create_zero_data ();

    for (int i = 0; i < play_ports; i++)
    {
        qs.sprintf("out_%d", i);
        jack_out [i] = jack_port_register(jack_handle,
                qs.toLatin1().constData(), JACK_DEFAULT_AUDIO_TYPE,
                JackPortIsOutput, 0);
    }
    for (int i = 0; i < capt_ports; i++)
    {
        qs.sprintf("in_%d", i);
        jack_in [i] = jack_port_register (jack_handle,
                qs.toLatin1().constData(), JACK_DEFAULT_AUDIO_TYPE,
                JackPortIsInput, 0);
    }

#ifdef JACK_SESSION
    /*register jack session callback*/
    int result = 0;
    if (jack_set_session_callback)
        result = jack_set_session_callback(jack_handle,
                jack_session_static_callback, (void *) this );
    if (result != 0)
        qWarning("JACK session callback registration failed: %d", result);
    else
        if (synthoptions->verbose == 1)
            qWarning("JACK session callback registration success");
#endif

    if (jack_activate (jack_handle))
    {
        qWarning("%s", QObject::tr("Can't activate JACK")
                .toUtf8().constData());
        exit (1);
    }

    if (synthoptions->verbose == 1)
        qWarning("Connected to JACK with %d inputs and %d outputs",
            capt_ports, play_ports);

    return 0;
}

int SynthData::closeJack()
{
    if (!jack_handle)
        return 0;

    if (synthoptions->verbose == 1)
        qWarning("%s", QObject::tr("Closing JACK...").toUtf8().constData());

    int result = jack_client_close(jack_handle);
    if (result)
        qWarning("JACK client close failed: %d.", result);

    return 0;
}


#ifdef JACK_SESSION
void SynthData::jack_session_static_callback(jack_session_event_t *ev,
        void *arg)
{
    ((SynthData*) arg)->jack_session_callback(ev);
}


void SynthData::jack_session_callback(jack_session_event_t *ev)
{
    jsession_ev = ev;
    jack_session_event();
}


bool SynthData::jack_session_event()
{
    if (jsession_ev == NULL) {
        qCritical("%s", QObject::tr("No JACK session event set.")
                .toUtf8().constData());
        return false;
    }

    /* assemble new patch file name*/
    js_filename = jsession_ev->session_dir;
    js_filename += JSFILENAME;

    /* send mainwindow an action message */
    switch (jsession_ev->type) {
        case JackSessionSave:
            emit jackSessionEvent(jsaSave);
            break;
        case JackSessionSaveAndQuit:
            emit jackSessionEvent(jsaSaveAndQuit);
            break;
        default:
            qWarning("Unsupported JACK session type: %d",
                    jsession_ev->type);
            break;
    }

    /* assemble ams start command line */
    QString command = PACKAGE " \"${SESSION_DIR}\"";
    command.append(JSFILENAME);
    command.append(" -U ");
    command.append(jsession_ev->client_uuid);

    jsession_ev->command_line = strdup(command.toAscii());
    jack_session_reply(jack_handle, jsession_ev);

    /* prepare application quit if requested by jack */
    if (jsession_ev->type == JackSessionSaveAndQuit)
        emit jackSessionEvent(jsaAppQuit);

    jack_session_event_free(jsession_ev);

    return false;
}


QString SynthData::getJackSessionFilename() const
{
    return js_filename;
}
#endif


int SynthData::jack_static_callback (jack_nframes_t nframes, void *arg)
{
    return ((SynthData *) arg)->jack_callback (nframes);
}


int SynthData::jack_callback(jack_nframes_t nframes)
{
    int i, j;

    if (nframes > MAXIMUM_PERIODSIZE) {
        fprintf(stderr, "nframes exceeds allowed value %d\n", MAXIMUM_PERIODSIZE);
        return 0;
    }

    pthread_mutex_lock(&rtMutex);

    readAlsaMidiEvents();

    cyclesize = nframes;

    for (i = 0; i < capt_ports; i += 2) {
      M_pcmin *C = doSynthesis ? (M_pcmin *)(capt_mods [i / 2]) : 0;
      if (C)
	for (j = 0; j < 2; j++)
	  C->pcmdata[j] = (float *)jack_port_get_buffer(jack_in[i + j], nframes);
    }

    for (i = 0; i < play_ports; i += 2) {
      M_pcmout *P = doSynthesis ? (M_pcmout *)(play_mods [i >> 1]) : 0;
      if (P) {
	P->pcmdata[0] = (jack_default_audio_sample_t *)jack_port_get_buffer(jack_out[i], nframes);
	P->pcmdata[1] = (jack_default_audio_sample_t *)jack_port_get_buffer(jack_out[i + 1], nframes);
	P->generateCycle();
      } else
	for (j = 0; j < 2; j++) {
	  void *p = jack_port_get_buffer(jack_out [i + j], nframes);
	  memset(p, 0, sizeof(jack_default_audio_sample_t) * nframes);
	}
    }

    if (doSynthesis)
      call_modules();

   pthread_mutex_unlock(&rtMutex);
   return 0;
}

void SynthData::call_modules(void)
{
  int i;

  for (i = 0; i < wavoutModuleList.count(); i++)
    wavoutModuleList.at(i)->generateCycle();
  for (i = 0; i < scopeModuleList.count(); i++)
    scopeModuleList.at(i)->generateCycle();
#ifdef OUTDATED_CODE
  for (i = 0; i < spectrumModuleList.count(); i++)
    spectrumModuleList.at(i)->generateCycle();
#endif
  for (i = 0; i < midioutModuleList.count(); i++)
    midioutModuleList.at(i)->generateCycle();
  for (i = 0; i < moduleList.count(); i++)
    moduleList.at(i)->cycleReady = false;
  for (i = 0; i < poly; i++) {
    noteCounter[i]++;
    if (noteCounter[i] > 1000000000)
      noteCounter[i] = 1000000000;
  }
  framesDone += cyclesize;
  if (framesDone - framesGUIPinged > 2000 && pipeMessage)
    if (write(pipeFd[1], &pipeMessage, 1) == 1) {
      framesGUIPinged = framesDone;
      pipeMessage = 0;
    }
}

void SynthData::readAlsaMidiEvents(void)
{
    if (!seq_handle)
        return;

    snd_seq_event_t *ev;
    int result;

    for (int pending = snd_seq_event_input_pending(seq_handle, 1);
            pending > 0; --pending) {
        result = snd_seq_event_input(seq_handle, &ev);
        if (result < 0)
            break;
        if (ev != NULL)
            processAlsaMidiEvent(ev);
    }

    if (!setAllNotesOff)
	return;

    setAllNotesOff = false;
    for (int l2 = 0; l2 < synthdata->poly; ++l2)
	if (synthdata->noteCounter[l2] < 1000000)
	    synthdata->noteCounter[l2] = 1000000;
    noteList.reset();
}

void SynthData::processAlsaMidiEvent(snd_seq_event_t *ev)
{
    switch (ev->type) {
        case SND_SEQ_EVENT_NOTEON:
            handleMidiEventNoteOn(ev);
            break;
        case SND_SEQ_EVENT_NOTEOFF:
            handleMidiEventNoteOff(ev);
            break;
        case SND_SEQ_EVENT_CONTROLLER:
            handleMidiEventController(ev);
            break;
        case SND_SEQ_EVENT_PITCHBEND:
            handleMidiEventPitchbend(ev);
            break;
        case SND_SEQ_EVENT_PGMCHANGE:
            handleMidiEventPgmChange(ev);
            break;
        case SND_SEQ_EVENT_CHANPRESS:
            handleMidiEventChanPress(ev);
            break;
        case SND_SEQ_EVENT_CONTROL14:
            handleMidiEventControll14(ev);
            break;

	// Ignore list
	case SND_SEQ_EVENT_PORT_SUBSCRIBED:
	case SND_SEQ_EVENT_SENSING:
            break;

	default:
	    MidiControllerKey mcK(ev);
	    mckDump.put(mcK);
	    pipeMessage |= 2;
	    break;
    }
}

void SynthData::handleMidiEventNoteOn(snd_seq_event_t *ev)
{
    if (ev->data.note.velocity == 0) {
        handleMidiEventNoteOff(ev);
        return;
    }

    if (midiChannel < 0 || midiChannel == ev->data.control.channel) {
        int osc = 0;
        int noteCount = 0;
        bool foundOsc = false;

        for (int i = 0; i < poly; ++i)
            if (noteCounter[i] > noteCount) {
                noteCount = noteCounter[i];
                osc = i;
                foundOsc = true;
            }

        if (foundOsc) {
            noteCounter[osc] = 0;
            sustainNote[osc] = false;
            velocity[osc] = ev->data.note.velocity;
            channel[osc] = ev->data.note.channel;
            notes[osc] = ev->data.note.note;
	    if (poly == 1)
		noteList.pushNote(ev->data.note.note);
        }

    }

    MidiControllerContext *mcctx = getMidiControllerContext(ev);
    if (mcctx)
        mcctx->setMidiValueRT(
                (ev->data.note.velocity << 7) + ev->data.note.velocity);
}

void SynthData::handleMidiEventNoteOff(snd_seq_event_t *ev)
{
    if (midiChannel < 0 || midiChannel == ev->data.control.channel) {
	for (int i = 0; i < poly; ++i)
	    if (channel[i] == ev->data.note.channel) {
		if (poly == 1)
		    noteList.deleteNote(ev->data.note.note);
		if (notes[i] == ev->data.note.note && noteCounter[i] < 1000000) {
		    if (poly == 1 && noteList.anyNotesPressed()) {
			notes[i] = noteList.lastNote();
			noteCounter[i] = 0;
		    } else
			if (sustainFlag)
			    sustainNote[i] = true;
			else
			    noteCounter[i] = 1000000;
		}
	    }
    }
    MidiControllerContext *mcctx = getMidiControllerContext(ev);
    if (mcctx)
        mcctx->setMidiValueRT(0);
}

void SynthData::handleMidiEventPgmChange(snd_seq_event_t *ev)
{
    guiWidget->setCurrentPreset(ev->data.control.value, true);
    pipeMessage |= 4;
}

void SynthData::handleMidiEventController(snd_seq_event_t *ev)
{
    MidiControllerContext* mcctx = getMidiControllerContext(ev);
    if (mcctx != NULL)
        mcctx->setMidiValueRT(
                (ev->data.control.value << 7) + ev->data.control.value);

    if (ev->data.control.param == MIDI_CTL_ALL_NOTES_OFF)
        for (int i = 0; i < poly; ++i)
            if (noteCounter[i] < 1000000 && channel[i] == ev->data.note.channel)
                noteCounter[i] = 1000000;

    else if (ev->data.control.param == MIDI_CTL_SUSTAIN) {
        bool sustainFlag = ev->data.control.value > 63;
        if (!sustainFlag)
            for (int i = 0; i < poly; ++i)
                if (sustainNote[i])
                    noteCounter[i] = 1000000;
    }

    for (int i = 0; i < synthdata->listM_advmcv.count(); ++i)
        synthdata->listM_advmcv.at(i)->controllerEvent(
                ev->data.control.param, ev->data.control.value);
}

void SynthData::handleMidiEventPitchbend(snd_seq_event_t *ev)
{
    MidiControllerContext* mcctx = getMidiControllerContext(ev);
    if (mcctx != NULL)
        mcctx->setMidiValueRT(ev->data.control.value + 8192);

    for (int i = 0; i < synthdata->listM_advmcv.count(); ++i)
        synthdata->listM_advmcv.at(i)->pitchbendEvent(
                ev->data.control.value);
}

void SynthData::handleMidiEventChanPress(snd_seq_event_t *ev)
{
    for (int i = 0; i < synthdata->listM_advmcv.count(); ++i)
        synthdata->listM_advmcv.at(i)->aftertouchEvent(
                ev->data.control.value);
}

void SynthData::handleMidiEventControll14(snd_seq_event_t *ev)
{
    MidiControllerContext* mcctx = getMidiControllerContext(ev);
    if (mcctx != NULL)
        mcctx->setMidiValueRT(ev->data.control.value);
}

MidiControllerContext* SynthData::getMidiControllerContext(snd_seq_event_t *ev)
{
    MidiControllerContext* result = NULL;
    MidiControllerKey mcK(ev);

    typeof(activeMidiControllers->constBegin()) mc =
        qBinaryFind(activeMidiControllers->constBegin(),
                activeMidiControllers->constEnd(), mcK);
    if (mc != activeMidiControllers->constEnd())
        result = mc->context;
    if (midiWidget->isVisible()) {
        mckRead.put(mcK);
        pipeMessage |= 2;
    }
    return result;
}

