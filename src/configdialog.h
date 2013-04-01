#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <QVBoxLayout>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <QPointF>
#include <alsa/asoundlib.h>

#include "synthdata.h"
#include "function.h"

template <typename t> class MidiControllable;

/** configuration dialog for each module
 *
 */ 
class ConfigDialog : public QDialog
{
    Q_OBJECT

    QVBoxLayout *configBox;
    int addStretch;
    QPushButton *removeButton;
    QHBoxLayout *removeFrame;

  public: 
    QList<class MidiSlider*> midiSliderList; 
    QList<class IntMidiSlider*> intMidiSliderList; 
    QList<class IntMidiSlider*> floatIntMidiSliderList; 
    QList<class MidiComboBox*> midiComboBoxList;
    QList<class MidiCheckBox*> midiCheckBoxList;
    QList<class MidiPushButton*> midiPushButtonList;
    QList<class Envelope*> envelopeList;
    QList<class ScopeScreen*> scopeScreenList;
#ifdef OUTDATED_CODE
    QList<class SpectrumScreen*> spectrumScreenList;
#endif
    QList<class Function*> functionList;
    QList<class QLineEdit*> lineEditList;
    QList<class QLabel*> labelList;
    Module &module;
    QTabWidget *tabWidget;

  protected:
    void insertWidget(QBoxLayout *layout, QWidget *widget,
            int stretch = 0, Qt::Alignment alignment = 0, int pos = -1);

  public:
    ConfigDialog(Module &module);
    ~ConfigDialog();
    void setAddStretch(int v) {
        addStretch = v;
    }
    void removeButtonShow(bool show);
    MidiSlider *addSlider(const QString &name, float &valueRef,
            float minValue, float maxValue, bool isLog = false,
            QBoxLayout *layout = NULL);
    IntMidiSlider *addIntSlider(const QString &name, int &valueRef,
            int minValue, int maxValue, QBoxLayout *layout = NULL);
    int addFloatIntSlider(const QString &name, float &valueRef,
            float minValue, float maxValue, QBoxLayout *layout = NULL);
    MidiComboBox *addComboBox(const QString &name, int &valueRef,
            const QStringList &itemNames, QBoxLayout *layout = NULL);
    int addCheckBox(const QString &name, float &valueRef,
            QBoxLayout *layout = NULL);
    int addCheckBox(MidiControllable<float> &mcAble, QBoxLayout *layout = NULL);
    class MidiControllableDoOnce *addPushButton(const QString &name,
            QBoxLayout *layout = NULL);
    int addEnvelope(class MidiControllableFloat &delayRef,
            MidiControllableFloat &attackRef, MidiControllableFloat &holdRef, 
            MidiControllableFloat &decayRef, MidiControllableFloat &sustainRef,
            MidiControllableFloat &releaseRef, QBoxLayout *layout = NULL);
    class MultiEnvelope *addMultiEnvelope(int envCount, float *timeScaleRef,
            float *attackRef, float *sustainRef, float *releaseRef,
            QBoxLayout *layout = NULL);
    int addFunction(int p_functionCount, int *p_mode, int *p_editIndex,
            tFunction &, int p_pointCount, QBoxLayout *layout = NULL);
    int addLabel(const QString& label, QBoxLayout *layout = NULL);
    int addScopeScreen(float &timeScaleRef, int &modeRef, int &edgeRef,
            int &triggerModeRef, float &triggerThrsRef,
            float &zoomRef, QBoxLayout *layout = NULL);
#ifdef OUTDATED_CODE
    int addSpectrumScreen(QWidget *parent=0);
#endif
    int addTab(QWidget *tabPage, const QString &tabLabel) {
        return tabWidget->addTab(tabPage, tabLabel);
    }
    QVBoxLayout *addVBoxTab(const char *tabLabel);
    QVBoxLayout *addVBoxTab(const QString &tabLabel);
    QHBoxLayout *addHBox(QBoxLayout *layout = NULL);
    QVBoxLayout *addVBox(QBoxLayout *layout = NULL);
    int addLineEdit(const char *lineName, QBoxLayout *layout = NULL);
    int initTabWidget();
    MidiSlider* getMidiSlider(int);
    IntMidiSlider* getIntMidiSlider(int); 
    IntMidiSlider* getFloatIntMidiSlider(int); 
    MidiComboBox* getMidiComboBox(int);
    MidiCheckBox* getMidiCheckBox(int);
    MidiPushButton* getMidiPushButton(int);
    Function* getFunction(int);

  signals:
    void removeModuleClicked();

  public slots: 
    void removeButtonClicked();
};
  
#endif
