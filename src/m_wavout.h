#ifndef M_WAVOUT_H
#define M_WAVOUT_H

#include <QFile>
#include "module.h"


#define MODULE_WAVOUT_HEIGHT                80

class M_wavout : public Module
{
  Q_OBJECT

  private:
    QFile wavfile;
    long wavDataSize;
    float gain;
    float mixer_gain[2]; 
    int agc;
    float doRecord;
    QString wavname;
    QTimer *timer;
    Port *port_in[2];
    char outbuf[8];
    char *wavdata;
    float *floatdata;
    
  public: 
    float **inData[2];
                            
  public:
    M_wavout(QWidget* parent=0);
    ~M_wavout();

    void generateCycle();
    int setGain(float p_gain);
    float getGain();

  public slots:
    void recordToggled(bool on);
    void recordClicked();
    void openBrowser();
    void stopClicked();
    void createWav();
    void timerProc();
};
  
#endif
