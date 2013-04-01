#ifndef GUIWIDGET_H
#define GUIWIDGET_H

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <alsa/asoundlib.h>

#include "synthdata.h"
#include "midiguicomponent.h"

#define GUI_DEFAULT_WIDTH  300
#define GUI_DEFAULT_HEIGHT 200
#define MAX_PRESETS        128

class GuiWidget : public QDialog
{
  Q_OBJECT


  QVBoxLayout vLayout;

  QVBoxLayout *currentGroupBox;
  QTabWidget *tabWidget;
  QHBoxLayout *currentTab;
  int currentPreset, currentTabIndex;
  QLabel *presetLabel, *presetCountLabel;
  QLineEdit *presetName;
   
public: 
  struct GuiFrame {
    int tabIndex;
    QVBoxLayout *frameBox;
  };
  
  int presetCount;
  QStringList frameNameList;
  QList<GuiFrame*> frameBoxList;
  QStringList tabNameList;
  QStringList presetNameList;
  QList<QHBoxLayout*> tabList;
  QList<MidiControllableBase*> parameterList;
  QList<MidiGUIcomponent*> mgcs;
  QList<int> presetList[MAX_PRESETS];
    
public:
  GuiWidget(QWidget* parent, const char *name=0);

  int addFrame(const QString &frameName);
  int setFrame(int index);
  int addTab(const QString &tabName);
  int setTab(int index);
  int setPresetCount(int count);
  int setCurrentPreset(int presetNum, bool rt = false);
  void setCurrentPresetText();
  int addParameter(MidiControllableBase *, const QString &parameterName);
  void remove(MidiControllableBase *);
  MidiControllableBase* getMidiControllableParameter(int);
  GuiWidget::GuiFrame* getGuiFrame(int);
  void save(QTextStream&);

public slots:
  void presetDec();
  void presetInc();  
  void addPreset();
  void overwritePreset();
  void clearPresets();
  void clearGui();
  void refreshGui();
};
  
#endif
