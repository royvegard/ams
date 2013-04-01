#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qsizepolicy.h>
#include <qlineedit.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "configdialog.h"
#include "midislider.h"
#include "intmidislider.h"
#include "midicheckbox.h"
#include "midicombobox.h"
#include "midicontrollable.h"
#include "midipushbutton.h"
#include "envelope.h"
#include "multi_envelope.h"
#include "scopescreen.h"
#include "spectrumscreen.h"
#include "function.h"


ConfigDialog::ConfigDialog(Module &module)
  : QDialog(&module),
    addStretch(1)
  , removeButton(new QPushButton(tr("&Remove Module")))
  , removeFrame(new QHBoxLayout())
  , module(module)
{
  QVBoxLayout *vBox = new QVBoxLayout();
  setLayout(vBox);
  vBox->setMargin(0);
  vBox->setSpacing(0);
  configBox = new QVBoxLayout();
  configBox->setMargin(5);
  configBox->setSpacing(5);
  vBox->addLayout(configBox);

  tabWidget = NULL;
  removeFrame->addStretch();
  removeFrame->addWidget(removeButton);
  removeFrame->addStretch();
  vBox->addLayout(removeFrame);
  removeButtonShow(true);
  QObject::connect(removeButton, SIGNAL(clicked()),
          this, SLOT(removeButtonClicked()));
}

ConfigDialog::~ConfigDialog()
{}

MidiSlider *ConfigDialog::addSlider(const QString &name, float &valueRef, float minValue, float maxValue, bool isLog, QBoxLayout *layout)
{
  MidiControllableFloat * mcAble =
    new MidiControllableFloat(module, name, valueRef, minValue, maxValue, isLog);

  MidiSlider *midiSlider = new MidiSlider(*mcAble);
  insertWidget(layout, midiSlider);

  midiSliderList.append(midiSlider);

  return midiSlider;
}

int ConfigDialog::addFloatIntSlider(const QString &name,
        float &valueRef, float minValue, float maxValue, QBoxLayout *layout)
{
  MidiControllable<float> *mcAble =
    new MidiControllable<float>(module, name, valueRef, minValue, maxValue);

  IntMidiSlider *intMidiSlider = new IntMidiSlider(
          GUIcomponentType_floatintslider, *mcAble);
  insertWidget(layout, intMidiSlider);

  floatIntMidiSliderList.append(intMidiSlider);

  return 0;
}

IntMidiSlider *ConfigDialog::addIntSlider(const QString &name, int &valueRef, int minValue, int maxValue, QBoxLayout *layout)
{
  MidiControllable<int> * mcAble =
    new MidiControllable<int>(module, name, valueRef, minValue, maxValue);

  IntMidiSlider *intMidiSlider = new IntMidiSlider(GUIcomponentType_intslider, *mcAble);
  insertWidget(layout, intMidiSlider);

  intMidiSliderList.append(intMidiSlider);

  return intMidiSlider;
}

MidiComboBox *ConfigDialog::addComboBox(const QString &name, int &valueRef, const QStringList &itemNames, QBoxLayout *layout) {
  MidiControllableNames * mcAble =
    new MidiControllableNames(module, name, valueRef, itemNames);

  MidiComboBox *midiComboBox = new MidiComboBox(*mcAble);
  insertWidget(layout, midiComboBox, 0, Qt::AlignCenter);

  midiComboBoxList.append(midiComboBox);

  return midiComboBox;
}

int ConfigDialog::addCheckBox(const QString &name, float &valueRef, QBoxLayout *layout)
{
  MidiControllable<float> *mcAble =
    new MidiControllable<float>(module, name, valueRef, 0, 1);

  return addCheckBox(*mcAble, layout);
}

int ConfigDialog::addCheckBox(MidiControllable<float> &mcAble, QBoxLayout *layout)
{
  MidiCheckBox *midiCheckBox;

  midiCheckBox = new MidiCheckBox(mcAble);
  insertWidget(layout, midiCheckBox, 0, Qt::AlignCenter);

  midiCheckBoxList.append(midiCheckBox);

  return 0;
}

MidiControllableDoOnce *ConfigDialog::addPushButton(const QString &name, QBoxLayout *layout)
{
  MidiControllableDoOnce *mcAble =
    new MidiControllableDoOnce(module, name);

  MidiPushButton *midiPushButton;

  midiPushButton = new MidiPushButton(*mcAble);
  insertWidget(layout, midiPushButton, 0, Qt::AlignCenter);

  midiPushButtonList.append(midiPushButton);

  return mcAble;
}

int ConfigDialog::addEnvelope(MidiControllableFloat &delayRef, MidiControllableFloat &attackRef, MidiControllableFloat &holdRef,
			      MidiControllableFloat &decayRef, MidiControllableFloat &sustainRef, MidiControllableFloat &releaseRef, QBoxLayout *layout)
{
  Envelope *envelope = new Envelope(delayRef, attackRef, holdRef, decayRef, sustainRef, releaseRef);
  insertWidget(layout, envelope, 100, 0, 0);

  envelopeList.append(envelope);
  return(0);
}

MultiEnvelope *ConfigDialog::addMultiEnvelope(int envCount, float *timeScaleRef, float *attackRef, float *sustainRef, float *releaseRef, QBoxLayout *layout) {

  MultiEnvelope *envelope;

  envelope = new MultiEnvelope(envCount, timeScaleRef, attackRef, sustainRef, releaseRef);
  insertWidget(layout, envelope, 100);

  return envelope;
}

int ConfigDialog::addLabel(const QString& label, QBoxLayout *layout) {

  QLabel *configLabel;

  configLabel = new QLabel();
  insertWidget(layout, configLabel);

  configLabel->setAlignment(/*Qt::WordBreak |*/ Qt::AlignLeft);
  configLabel->setText(label);
  labelList.append(configLabel);
  return(0);
}

void ConfigDialog::removeButtonClicked() {

  emit removeModuleClicked();
}

int ConfigDialog::initTabWidget() {

  tabWidget = new QTabWidget();
  configBox->insertWidget(configBox->count() - 1, tabWidget);
  return 0;
}

QHBoxLayout *ConfigDialog::addHBox(QBoxLayout *layout)
{
  QHBoxLayout *hbox;

  if (!layout)
    layout = configBox;

  hbox = new QHBoxLayout();
  layout->addLayout(hbox);
  if (addStretch > 0)
    layout->addStretch(addStretch);

  return hbox;
}

QVBoxLayout *ConfigDialog::addVBox(QBoxLayout *layout)
{
  QVBoxLayout *vbox;

  if (!layout)
    layout = configBox;

  vbox = new QVBoxLayout();
  layout->addLayout(vbox);
  if (addStretch > 0)
    layout->addStretch(addStretch);

  return vbox;
}

QVBoxLayout *ConfigDialog::addVBoxTab(const char *tabLabel)
{
  return addVBoxTab(QString(tabLabel));
}


QVBoxLayout *ConfigDialog::addVBoxTab(const QString &tabLabel)
{
  QWidget *w = new QWidget();
  tabWidget->addTab(w, tabLabel);
  return new QVBoxLayout(w);
}

int ConfigDialog::addLineEdit(const char *Name, QBoxLayout *layout) {

  QLineEdit *lineEdit;

  QHBoxLayout *line = addHBox(layout);

//   line->setSpacing(5);
//   line->setMargin(10);
  QLabel *nameLabel = new QLabel();
  nameLabel->setText(Name);
  line->addWidget(nameLabel);
  lineEdit = new QLineEdit();
  lineEditList.append(lineEdit);
  line->addWidget(lineEdit);
  nameLabel->setBuddy(lineEdit);

  return 0;
}

int ConfigDialog::addScopeScreen(float &timeScaleRef, int &modeRef, int &edgeRef, int &triggerModeRef,
                                 float &triggerThrsRef, float &zoomRef, QBoxLayout *layout) {

  ScopeScreen *scopeScreen;

  scopeScreen = new ScopeScreen(timeScaleRef, modeRef, edgeRef, triggerModeRef, triggerThrsRef, zoomRef);
  insertWidget(layout, scopeScreen, 1);

  scopeScreenList.append(scopeScreen);
  return(0);
}

#ifdef OUTDATED_CODE
int ConfigDialog::addSpectrumScreen(QBoxLayout *layout) {

  SpectrumScreen *spectrumScreen;

  if (!parent) {
    spectrumScreen = new SpectrumScreen(NULL, "Spectrum");
  } else {
    spectrumScreen = new SpectrumScreen(parent, "Spectrum");
  }
  spectrumScreenList.append(spectrumScreen);
  return(0);
}
#endif

int ConfigDialog::addFunction(int p_functionCount, int *p_mode,
        int *p_editIndex, tFunction &point, int p_pointCount, QBoxLayout *layout) {

  Function *function;

  function = new Function(p_functionCount, p_mode, p_editIndex, point, p_pointCount, NULL);
  insertWidget(layout, function, 1);

  functionList.append(function);
  return(0);
}

void ConfigDialog::insertWidget(QBoxLayout *layout, QWidget *widget,
				int stretch, Qt::Alignment alignment, int pos)
{
  if (!layout)
    layout = configBox;

  layout->insertWidget(pos, widget, stretch, alignment);
  if (addStretch > 0)
    layout->addStretch(addStretch);
}

void ConfigDialog::removeButtonShow(bool show)
{
  removeFrame->setEnabled(show);
  removeFrame->setMargin(show ? 5 : 0);
  removeButton->setVisible(show);
}

MidiSlider* ConfigDialog::getMidiSlider(int idx)
{
    MidiSlider* ms = NULL;

    if ((idx + 1) > midiSliderList.count())
        qWarning("Slider index out of range (value = %d)", idx);
    else
        ms = midiSliderList.at(idx);
    return ms;
}

IntMidiSlider* ConfigDialog::getIntMidiSlider(int idx)
{
    IntMidiSlider* ims = NULL;

    if ((idx + 1) > intMidiSliderList.count())
        qWarning("Integer slider index out of range (value = %d)", idx);
    else
        ims = intMidiSliderList.at(idx);
    return ims;
}

IntMidiSlider* ConfigDialog::getFloatIntMidiSlider(int idx)
{
    IntMidiSlider* ims = NULL;

    if ((idx + 1) > floatIntMidiSliderList.count())
        qWarning("Float slider index out of range (value = %d)", idx);
    else
        ims = floatIntMidiSliderList.at(idx);
    return ims;
}

MidiComboBox* ConfigDialog::getMidiComboBox(int idx)
{
    MidiComboBox* mcb = NULL;

    if ((idx + 1) > midiComboBoxList.count())
        qWarning("ComboBox index out of range (value = %d)", idx);
    else
        mcb = midiComboBoxList.at(idx);
    return mcb;
}

MidiCheckBox* ConfigDialog::getMidiCheckBox(int idx)
{
    MidiCheckBox* mcb = NULL;

    if ((idx + 1) > midiCheckBoxList.count())
        qWarning("CheckBox index out of range (value = %d)", idx);
    else
        mcb = midiCheckBoxList.at(idx);
    return mcb;
}

MidiPushButton* ConfigDialog::getMidiPushButton(int idx)
{
    MidiPushButton* mpb = NULL;

    if ((idx + 1) > midiPushButtonList.count())
        qWarning("PushButton index out of range (value = %d)", idx);
    else
        mpb = midiPushButtonList.at(idx);
    return mpb;
}

Function* ConfigDialog::getFunction(int idx)
{
    Function* fnc = NULL;

    if ((idx + 1) > functionList.count())
        qWarning("Function index out of range (value = %d)", idx);
    else
        fnc = functionList.at(idx);
    return fnc;
}
