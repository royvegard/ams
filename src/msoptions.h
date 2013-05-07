/* msoptions.h 
 * This file defines the struct for modular synth options
*/
#ifndef MSOPTIONS_H
#define MSOPTIONS_H

#include <alsa/asoundlib.h>
#include <QString>
#include "config.h"


struct ModularSynthOptions {
    QString synthName;
    QString cname;
    QString pname;
    QString presetName;
    QString presetPath;
    snd_pcm_uframes_t frsize;
    unsigned int fsamp;
    unsigned int nfrags;
    int ncapt;
    int nplay;
    int poly;
    int rcFd;
    bool noGui;
    bool havePreset;
    bool havePresetPath;
    bool forceJack;
    bool forceAlsa;
    float edge;
    int verbose;
#ifdef JACK_SESSION
    QString global_jack_session_uuid;
#endif
#ifdef NSM_SUPPORT
    QString execName;
#endif
};

#endif    // MSOPTIONS_H
