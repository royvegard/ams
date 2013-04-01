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


class MainWindow: public QMainWindow {
    Q_OBJECT

    static int pipeFd[2];
    static void sighandler(int);
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
    void openFile(const QString&);
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

};

#endif
