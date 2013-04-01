#include <QGroupBox>
#include <QTextStream>
#include <alsa/asoundlib.h>

#include "synthdata.h"
#include "guiwidget.h"
#include "midicontroller.h"
#include "midiguicomponent.h"
#include "module.h"
#include "midislider.h"
#include "intmidislider.h"
#include "midicombobox.h"
#include "midicheckbox.h"
#include "midipushbutton.h"


GuiWidget::GuiWidget(QWidget* parent, const char *name)
    : QDialog(parent), vLayout(this)
{
    setObjectName(name);

    setGeometry(0, 0, GUI_DEFAULT_WIDTH, GUI_DEFAULT_HEIGHT);


    QWidget *presetContainer = new QWidget();
    QHBoxLayout *presetContainerLayout = new QHBoxLayout(presetContainer);
    vLayout.addWidget(presetContainer);

    QWidget *presetNameContainer = new QWidget();
    QHBoxLayout *presetNameContainerLayout = new QHBoxLayout(
            presetNameContainer);
    vLayout.addWidget(presetNameContainer);

    tabWidget = new QTabWidget();
    vLayout.addWidget(tabWidget);
    currentTab = NULL;
    currentGroupBox = NULL;
    currentTabIndex = 0;

    QPushButton *addPresetButton = new QPushButton(tr("&Add Preset"));
    presetContainerLayout->addWidget(addPresetButton);
    QObject::connect(addPresetButton, SIGNAL(clicked()),
            this, SLOT(addPreset()));
    QPushButton *overwritePresetButton =
        new QPushButton(tr("&Overwrite Preset"));
    presetContainerLayout->addWidget(overwritePresetButton);
    QObject::connect(overwritePresetButton, SIGNAL(clicked()),
            this, SLOT(overwritePreset()));
    presetCountLabel = new QLabel();
    presetCountLabel->setText(tr("Presets for this configuration: 0"));
    presetContainerLayout->addWidget(presetCountLabel);
    QPushButton *decButton = new QPushButton("-1");
    presetContainerLayout->addWidget(decButton);
    QObject::connect(decButton, SIGNAL(clicked()), this, SLOT(presetDec()));
    QPushButton *incButton = new QPushButton("+1");
    presetContainerLayout->addWidget(incButton);
    QObject::connect(incButton, SIGNAL(clicked()), this, SLOT(presetInc()));

    presetLabel = new QLabel();
    presetNameContainerLayout->addWidget(presetLabel);
    presetName = new QLineEdit();
    presetNameContainerLayout->addWidget(presetName);

    setPresetCount(0);
    setCurrentPreset(0);
}

int GuiWidget::addFrame(const QString &frameName)
{
    if (!currentTab)
        return -1;

    frameNameList.append(frameName);
    QGroupBox *gbox = new QGroupBox(frameName);
    gbox->setObjectName(frameName);
    currentTab->addWidget(gbox);
    QVBoxLayout *vbox = new QVBoxLayout(gbox);

    GuiFrame *guiFrame = new GuiFrame;
    guiFrame->frameBox = vbox;
    guiFrame->tabIndex = currentTabIndex;
    frameBoxList.append(guiFrame);
    currentGroupBox = vbox;
    return(0);
}

int GuiWidget::setFrame(int index) {

    GuiFrame* gf = getGuiFrame(index);
    if (gf != NULL)
        currentGroupBox = gf->frameBox;
    return(0);
}

int GuiWidget::addTab(const QString &tabName)
{
    tabNameList.append(tabName);
    QWidget *tab = new QWidget();
    setObjectName(tabName);
    currentTab = new QHBoxLayout(tab);
    currentTabIndex = tabNameList.count() - 1;
    tabWidget->insertTab(-1, tab, tabName);
    tabList.append(currentTab);
    return(0);
}

int GuiWidget::setTab(int index)
{
    currentTab = tabList.at(index);
    currentTabIndex = index;
    return(0);
}

int GuiWidget::addParameter(MidiControllableBase *mcAble,
        const QString &parameterName)
{
    if (currentGroupBox == NULL) {
        qWarning("No current group box found.");
        return -1;
    }

    if (mcAble == NULL) {
        qWarning("No MIDI controlable item found.");
        return -1;
    }

    clearPresets();

    MidiGUIcomponent *mgc = dynamic_cast<MidiGUIcomponent *>(
            mcAble->mcws.at(0))->createTwin();

    if (mgc == NULL) {
        qWarning("No GUI component for MIDI controlable item found.");
        return -1;
    }

    currentGroupBox->addWidget(mgc);
    currentGroupBox->addStretch(100);
    parameterList.append(mcAble);
    mgcs.append(mgc);
    mgc->nameLabel.setText(parameterName);

    currentGroupBox->parentWidget()->show();

    return 0;
}

int GuiWidget::setPresetCount(int count) {

    QString qs;

    presetCount = count;
    qs = tr("Presets for this configuration: %1").arg(presetCount);
    presetCountLabel->setText(qs);
    return 0;
}

int GuiWidget::setCurrentPreset(int presetNum, bool rt)
{
    int index, value;

    if (presetCount == 0)
        currentPreset = 0;

    if (presetNum < 0 || presetNum >= presetCount)
        return -1;

    currentPreset = presetNum;

    for (index = 0; index < presetList[currentPreset].count(); index++) {
        value = presetList[currentPreset][index];
        //!!    parameterList.at(index)->invalidateController();
        MidiControllableBase* mcb = getMidiControllableParameter(index);
        if (mcb != NULL) {
            if (rt) {
                mcb->setValRT(value);
                synthdata->mcSet.put(mcb);
            } else {
                mcb->setVal(value, NULL);
            }
        }
    }

    if (!rt)
        setCurrentPresetText();

    return 0;
}

void GuiWidget::setCurrentPresetText()
{
    QString qs;
    QTextStream ts(&qs);

    ts << "Preset " << currentPreset << ":";
    presetLabel->setText(qs);

    for (QStringList::Iterator it = presetNameList.begin();
            it != presetNameList.end(); it++) {
        qs = (*it).mid(0, 3);
        if (qs.toInt() == currentPreset) {
            qs = (*it).mid(3);
            presetName->setText(qs);
        }
    }
}

void GuiWidget::presetDec()
{
    if (currentPreset > 0)
        setCurrentPreset(currentPreset - 1);
}

void GuiWidget::presetInc()
{
    if (currentPreset < presetCount - 1)
        setCurrentPreset(currentPreset + 1);
}

void GuiWidget::addPreset()
{
    QString qs;

    if (presetCount) {
        setPresetCount(presetCount + 1);
        setCurrentPreset(presetCount - 1);
    }
    qs.sprintf("%3d", currentPreset);
    presetNameList.append(qs + presetName->text());
    overwritePreset();
}

void GuiWidget::overwritePreset()
{
    int l1;
    QString qs;

    if (!presetCount)
        setPresetCount(presetCount + 1);

    presetList[currentPreset].clear();
    for (l1 = 0; l1 < parameterList.count(); l1++) {
        MidiControllableBase* mcb = getMidiControllableParameter(l1);
        if (mcb != NULL)
            presetList[currentPreset].append(mcb->sliderVal());
    }
    for (QStringList::Iterator it = presetNameList.begin();
            it != presetNameList.end(); it++) {
        qs = (*it).mid(0, 3);
        if (qs.toInt() == currentPreset) {
            qs.sprintf("%3d", currentPreset);
            *it = qs + presetName->text();
        }
    }
}

void GuiWidget::clearPresets() {

    int l1;

    for (l1 = 0; l1 < MAX_PRESETS; l1++) {
        presetList[l1].clear();
    }
    presetNameList.clear();
    presetName->setText(" ");
    setPresetCount(0);
}

void GuiWidget::clearGui() {

    delete tabWidget;
    frameBoxList.clear();
    tabList.clear();
    parameterList.clear();
    presetNameList.clear();
    presetName->setText(" ");
    frameNameList.clear();
    tabNameList.clear();
    tabWidget = new QTabWidget();
    vLayout.addWidget(tabWidget);
    tabWidget->show();
    setPresetCount(0);
    setCurrentPreset(0);
}

void GuiWidget::refreshGui() {

    tabWidget->hide();
    tabWidget->show();
}

void GuiWidget::remove(MidiControllableBase *mcAble)
{
    int index = parameterList.indexOf(mcAble);
    if (index < 0)
        return;

    MidiGUIcomponent *mgc = mgcs.takeAt(index);
    delete mgc;

    for (int ps = 0; ps < presetCount; ps++)
        presetList[ps].removeAt(index);

    parameterList.removeAt(index);
}

MidiControllableBase* GuiWidget::getMidiControllableParameter(int idx)
{
    MidiControllableBase* mcb = NULL;

    if ((idx + 1) > parameterList.count())
        qWarning("Midi controllable parameter index out of "
                "range (value = %d)", idx);
    else
        mcb = parameterList.at(idx);
    return mcb;
}

GuiWidget::GuiFrame* GuiWidget::getGuiFrame(int idx)
{
    GuiFrame* gf = NULL;

    if ((idx + 1) > frameBoxList.count())
        qWarning("Gui frame index out of range (value = %d)", idx);
    else
        gf = frameBoxList.at(idx);
    return gf;
}

// save MIDI controller window configuration
void GuiWidget::save(QTextStream& ts)
{
    QStringList::iterator presetit;

    // save MIDI controller window configuration
    for (int i = 0; i < tabList.count(); ++i)
        ts << "Tab \"" << tabNameList.at(i) << "\"" << endl;

    for (int i = 0; i < frameBoxList.count(); ++i) {
        ts << "Frame \""
            << frameBoxList.at(i)->frameBox->parentWidget()
            ->objectName() << "\" "
            << frameBoxList.at(i)->tabIndex << endl;

        for (int j = 0; j < parameterList.count(); ++j)
            if (mgcs.at(j)->parent() ==
                    frameBoxList.at(i)->frameBox->parentWidget()) {
                ts << "Parameter \""
                    << mgcs.at(j)->nameLabel.text() << "\" "
                    << parameterList.at(j)->module.moduleID << ' '
                    << parameterList.at(j)->midiControllableListIndex;

                MidiControllableFloat *mcAbleF = dynamic_cast<MidiControllableFloat *>(parameterList.at(j));
                if (mcAbleF)
                    ts << ' ' << mcAbleF->sliderMin() << ' '
                        << mcAbleF->sliderMax() << ' '
                        << mcAbleF->getLog() << endl;
                else
                    ts << endl;
            }
    }

    for (int i = 0; i < presetCount; ++i)
        for (int p = 0; p < presetList[i].count(); p++)
            ts << "Program " << i << ' ' << presetList[i][p] << endl;

    for (presetit = presetNameList.begin();
            presetit != presetNameList.end(); ++presetit) {
        ts << "PresetName \"" << (*presetit).mid(3) << "\"" << endl;
    }
}
