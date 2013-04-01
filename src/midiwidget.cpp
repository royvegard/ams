#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QSplitter>
#include <QStringList>

#include "guiwidget.h"
#include "intmidislider.h"
#include "midicheckbox.h"
#include "midicombobox.h"
#include "midicontroller.h"
#include "midiguicomponent.h"
#include "midipushbutton.h"
#include "midislider.h"
#include "midiwidget.h"
#include "module.h"
#include "synthdata.h"


static const char CF_MIDIENABLENOTEEVENTS[] = "MidiEnableNoteEvents";
static const char CF_MIDIFOLLOWCONFIGDIALOG[] = "MidiFollowConfigDialog";
static const char CF_MIDIFOLLOWMIDI[] = "MidiFollowMidi";
static const char CF_MIDICHANNEL[] = "MidiChannel";
static const char CF_MIDIWIDGETGEOMETRY[] = "MidiWidgetGeometry";


MidiControllerModel::MidiControllerModel(QList<MidiController> &rMidiControllers,
         QObject *parent)
    : QAbstractItemModel(parent)
    , rMidiControllers(rMidiControllers)
{
}


int MidiControllerModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return rMidiControllers.count();

    const MidiController *c = (const MidiController *)parent.internalPointer();
    if (c == NULL)
	return rMidiControllers.at(parent.row()).context->mcAbles.count();

    return 0;
}


QVariant MidiControllerModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && role == Qt::DisplayRole) {
        const MidiController *c = (const MidiController *)index.internalPointer();
        MidiControllableBase *mcAble = NULL;
        if ((c != NULL) && (c->context != NULL)) {
            //FIXME: crash if context not valid
            if (c->context->mcAbles.count() > 0) {
                if (index.row() < c->context->mcAbles.count()) {
                    mcAble = c->context->mcAbles.at(index.row());
                }
            }
        }

        if (mcAble != NULL) {
            if (index.column() > 0)
                return mcAble->module.configDialog->windowTitle();
            else
                return mcAble->getCleanName();
        } else
            if ((c == NULL) && (index.column() == 0)) {
                QString qs;
                c = &rMidiControllers.at(index.row());
                return qs = tr("Type: %1 Channel: %2 Param: %3")
                    .arg(c->type()).arg(c->ch() + 1).arg(c->param());
            }
    }
    return QVariant();
}


QVariant MidiControllerModel::headerData(int section,
        Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return (section == 1)
            ? tr("Module")
            : tr("MIDI Controller / Parameter");

    return QVariant();
}


QModelIndex MidiControllerModel::index(int row, int column,
        const QModelIndex &parent) const
{
    if (parent.isValid())
	return createIndex(row, column,
			   (void *)&rMidiControllers.at(parent.row()));
    else
        return createIndex(row, column);
}


QModelIndex MidiControllerModel::parent(const QModelIndex &child) const
{
    if (child.isValid() && child.internalPointer()) {
        const MidiController *mc =
            (const MidiController *)child.internalPointer();

	typeof(rMidiControllers.constEnd()) c
		= qBinaryFind(rMidiControllers.constBegin(),
			      rMidiControllers.constEnd(), *mc);
        int row = c - rMidiControllers.begin();
        return index(row, 0);
    }
    return QModelIndex();
}


int MidiControllerModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 2;
}


void MidiControllerModel::insert(int row, MidiController &mc)
{
    beginInsertRows(QModelIndex(), row, row);
    rMidiControllers.insert(row, mc);
    rMidiControllers[row].context = new MidiControllerContext();
    endInsertRows();
}


/*class ModuleModel*/
ModuleModel::ModuleModel(QObject* parent): QAbstractItemModel(parent)
{
}


int ModuleModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return list.count();

    if (!parent.internalPointer())
        return list.at(parent.row())->midiControllables.count();

    return 0;
}


QVariant ModuleModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && role == Qt::DisplayRole) {
        Module *m = (Module *)index.internalPointer();
        if (m == NULL) {
            if (index.column() == 0 && index.row() < list.count())
                return list.at(index.row())->configDialog->windowTitle();
        }
        else {
            if (index.row() < m->midiControllables.count()) {
                MidiControllableBase *mcAble = m->midiControllables.at(
                        index.row());
                if (mcAble != NULL) {
                    if (index.column())
                        return mcAble->midiSign ? "1" : "-1";
                    else
                        return mcAble->getCleanName();
                }
            }
        }
    }
    return QVariant();
}


QVariant ModuleModel::headerData(int section, Qt::Orientation orientation,
        int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return section ? tr("MIDI Sign") : tr("Module / Parameter");

    return QVariant();
}


QModelIndex ModuleModel::index(int row, int column,
        const QModelIndex &parent) const
{
    if (parent.isValid())
        return createIndex(row, column, list.at(parent.row()));
    else
        return createIndex(row, column, (void *)NULL);
}


QModelIndex ModuleModel::parent(const QModelIndex &child) const
{
    if (child.isValid() && child.internalPointer())
        return index(list.indexOf((Module *)child.internalPointer()), 0);

    return QModelIndex();
}


int ModuleModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 2;
}



/* MidiWidget class*/

MidiWidget::MidiWidget(QWidget* parent, const char *name)
    : QWidget(parent)
    , mgc(NULL)
    , vbox(this)
    , midiControllerModel(midiControllers)
    , selectedMcAbleIndex(QModelIndex())
    , midiControllable(NULL)
{
    setObjectName(name);
    int l1;
    QString qs;

    vbox.setMargin(10);
    vbox.setSpacing(5);

    QSplitter *listViewBox = new QSplitter();
    vbox.addWidget(listViewBox, 2);

    midiControllerView = new QTreeView(listViewBox);
    midiControllerView->setModel(&midiControllerModel);
    midiControllerView->setAllColumnsShowFocus(true);

    moduleListView = new QTreeView(listViewBox);
    moduleListView->setModel(&moduleModel);
    moduleListView->setAllColumnsShowFocus(true);

    QObject::connect(midiControllerView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &,
                    const QItemSelection &)),
            this,
            SLOT(midiControlChanged(const QItemSelection &,
                    const QItemSelection &)));

    QObject::connect(moduleListView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &,
                    const QItemSelection &)),
            this,
            SLOT(guiControlChanged(const QItemSelection &,
                    const QItemSelection &)));

    QVBoxLayout *controlFrame = new QVBoxLayout();
    controlFrame->setSpacing(5);
    vbox.addLayout(controlFrame);
    QFrame* guiControlParent = new QFrame(); // QVBoxLayout
    controlFrame->addWidget(guiControlParent);
    guiControlParent->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    currentGUIcontrol = new QVBoxLayout(guiControlParent);
    currentGUIcontrol->setMargin(5);
    QHBoxLayout* floatHelperLayout = new QHBoxLayout();
    currentGUIcontrol->addLayout(floatHelperLayout);

    logCheck = new QCheckBox(tr("&Log"));
    floatHelperLayout->addWidget(logCheck);
    QObject::connect(logCheck, SIGNAL(toggled(bool)),
            this, SLOT(setLogMode(bool)));
    floatHelperLayout->addStretch();

    newMinButton = new QPushButton(tr("Set Mi&n"));
    floatHelperLayout->addWidget(newMinButton);
    QObject::connect(newMinButton, SIGNAL(clicked()), this, SLOT(setNewMin()));
    floatHelperLayout->addStretch();

    newMaxButton = new QPushButton(tr("Set Ma&x"));
    floatHelperLayout->addWidget(newMaxButton);
    QObject::connect(newMaxButton, SIGNAL(clicked()), this, SLOT(setNewMax()));
    floatHelperLayout->addStretch();

    resetMinMaxButton = new QPushButton(tr("&Reset Min/Max"));
    floatHelperLayout->addWidget(resetMinMaxButton);
    QObject::connect(resetMinMaxButton, SIGNAL(clicked()),
            this, SLOT(setInitialMinMax()));
    showFloatHelpers(false);

    //  currentGUIcontrol = NULL;
    QHBoxLayout *checkbuttonBox = new QHBoxLayout();
    controlFrame->addLayout(checkbuttonBox);
    checkbuttonBox->setSpacing(10);
    checkbuttonBox->setMargin(5);

    QStringList channelNames;
    channelNames << tr("Omni");
    //TODO: remove obsolete empty spaces
    for (l1 = 1; l1 < 17; l1++) {
        qs.sprintf("%4d", l1);
        channelNames << qs;
    }

    QHBoxLayout *midiChannelBox = new QHBoxLayout();
    controlFrame->addLayout(midiChannelBox);
    QLabel *channelText = new QLabel(tr("MIDI C&hannel:"));
    midiChannelBox->addWidget(channelText);
    midiChannelCb = new QComboBox();
    channelText->setBuddy(midiChannelCb);
    midiChannelBox->addWidget(midiChannelCb);
    midiChannelBox->addStretch();
    midiChannelBox->addStretch();
    midiChannelBox->addStretch();
    midiChannelCb->addItems(channelNames);
    midiChannelCb->setFixedSize(midiChannelCb->sizeHint());
    QObject::connect(midiChannelCb, SIGNAL(highlighted(int)),
            this, SLOT(updateMidiChannel(int)));
    midiChannelBox->addStretch();

    addGuiButton = new QPushButton(tr("Add to &Parameter View"));
    addGuiButton->setEnabled(false);
    midiChannelBox->addWidget(addGuiButton);
    QObject::connect(addGuiButton, SIGNAL(clicked()),
            this, SLOT(addToParameterViewClicked()));
    midiChannelBox->addStretch();

    QHBoxLayout *buttonBox = new QHBoxLayout();
    controlFrame->addLayout(buttonBox);
    buttonBox->setSpacing(5);
    buttonBox->setMargin(5);
    buttonBox->addStretch();

    enableNoteEventsCb = new QCheckBox(tr("&Enable note events"));
    checkbuttonBox->addWidget(enableNoteEventsCb);

    configCheck = new QCheckBox(tr("&Follow Configuration Dialog"));
    checkbuttonBox->addWidget(configCheck);

    follwoMidiCb = new QCheckBox(tr("Follow &MIDI"));
    checkbuttonBox->addWidget(follwoMidiCb);
    buttonBox->addStretch();

    bindButton = new QPushButton(tr("&Bind"));
    bindButton->setEnabled(false);
    buttonBox->addWidget(bindButton);
    buttonBox->addStretch();

    clearButton = new QPushButton(tr("&Clear Binding"));
    buttonBox->addWidget(clearButton);
    clearButton->setEnabled(false);
    buttonBox->addStretch();

    clearAllButton = new QPushButton(tr("Clear &All"));
    buttonBox->addWidget(clearAllButton);
    buttonBox->addStretch();

    midiSignButton = new QPushButton(tr("&Toggle MIDI Sign"));
    buttonBox->addWidget(midiSignButton);
    midiSignButton->setEnabled(false);
    buttonBox->addStretch();

    QObject::connect(bindButton, SIGNAL(clicked()),
            this, SLOT(bindClicked()));
    QObject::connect(clearButton, SIGNAL(clicked()),
            this, SLOT(clearClicked()));
    QObject::connect(clearAllButton, SIGNAL(clicked()),
            this, SLOT(clearAllClicked()));
    QObject::connect(midiSignButton, SIGNAL(clicked()),
            this, SLOT(toggleMidiSign()));
}


MidiWidget::~MidiWidget()
{
}


void MidiWidget::clearAllClicked()
{
    for (int l1 = 0; l1 < midiControllers.count(); l1++) {
        const MidiController &c = midiControllers.at(l1);
        while (c.context->mcAbles.count()) {
            MidiControllableBase *mcAble = c.context->mcAbles.at(0);
            mcAble->disconnectController(c);
        }
    }
    midiControllerModel.beginRemoveRows(QModelIndex(), 0,
					midiControllers.count() - 1);
    midiControllers.clear();
    midiControllerModel.endRemoveRows();
}


/* MIDI event arrived from modularsynth.cpp
 * this adds a controller to the controller list view or
 * update the selection, if it is already there */
void MidiWidget::addMidiController(MidiControllerKey mck)
{
    MidiController mc(mck.getKey());
    typeof(midiControllers.end()) c(midiControllers.end());

    if (!midiControllers.empty()) {
        c = qLowerBound(midiControllers.begin(),
			midiControllers.end(), mck);
        if (c != midiControllers.end()) {
            if (*c == mc) {
                /* controller is found, now update selection */
                if (follwoMidiCb->isChecked()) {
                    int row = midiControllers.indexOf(mc);
                    QModelIndex index = midiControllerModel.index(row, 0);
                    midiControllerView->scrollTo(index);

                    /* only update selection if not already selected*/
                    if (mck == selectedController)
                        return;
                    midiControllerView->selectionModel()->
                        select(index, QItemSelectionModel::ClearAndSelect);
                }
		return;
            }
        }
    }
    midiControllerModel.insert(c - midiControllers.begin(), mc);
    midiControllerView->resizeColumnToContents(0);
}


void MidiWidget::addMidiControllable(MidiControllerKey mck,
        MidiControllableBase *mcAble)
{
    typeof(midiControllers.constEnd()) c
        = qBinaryFind(midiControllers.constBegin(),
                midiControllers.constEnd(), mck);

    if (c == midiControllers.end()) {
        StdErr  << __PRETTY_FUNCTION__ << ":" << __LINE__ << endl;
        return;
    }

    int row = c - midiControllers.begin();
    int childRow = c->context->mcAbles.count();

    midiControllerModel.beginInsertRows(midiControllerModel.index(row, 0),
            childRow, childRow);
    c->context->mcAbles.append(mcAble);
    midiControllerModel.endInsertRows();
    moduleListView->resizeColumnToContents(0);
}


void MidiWidget::removeMidiControllable(MidiControllerKey mck,
					MidiControllableBase *mcAble,
					bool *updateActiveMidiControllers)
{
    typeof(midiControllers.constEnd()) c
        = qBinaryFind(midiControllers.constBegin(),
                midiControllers.constEnd(), mck);
    if (c == midiControllers.end()) {
        StdErr  << __PRETTY_FUNCTION__ << ":" << __LINE__ << endl;
        return;
    }

    int row = c - midiControllers.begin();
    int childRow = c->context->mcAbles.indexOf(mcAble);
    if (childRow != -1) {
        midiControllerModel.beginRemoveRows(midiControllerModel.index(row, 0),
                childRow, childRow);
        c->context->mcAbles.removeAll(mcAble);
        midiControllerModel.endRemoveRows();
    }
    if (!updateActiveMidiControllers || *updateActiveMidiControllers)
	setActiveMidiControllers();
    if (updateActiveMidiControllers)
	*updateActiveMidiControllers = false;
}


void MidiWidget::clearClicked()
{
    if (selectedController.isValid() && selectedMcAbleIndex.isValid()) {
        typeof(midiControllers.constEnd()) c
            = qBinaryFind(midiControllers.constBegin(),
                    midiControllers.constEnd(), selectedController);
        if (c == midiControllers.end()) {
            StdErr  << __PRETTY_FUNCTION__ << ":" << __LINE__ << endl;
            return;
        }

        MidiControllableBase *mcAble =
            c->context->mcAbles.at(selectedMcAbleIndex.row());
        if (mcAble != NULL)
            mcAble->disconnectController(selectedController);
    }
}


void MidiWidget::addToParameterViewClicked()
{
    QString qs, qs2, qs3;
    bool ok, foundFrameName;
    int l1, frameIndex, tabIndex;

    if (midiControllable == NULL)
        return;

    if (synthdata->guiWidget->presetCount > 0) {
        QMessageBox questionContinue("AlsaModularSynth",
                tr("This will erase all presets for this configuration. "
                    "Continue?"),
                QMessageBox::NoIcon,
                QMessageBox::Yes | QMessageBox::Default,
                QMessageBox::No  | QMessageBox::Escape,
                QMessageBox::NoButton);
        if (questionContinue.exec() == QMessageBox::No)
            return;
    }

    qs = QInputDialog::getText(this, "AlsaModularSynth",
            tr("Add this parameter to frame:"),
            QLineEdit::Normal, currentFrameName, &ok);

    currentFrameName = qs;
    if (qs.isEmpty())
        return;

    foundFrameName = false;
    frameIndex = 0;
    if ((l1 =synthdata->guiWidget->frameNameList.indexOf(qs.trimmed())) >= 0) {
        foundFrameName = true;
        frameIndex = l1;
    }

    if (!foundFrameName) {
        qs2 = tr("Frame '%1' does not exist. Create?").arg(qs);
        QMessageBox question("AlsaModularSynth", qs2, QMessageBox::NoIcon,
                QMessageBox::Yes | QMessageBox::Default,
                QMessageBox::No  | QMessageBox::Escape, QMessageBox::NoButton);

        if (question.exec() == QMessageBox::Yes) {
            qs3 = QInputDialog::getText(this, "AlsaModularSynth",
                    tr("Add frame to tab:"), QLineEdit::Normal,
                    currentTabName, &ok);
            currentTabName = qs3;
            tabIndex = 0;

            if ((l1 =synthdata->guiWidget->tabNameList.indexOf(
                            qs3.trimmed())) >= 0) {
                tabIndex = l1;
                synthdata->guiWidget->setTab(tabIndex);
            }

            else {
                qs2 = tr("Tab '%1' does not exist. Create?").arg(qs3);
                QMessageBox question("AlsaModularSynth", qs2,
                        QMessageBox::NoIcon,
                        QMessageBox::Yes | QMessageBox::Default,
                        QMessageBox::No  | QMessageBox::Escape,
                        QMessageBox::NoButton);

                if (question.exec() == QMessageBox::Yes)
                    synthdata->guiWidget->addTab(qs3.trimmed());
                else
                    return;
            }

            synthdata->guiWidget->addFrame(qs.trimmed());
        }
        else
            return;

    }
    else
        synthdata->guiWidget->setFrame(frameIndex);

    qs2 = midiControllable->getCleanName() + " ID " +
        QString().setNum(midiControllable->module.moduleID);

    qs = QInputDialog::getText(this, "AlsaModularSynth",
            tr("Parameter name:"), QLineEdit::Normal, qs2, &ok);

    synthdata->guiWidget->addParameter(midiControllable, qs);
}


void MidiWidget::bindClicked()
{
    if (midiControllable &&
	selectedController.isValid() &&	!selectedMcAbleIndex.isValid()) {
        int row = midiControllers.indexOf(selectedController.getKey());
        midiControllerView->setExpanded(
                midiControllerModel.index(row, 0), true);
        midiControllable->connectToController(selectedController);
        setActiveMidiControllers();
    }
}


void MidiWidget::addModule(Module *m)
{
    if (!m->midiControllables.count())
        return;

    int row = moduleModel.list.count();

    moduleModel.beginInsertRows(QModelIndex(), row, row);
    moduleModel.list.append(m);
    moduleModel.endInsertRows();
}


void MidiWidget::removeModule(Module *m)
{
    synthdata->moduleList.removeAll(m);
    synthdata->decModuleCount();

    int row = moduleModel.list.indexOf(m);
    if (row == -1)
        return;

    if (m->midiControllables.contains(midiControllable)) {
        delete mgc;
        mgc = NULL;
        midiControllable = NULL;
    }
    moduleModel.beginRemoveRows(QModelIndex(), row, row);
    moduleModel.list.removeAll(m);
    moduleModel.endRemoveRows();
}


void MidiWidget::toggleMidiSign()
{
    if (midiControllable == NULL)
        return;

    midiControllable->midiSign = !midiControllable->midiSign;
    Module *m = &midiControllable->module;
    QModelIndex mMi = moduleModel.index(moduleModel.list.indexOf(m), 0);
    QModelIndex mgcMi = moduleModel.index(
            m->midiControllables.indexOf(midiControllable), 1, mMi);
    emit moduleModel.dataChanged(mgcMi, mgcMi);
}

/* Module parameter selection changed, right list */
void MidiWidget::guiControlChanged(const QItemSelection &selected,
				   const QItemSelection &/*deselected*/)
{
    bool success = false;

    if (midiControllable) {
        delete mgc;
        mgc = NULL;
        midiControllable = NULL;
    }

    if (selected.indexes().count() > 0) {
        const QModelIndex mi = selected.indexes().at(0);
        Module *module = (Module *)mi.internalPointer();

        if (module && mi.row() < module->midiControllables.count()) {
            midiControllable = module->midiControllables.at(mi.row());
            success = true;
        }
    }
    midiSignButton->setEnabled(success);
    addGuiButton->setEnabled(success);
    if (!success)
        return;

    mgc = dynamic_cast<MidiGUIcomponent *>(midiControllable->mcws.at(0))->createTwin();
    currentGUIcontrol->insertWidget(0, mgc);//, 10, Qt::AlignHCenter);
    showFloatHelpers(dynamic_cast<MidiControllableFloat *>(midiControllable));
}

/* MIDI controller selection changed , left list */
void MidiWidget::midiControlChanged(const QItemSelection &selected,
				    const QItemSelection &/*deselected*/)
{
    selectedController = MidiControllerKey();
    bool bindEnable = false;
    bool clearEnable = false;
    selectedMcAbleIndex = QModelIndex();
    if (selected.indexes().count() > 0) {
        const QModelIndex mi = selected.indexes().at(0);
        const MidiController *mc = (const MidiController *)mi.internalPointer();

        if (mc) {
            selectedController = mc->getKey();
            selectedMcAbleIndex = mi;
            clearEnable = true;
        } else {
            selectedController = midiControllers.at(mi.row()).getKey();
            bindEnable = true;
        }
    }
    bindButton->setEnabled(bindEnable);
    clearButton->setEnabled(clearEnable);
}


void MidiWidget::setLogMode(bool on)
{
    if (midiControllable != NULL)
        dynamic_cast<MidiControllableFloat *>(midiControllable)->setLog(on);
}

void MidiWidget::setNewMin()
{
    if (midiControllable != NULL)
        dynamic_cast<MidiControllableFloat *>(midiControllable)->setNewMin();
}

void MidiWidget::setNewMax()
{
    if (midiControllable != NULL)
        dynamic_cast<MidiControllableFloat *>(midiControllable)->setNewMax();
}

void MidiWidget::setInitialMinMax()
{
    if (midiControllable != NULL)
        dynamic_cast<MidiControllableFloat *>(midiControllable)->resetMinMax();
}

void MidiWidget::selectMcAble(MidiControllableBase &mcAble)
{
    int row = moduleModel.list.indexOf(&mcAble.module);
    QModelIndex index = moduleModel.index(row, 0);
    row = mcAble.module.midiControllables.indexOf(&mcAble);
    index = moduleModel.index(row, 0, index);
    moduleListView->scrollTo(index);
    if (&mcAble == midiControllable)
	return;
    moduleListView->selectionModel()->
	select(index, QItemSelectionModel::ClearAndSelect);
}

void MidiWidget::showFloatHelpers(bool show)
{
    if (show) {
        logCheck->blockSignals(true);
        logCheck->setChecked(static_cast<MidiControllableFloat *>(midiControllable)->getLog());
        logCheck->blockSignals(false);
    }
    logCheck->setVisible(show);
    newMinButton->setVisible(show);
    newMaxButton->setVisible(show);
    resetMinMaxButton->setVisible(show);
}


void MidiWidget::updateMidiChannel(int index)
{
    synthdata->midiChannel = index - 1;
}


void MidiWidget::setActiveMidiControllers()
{
  typeof(synthdata->activeMidiControllers) New =
    new typeof(*synthdata->activeMidiControllers);

  for (typeof(midiControllers.constBegin()) mc =
	 midiControllers.constBegin();
       mc != midiControllers.constEnd(); ++mc) {
    MidiControllerContext *amcc = NULL;

    for (typeof(mc->context->mcAbles.constBegin()) mca =
	   mc->context->mcAbles.constBegin();
	 mca != mc->context->mcAbles.constEnd(); ++mca)
      if ((*mca)->module.isAlive()) {
	if (!amcc) {
	  New->append(mc->getKey());
	  amcc = New->back().context = new MidiControllerContext();
	}
	amcc->mcAbles.append(*mca);
      }
  }

  typeof(synthdata->activeMidiControllers) old =
    synthdata->activeMidiControllers;

  pthread_mutex_lock(&synthdata->rtMutex);
  synthdata->activeMidiControllers = New;
  pthread_mutex_unlock(&synthdata->rtMutex);

  delete old;
}


// const MidiController* MidiWidget::midiController(MidiControllerKey mck)
// {
//     typeof(midiControllers.constEnd()) c =
//         qBinaryFind(midiControllers.constBegin(),
// 		    midiControllers.constEnd(), mck);
//     return c == midiControllers.constEnd() ? NULL : &*c;
// }


const MidiControllerKey MidiWidget::getSelectedController()
{
    return selectedController;
}


void MidiWidget::guiComponentTouched(MidiControllableBase &mcAble)
{
    if (configCheck->isChecked())
        selectMcAble(mcAble);
}

/* MIDI event arrived from modularsynth.cpp;
 * this only updates the module view selection but does not update the
 * controller view selection */
void MidiWidget::midiTouched(MidiControllableBase &mcAble)
{
    if (follwoMidiCb->isChecked())
        selectMcAble(mcAble);
}

void MidiWidget::loadPreference(QString& line)
{
    int value;

    if (line.startsWith(CF_MIDIENABLENOTEEVENTS)) {
	value = line.section(' ', 1).toInt();
        enableNoteEventsCb->setChecked(value != 0);
    }
    else if (line.startsWith(CF_MIDIFOLLOWCONFIGDIALOG)) {
	value = line.section(' ', 1).toInt();
        configCheck->setChecked(value != 0);
    }
    else if (line.startsWith(CF_MIDIFOLLOWMIDI)) {
	value = line.section(' ', 1).toInt();
        follwoMidiCb->setChecked(value != 0);
    }
    else if (line.startsWith(CF_MIDICHANNEL)) {
	value = line.section(' ', 1).toInt();
        midiChannelCb->setCurrentIndex(value);
        updateMidiChannel(value);
    }
    else if (line.startsWith(CF_MIDIWIDGETGEOMETRY)) {
        QStringList tokens = line.split(' ');
        if (tokens.count() < 6) {
            qWarning("Position widget geometry parameterlist too short.");
            return;
        }
        bool isvisible = (tokens[1].toInt() > 0);
        //FIXME: position may be off screen if size of current screen is
        //smaller than screen used for saving
        int x = tokens[2].toInt();
        int y = tokens[3].toInt();
        int width = tokens[4].toInt();
        int height = tokens[5].toInt();
        setGeometry(x, y, width, height);
        if (isvisible) {
            //FIXME: window is not shown in front of main window because
            //it is not a child of main window
            show();
        }
    }
}

void MidiWidget::savePreferences(QTextStream& ts)
{
    ts << CF_MIDIENABLENOTEEVENTS << ' ' << enableNoteEventsCb->isChecked() << endl;
    ts << CF_MIDIFOLLOWCONFIGDIALOG << ' ' << configCheck->isChecked() << endl;
    ts << CF_MIDIFOLLOWMIDI << ' ' << follwoMidiCb->isChecked() << endl;
    ts << CF_MIDICHANNEL << ' ' << midiChannelCb->currentIndex() << endl;
    ts << CF_MIDIWIDGETGEOMETRY << ' '
        << isVisible() << ' '
        << geometry().x() << ' '
        << geometry().y() << ' '
        << geometry().width() << ' '
        << geometry().height() << endl;
}

bool MidiWidget::noteControllerEnabled()
{
    return enableNoteEventsCb->isChecked();

}

bool MidiWidget::followConfig()
{
    return configCheck->isChecked();

}

void MidiWidget::setFollowConfig(bool follow)
{
    configCheck->setChecked(follow);
}

