#ifndef MODULARSYNTH_H
#define MODULARSYNTH_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qlist.h>
#include <qmessagebox.h>
#include <QMainWindow>
#include <qsocketnotifier.h>
#include <qpoint.h>
#include <qcolor.h>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QTextStream>
#include <alsa/asoundlib.h>

#include "config.h"
#include "main.h"
#include "synthdata.h"
#include "module.h"
#include "port.h"
#include "port_popup.h"
#include "midiwidget.h"
#include "msoptions.h"
#include "ladspadialog.h"

extern QTextStream StdErr;
extern QTextStream StdOut;

enum connectorStyleType {CONNECTOR_STRAIGHT, CONNECTOR_BEZIER};


class ModularSynth : public QWidget
{
  Q_OBJECT

    QList<Module*> listModule;
    QList<class TextEdit*> listTextEdit;
    connectorStyleType connectorStyle;    
    Port *selectedPort;
    QSocketNotifier *seqNotifier;
    LadspaDialog *ladspaDialog;
    MidiWidget *midiWidget;
    class GuiWidget *guiWidget;
    bool loadingPatch;
    QString cname;
    QString pname;
    unsigned int fsamp;
    snd_pcm_uframes_t frsize;
    unsigned int nfrags;
    int   ncapt;
    int   nplay;
    int   verbose;
    bool paintFastly;
    double _zoomFactor;
    QPoint newBoxPos;
    QPoint lastMousePos;
    bool enablemodulegrid;
    int modulegrid;

    void initNewModule(Module *m);
    snd_pcm_t *open_pcm(bool openCapture);
    snd_seq_t *open_seq(); 
    void initSeqNotifier();  
    void newM_mix(int in_channels);
    void newM_function(int functionCount);
    void newM_stereomix(int in_channels);
    void newM_seq(int seqLen);
    void newM_vcorgan(int oscCount);
    void newM_dynamicwaves(int oscCount);
    void newM_ad(int outCount);
    void new_textEdit(int w, int h);
    void showContextMenu(const QPoint&);
            
public:
    ModularSynth(QWidget* parent, const ModularSynthOptions&);
    ~ModularSynth();

    bool clearConfig(bool restart);
    QMenu *contextMenu;
    int go(bool forceJack, bool forceAlsa);
    void setPaintFastly(bool v) {
      paintFastly = v;
    }
    void moveAllBoxes(const QPoint &delta);
    bool isModified();
    QString getColorPath();
    void setColorPath(const QString& sp);
    QString getPatchPath();
    void setPatchPath(const QString& sp);
    int getSynthDataPoly();
    QSize sizeHint() const;
#ifdef JACK_SESSION
    QString getJackSessionFilename() const;
#endif
    QColor getBackgroundColor() const;
    QColor getModuleBackgroundColor() const;
    QColor getModuleBorderColor() const;
    QColor getModuleFontColor() const;
    QColor getPortFontColor() const;
    QColor getCableColor() const;
    QColor getJackColor() const;
    void setBackgroundColor(QColor color);
    void setModuleBackgroundColor(QColor color);
    void setModuleBorderColor(QColor color);
    void setModuleFontColor(QColor color);
    void setPortFontColor(QColor color);
    void setCableColor(QColor color);
    void setJackColor(QColor color);
    int getMidiControllerMode();
    void setMidiControllerMode(int mode);
    int getModuleMoveMode();
    void setModuleMoveMode(int mode);
    int getModuleGrid();
    void setModuleGrid(int grid);
    bool getEnableModuleGrid();
    void setEnableModuleGrid(bool enable);
    void loadPreference(QString&);
    void savePreferences(QTextStream&);

  protected:
    class QAbstractScrollArea *scrollArea() {
      return (QAbstractScrollArea *)parent();
    }
    void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);

  private:
    QWidget* dragWidget;
    bool modified;
    PopupMenu* portPopup;
    Module* getModuleWithId(int);
    TextEdit* getTextEditAt(int);
    
  public slots: 
    void displayMidiController();
    void displayParameterView();
    void displayLadspaPlugins();
    void midiAction(int fd);
    void startSynth();
    void stopSynth();
    void new_textEdit();
    void newM_ad_2();
    void newM_ad_4();
    void newM_ad_6();
    void newM_advenv();
    void newM_advmcv();
    void newM_amp();
    void newM_analogmemory();
    void newM_bitgrind();
    void newM_conv();
    void newM_cvs();
    void newM_delay();
    void newM_env();
    void newM_function_1();
    void newM_function_2();
    void newM_function_4();
    void newM_hysteresis();
    void newM_inv();
    void newM_ladspa(int p_ladspaDesFuncIndex, int n,
            bool p_newLadspaPoly, bool p_extCtrlPorts);
    void newM_lfo();
    void newM_mcv();
    void newM_mix_2();
    void newM_mix_4();
    void newM_mix_8();
    void newM_mphlfo();
    void newM_noise();
    void newM_noise2();
    void newM_quantizer();
    void newM_ringmod();
    void newM_scmcv();
    void newM_scmcv(QString *p_scalaName);
    void newM_scquantizer();
    void newM_scquantizer(QString *p_scalaName);
    void newM_seq_12();
    void newM_seq_16();
    void newM_seq_24();
    void newM_seq_32();
    void newM_seq_8();
    void newM_slew();
    void newM_stereomix_2();
    void newM_stereomix_4();
    void newM_stereomix_8();
    void newM_v8sequencer();
    void newM_vca_exp();
    void newM_vca_lin();
    void newM_vcdelay();
    void newM_vcdoubledecay();
    void newM_vcenv();
    void newM_vcenv2();
    void newM_vcf();
    void newM_vco();
    void newM_vco2();
    void newM_vcpanning();
    void newM_vquant();
    void newM_vocoder();
    void newM_dynamicwaves_4();
    void newM_dynamicwaves_6();
    void newM_dynamicwaves_8();
    void newM_midiout();
    void newM_pcmin();
    void newM_pcmout();
    void newM_scope();
    void newM_sh();
    void newM_spectrum();
    void newM_vcorgan_4();
    void newM_vcorgan_6();
    void newM_vcorgan_8();
    void newM_vcswitch();
    void newM_wavout();
    void resizeTextEdit(const QPoint& pos);
    void portSelected(Port*);
    void deleteModule();
    void deleteTextEdit();
    void deleteTextEdit(TextEdit *te);
    void load(QTextStream&);
    void save(QTextStream&);
    void loadColors();
    void saveColors();
    void allVoicesOff();
    void refreshColors();
    void redrawPortConnections();
};

  
#endif
