#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <signal.h>
#include <QApplication>
#include <QCloseEvent>
#include <QMainWindow>
#include <QMenu>
#include <QSocketNotifier>
#include <QStringList>

#include "config.h"
#include "modularsynth.h"
#include "msoptions.h"
#include "prefwidget.h"

#ifdef NSM_SUPPORT
#include "nsm.h"
#endif

class MainWindow: public QMainWindow {
    Q_OBJECT

    static int pipeFd[2];
    static void sighandler(int);
#ifdef NSM_SUPPORT
    static nsm_client_t *nsm;
    QString configFile;
    QAction *fileNewAction, *fileOpenAction, *fileSaveAction,
            *fileSaveAsAction, *fileOpenDemoAction, *fileOpenDemoInstrumentAction;
#endif
    bool restoregeometry;
    bool hiderecentfiles;
    int rcFd;
    QString fileName;
    QStringList recentFiles;
    ModularSynth *modularSynth;
    QMenu* fileRecentlyOpenedFiles;
    PrefWidget* prefDialog;
    const ModularSynthOptions* synthoptions;

    bool saveFile();
    void newFile();
#ifndef NSM_SUPPORT
    void openFile(const QString&);
#endif
    void chooseFile();
    void chooseDemoFile();
    void chooseDemoInstrumentFile();
    bool isSave();
    int querySaveChanges();
    bool isModified();
    void addRecentlyOpenedFile(const QString&, QStringList&);
    void appendRecentlyOpenedFile(const QString&, QStringList&);

private slots:
    void unixSignal(int fd);
    void fileNew();
    void fileOpen();
    void fileOpenDemo();
    void fileOpenDemoInstrument();
    void fileSave();
    void fileSaveAs();
    void updateWindowTitle();
    void helpAboutAms();
    void helpAboutQt();
    void recentFileActivated(QAction*);
    void setupRecentFilesMenu();
    void applyPreferences();
    void displayPreferences();

#ifdef JACK_SESSION
public slots:
    void handleJackSessionEvent(int);
#endif

public:
  MainWindow(const ModularSynthOptions&);
  virtual ~MainWindow();

protected:
  void closeEvent(QCloseEvent *e);
  void readConfig();
  void writeConfig();

#ifdef NSM_SUPPORT
private:
  static int cb_nsm_open(const char *name, const char *display_name, const char *client_id, char **out_msg, void *userdata);
  static int cb_nsm_save(char **out_msg, void *userdata);

  int nsm_open(const char *name, const char *display_name, const char *client_id, char **out_msg);
  int nsm_save(char **out_msg);

signals:
  void nsmOpenFile(const QString & name);

public slots:
  void openFile(const QString&);

#endif
};

#endif
