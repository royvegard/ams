#ifndef M_SCQUANTIZER_H
#define M_SCQUANTIZER_H

#include <QFileDialog>
#include "module.h"


#define MODULE_SCQUANTIZER_HEIGHT               140

class M_scquantizer : public Module
{
  Q_OBJECT

  private:
    Port *port_M_in, *port_M_trigger, *port_M_transpose, *port_out, *port_trigger_out;
    PolyArr<int> trigCount;
    PolyArr<bool> trigger;
    int quantum;

    float scale_lut[128];
    bool scale_lut_isRatio[128];
    int scale_lut_length;
    float scale_notes[128];
    PolyArr<float> qsig;
    QFileDialog *fileDialog;
                           
  private:
    void calcScale();
                                  
  public: 
    int base, lastbase;
    QString sclname, dirpath;
    float **inData, **triggerData, **transposeData;       
                            
  public:
    M_scquantizer(QWidget* parent=0, QString *p_sclname = 0);

    void generateCycle();

  public slots:
    void loadScale(const QString &p_sclname);
    void openBrowser();
};
  
#endif
