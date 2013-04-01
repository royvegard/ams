#ifndef MIDIWIDGET_H
#define MIDIWIDGET_H

#include <QAbstractListModel>
#include <QCheckBox>
#include <QComboBox>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QString>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

#include "midicontroller.h"
#include "midiguicomponent.h"


/*forward declarations*/
class Module;


class MidiControllerModel : public QAbstractItemModel
{
    Q_OBJECT

    friend class MidiWidget;

    QList<MidiController> &rMidiControllers;

  public:
    MidiControllerModel(QList<MidiController> & , QObject *parent = 0);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
			int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
		      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    void insert(int , MidiController &mc);
};


class ModuleModel : public QAbstractItemModel
{
    Q_OBJECT

    friend class MidiWidget;

    QList<Module*> list;

  public:
    ModuleModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
			int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
		      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
};


class MidiWidget : public QWidget
{
    Q_OBJECT

    MidiGUIcomponent *mgc;
    QVBoxLayout vbox;
    MidiControllerModel midiControllerModel;
    ModuleModel moduleModel;
    MidiControllerKey selectedController;
    QPersistentModelIndex selectedMcAbleIndex;
    QTreeView* midiControllerView;
    QTreeView* moduleListView;
    QVBoxLayout *currentGUIcontrol;
    QCheckBox *logCheck;
    QCheckBox* follwoMidiCb;
    QCheckBox* enableNoteEventsCb;
    QCheckBox* configCheck;
    QComboBox *midiChannelCb;
    QPushButton *newMinButton;
    QPushButton *newMaxButton;
    QPushButton *resetMinMaxButton;
    QPushButton *addGuiButton;
    QPushButton *bindButton;
    QPushButton *clearButton;
    QPushButton *clearAllButton;
    QPushButton *midiSignButton;
    QString currentFrameName, currentTabName;

    MidiControllableBase *midiControllable;
    QList<MidiController> midiControllers;

    void selectMcAble(MidiControllableBase &mcAble);
    void showFloatHelpers(bool show);

  public:

    MidiWidget(QWidget* parent, const char *name=0);
    ~MidiWidget();
    void addMidiController(MidiControllerKey midiController);
    const MidiController *midiController(MidiControllerKey midiController);
    void setActiveMidiControllers();
    void addMidiControllable(MidiControllerKey mck,
			     MidiControllableBase *midiGuiComponent);
    void removeMidiControllable(MidiControllerKey midiController,
				MidiControllableBase *midiGuiComponent,
				bool *updateActiveMidiControllers = NULL);
    const MidiControllerKey getSelectedController();
    void addModule(Module *m);
    void removeModule(Module *m);
    void guiComponentTouched(MidiControllableBase &mcAble);
    void midiTouched(MidiControllableBase &mcAble);
    void loadPreference(QString&);
    void savePreferences(QTextStream&);
    bool noteControllerEnabled();
    bool followConfig();
    void setFollowConfig(bool follow);

  public slots:
    void clearAllClicked();
    void clearClicked();
    void bindClicked();
    void addToParameterViewClicked();
    void toggleMidiSign();
    void guiControlChanged(const QItemSelection &selected,
			   const QItemSelection &deselected);
    void midiControlChanged(const QItemSelection &selected,
			    const QItemSelection &deselected);
    void setLogMode(bool on);
    void setNewMin();
    void setNewMax();
    void setInitialMinMax();
    void updateMidiChannel(int index);
};

#endif
