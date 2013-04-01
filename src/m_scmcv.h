#ifndef M_SCMCV_H
#define M_SCMCV_H

#include <QFileDialog>
#include "module.h"


#define MODULE_SCMCV_HEIGHT               120
#define MODULE_SCMCV_RESPONSE              32

class M_scmcv : public Module
{
  Q_OBJECT

  private:
    float pitchbend;
    float scale_lut[128];
    bool scale_lut_isRatio[128];
    int scale_lut_length, pitch;
    float scale_notes[128];
    Port *port_note_out, *port_gate_out, *port_velocity_out, *port_trig_out;
    QFileDialog *fileDialog;

  private:
    void calcScale();

  public:
    int base, lastbase, channel;
    QString sclname, dirpath;
    PolyArr<float> freq, lastfreq;
 
  public:
    M_scmcv(QWidget* parent=0, QString *p_sclname = 0);

    void generateCycle();

  public slots:
    void loadScale(const QString &p_sclname);
    void openBrowser();
};

#endif
