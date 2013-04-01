#ifndef __MAIN_H
#define __MAIN_H

#define AMS_LONGNAME "AlsaModularSynth"
#define AMS_SHORTNAME "AMS"

// Compile time options for AlsaModularSynth

#define DEFAULT_SETUP

#ifdef DEFAULT_SETUP

// Default setup - do not modify - change the local setup below.

// Functional
#define DEFAULT_PCMNAME   "plughw:0,0"
#define DEFAULT_RATE            44100
#define DEFAULT_PERIODSIZE       1024
#define MAXIMUM_PERIODSIZE       4096
#define DEFAULT_PERIODS             2
#define DEFAULT_CAPT_PORTS          2
#define DEFAULT_PLAY_PORTS          2
#define MAX_CAPT_PORTS             32
#define MAX_PLAY_PORTS             32
#define MAXPOLY                   128
#define MAX_SO                    256
#define WAVE_PERIOD             65536
#define M_MCV_MAX_FREQ        20000.0
#define EXP_TABLE_LEN           32768
#define MAX_LADPSA_CONTROLS_PER_TAB 8
#define MAX_ANALOGUE_DRIVER_OUT     6      

// Appearance
#define SYNTH_DEFAULT_WIDTH       750
#define SYNTH_DEFAULT_HEIGHT      500
#define SYNTH_MINIMUM_WIDTH       200
#define SYNTH_MINIMUM_HEIGHT      150
#define COLOR_MAINWIN_BG     0x825A31
#define COLOR_CONNECT_LINE   0xDCD8D8
#define COLOR_CONNECT_BEZ1   0x968C8C
#define COLOR_CONNECT_BEZ2   0xB4AAAA
#define COLOR_CONNECT_BEZ3   0xDCD8D8
#define COLOR_MODULE_BG      0x4D4640

#else

// Local setup. Modify these and comment out the #define DEFAULT_SETUP.

// Functional
#define DEFAULT_PCMNAME           "TT"
#define DEFAULT_RATE            48000
#define DEFAULT_PERIODSIZE        256
#define MAXIMUM_PERIODSIZE       1024
#define DEFAULT_PERIODS             4
#define DEFAULT_CAPT_PORTS          8
#define DEFAULT_PLAY_PORTS          8
#define MAX_CAPT_PORTS             32
#define MAX_PLAY_PORTS             32
#define MAXPOLY                    32
#define MAX_SO                    256
#define WAVE_PERIOD             65536
#define M_MCV_MAX_FREQ        20000.0
#define EXP_TABLE_LEN           32768

// Appearance
#define SYNTH_DEFAULT_WIDTH       750
#define SYNTH_DEFAULT_HEIGHT      500
#define SYNTH_MINIMUM_WIDTH       200
#define SYNTH_MINIMUM_HEIGHT      150
#define COLOR_MAINWIN_BG     0xD0D0D0
#define COLOR_CONNECT_LINE   0x000000
#define COLOR_CONNECT_BEZ1   0x606060
#define COLOR_CONNECT_BEZ2   0x909090
#define COLOR_CONNECT_BEZ3   0xC0C0C0
#define COLOR_MODULE_BG      0x6060E0

#endif

#endif

