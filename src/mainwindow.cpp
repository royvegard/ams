#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QScrollArea>

#include "aboutdialog.h"
#include "mainwindow.h"
#include "synthdata.h"

#include "../pixmaps/ams_32.xpm"

/*some constants*/
#define APPNAME  "AlsaModularSynth"
#define PATCHEXT ".ams"

/*string constants for user settings file*/
static const char CF_HIDERECENTFILES[] = "HideRecentFiles";
static const char CF_RECENTFILE[] = "RecentFile";
static const char CF_RESTOREGEOMETRY[] = "RestoreGeometry";
static const char CF_GEOMETRY[] = "Geometry";

/*constants for window geometry*/
enum {
    DEFAULT_WIDTH = 750,
    DEFAULT_HEIGHT = 550
};


class ScrollArea: public QScrollArea {
    void resizeEvent(QResizeEvent *ev)
    {
        QScrollArea::resizeEvent(ev);
        widget()->adjustSize();
    }
};


int MainWindow::pipeFd[2];

MainWindow::MainWindow(const ModularSynthOptions& mso)
{
  /*make sure the window destructor is called on program exit*/
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowIcon(QPixmap(ams_32_xpm));
  setObjectName("MainWindow");
  fileName = "";
  restoregeometry = true;
  hiderecentfiles = false;
  prefDialog = NULL;
  resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
  rcFd = mso.rcFd;
  synthoptions = &mso;

  /*init synthesizer*/
  modularSynth = new ModularSynth(this, mso);

  /*init window*/
  ScrollArea *scrollArea = new ScrollArea();
  scrollArea->setWidget(modularSynth);
  setCentralWidget(scrollArea);

  QMenu *filePopup = menuBar()->addMenu(tr("&File"));
  QMenu *synthesisPopup = menuBar()->addMenu(tr("&Synthesis"));
  QMenu *modulePopup = menuBar()->addMenu(tr("&Module"));
  QMenu *midiMenu = menuBar()->addMenu(tr("&View"));
  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

  QMenu *newModulePopup = modulePopup->addMenu(tr("&New"));
  modularSynth->contextMenu = newModulePopup;

  filePopup->addAction(tr("&New"), this, SLOT(fileNew()),
          Qt::CTRL + Qt::Key_N);
  filePopup->addAction(tr("&Open..."), this, SLOT(fileOpen()),
          Qt::CTRL + Qt::Key_O);
  filePopup->addAction(tr("Open &demo..."), this, SLOT(fileOpenDemo()),
          Qt::CTRL + Qt::Key_D);
  filePopup->addAction(tr("Open demo &instrument..."), this,
          SLOT(fileOpenDemoInstrument()), Qt::CTRL + Qt::Key_I);
  fileRecentlyOpenedFiles = filePopup->addMenu(tr("&Recently opened files"));
  filePopup->addAction(tr("&Save"), this, SLOT(fileSave()),
          Qt::CTRL + Qt::Key_S);
  filePopup->addAction(tr("Save &as..."), this, SLOT(fileSaveAs()));
  filePopup->addSeparator();
  filePopup->addAction(tr("&Load Colors..."), modularSynth,
          SLOT(loadColors()));
  filePopup->addAction(tr("Save &Colors as..."), modularSynth,
          SLOT(saveColors()));
  filePopup->addSeparator();
  filePopup->addAction(tr("&Quit"), qApp, SLOT(closeAllWindows()),
          Qt::CTRL + Qt::Key_Q);
  connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

  synthesisPopup->addAction(tr("&Start"), modularSynth, SLOT(startSynth()),
          Qt::CTRL + Qt::Key_B);
  synthesisPopup->addAction(tr("Sto&p"), modularSynth, SLOT(stopSynth()),
          Qt::CTRL + Qt::Key_H);
  synthesisPopup->addAction(tr("&All Voices Off"), modularSynth,
          SLOT(allVoicesOff()));

  /*In/Out submenu*/
  QMenu *newModuleInOut = newModulePopup->addMenu(tr("&In/Out"));
  newModuleInOut->addAction(tr("PCM &Out"),
          modularSynth, SLOT(newM_pcmout()));
  newModuleInOut->addAction(tr("PCM &In"),
          modularSynth, SLOT(newM_pcmin()));
  newModuleInOut->addSeparator();
  newModuleInOut->addAction(tr("&MCV"),
          modularSynth, SLOT(newM_mcv()));
  newModuleInOut->addAction(tr("&Advanced MCV"),
          modularSynth, SLOT(newM_advmcv()));
  newModuleInOut->addAction(tr("&Scala MCV"),
          modularSynth, SLOT(newM_scmcv()));
  newModuleInOut->addAction(tr("MI&DI Out"),
          modularSynth, SLOT(newM_midiout()));
  newModuleInOut->addAction(tr("&WAV Out"),
          modularSynth, SLOT(newM_wavout()));
  newModuleInOut->addSeparator();
  newModuleInOut->addAction(tr("S&cope View"),
          modularSynth, SLOT(newM_scope()));
  newModuleInOut->addAction(tr("S&pectrum View"),
          modularSynth, SLOT(newM_spectrum()));

  /*Oscillators submenu*/
  QMenu *newModuleOscillators = newModulePopup->addMenu(tr("&Oscillators"));
  newModuleOscillators->addAction(tr("&LFO"),
          modularSynth, SLOT(newM_lfo()));
  newModuleOscillators->addAction(tr("&VCO"),
          modularSynth, SLOT(newM_vco()));
  newModuleOscillators->addAction(tr("V&CO2"),
          modularSynth, SLOT(newM_vco2()));
  newModuleOscillators->addAction(tr("&Multiphase LFO"),
          modularSynth, SLOT(newM_mphlfo()));
  newModuleOscillators->addAction(tr("VC &Organ (4 Oscillators)"),
          modularSynth, SLOT(newM_vcorgan_4()));
  newModuleOscillators->addAction(tr("VC O&rgan (6 Oscillators)"),
          modularSynth, SLOT(newM_vcorgan_6()));
  newModuleOscillators->addAction(tr("VC Or&gan (8 Oscillators)"),
          modularSynth, SLOT(newM_vcorgan_8()));
  newModuleOscillators->addAction(tr("&Dynamic Waves (4 Oscillators)"),
          modularSynth, SLOT(newM_dynamicwaves_4()));
  newModuleOscillators->addAction(tr("D&ynamic Waves (6 Oscillators)"),
          modularSynth, SLOT(newM_dynamicwaves_6()));
  newModuleOscillators->addAction(tr("Dy&namic Waves (8 Oscillators)"),
          modularSynth, SLOT(newM_dynamicwaves_8()));

  /*Spectrum Modifiers submenu*/
  QMenu *newModuleSpectrumModifiers =
      newModulePopup->addMenu(tr("Spectrum &modifiers"));
  newModuleSpectrumModifiers->addAction(tr("&VCF"),
          modularSynth, SLOT(newM_vcf()));
  newModuleSpectrumModifiers->addAction(tr("&Analog Driver (2 Out)"),
          modularSynth, SLOT(newM_ad_2()));
  newModuleSpectrumModifiers->addAction(tr("A&nalog Driver (4 Out)"),
          modularSynth, SLOT(newM_ad_4()));
  newModuleSpectrumModifiers->addAction(tr("Ana&log Driver (6 Out)"),
          modularSynth, SLOT(newM_ad_6()));
  newModuleSpectrumModifiers->addAction(tr("&Bit Grinder"),
          modularSynth, SLOT(newM_bitgrind()));
  newModuleSpectrumModifiers->addAction(tr("&FFT Vocoder"),
          modularSynth, SLOT(newM_vocoder()));

  /*Time Modifiers submenu*/
  QMenu *newModuleTimeModifiers = newModulePopup->addMenu(tr("&Time modifiers"));
  newModuleTimeModifiers->addAction(tr("&Sample && Hold"),
          modularSynth, SLOT(newM_sh()));
  newModuleTimeModifiers->addAction(tr("&Delay"),
          modularSynth, SLOT(newM_delay()));
  newModuleTimeModifiers->addAction(tr("&VC Delay"),
          modularSynth, SLOT(newM_vcdelay()));
  newModuleTimeModifiers->addAction(tr("&Analog Memory"),
          modularSynth, SLOT(newM_analogmemory()));
  newModuleTimeModifiers->addAction(tr("&INV"),
          modularSynth, SLOT(newM_inv()));
  newModuleTimeModifiers->addAction(tr("V&C Double Decay"),
          modularSynth, SLOT(newM_vcdoubledecay()));

  /*Envelopes submenu*/
  QMenu *newModuleEnvelopes = newModulePopup->addMenu(tr("&Envelopes"));
  newModuleEnvelopes->addAction(tr("&ENV"),
          modularSynth, SLOT(newM_env()));
  newModuleEnvelopes->addAction(tr("&Advanced ENV"),
          modularSynth, SLOT(newM_advenv()));
  newModuleEnvelopes->addAction(tr("&VC Envelope"),
          modularSynth, SLOT(newM_vcenv()));
  newModuleEnvelopes->addAction(tr("V&C Envelope II"),
          modularSynth, SLOT(newM_vcenv2()));
  newModuleEnvelopes->addAction(tr("&Function 1 --> 1"),
          modularSynth, SLOT(newM_function_1()));
  newModuleEnvelopes->addAction(tr("F&unction 1 --> 2"),
          modularSynth, SLOT(newM_function_2()));
  newModuleEnvelopes->addAction(tr("Fu&nction 1 --> 4"),
          modularSynth, SLOT(newM_function_4()));

  /*Sequencers submenu*/
  QMenu *newModuleSequencers = newModulePopup->addMenu(tr("&Sequencers"));
  newModuleSequencers->addAction(tr("&SEQ  8"),
          modularSynth, SLOT(newM_seq_8()));
  newModuleSequencers->addAction(tr("S&EQ 12"),
          modularSynth, SLOT(newM_seq_12()));
  newModuleSequencers->addAction(tr("SE&Q 16"),
          modularSynth, SLOT(newM_seq_16()));
  newModuleSequencers->addAction(tr("SEQ &24"),
          modularSynth, SLOT(newM_seq_24()));
  newModuleSequencers->addAction(tr("SEQ &32"),
          modularSynth, SLOT(newM_seq_32()));
  newModuleSequencers->addAction(tr("&V8 Sequencer"),
          modularSynth, SLOT(newM_v8sequencer()));

  /*VCA & Mix submenu*/
  QMenu *newModuleVcaMix = newModulePopup->addMenu(tr("&VCA and Mix"));
  newModuleVcaMix->addAction(tr("&Amplifier"),
          modularSynth, SLOT(newM_amp()));
  newModuleVcaMix->addAction(tr("&VCA lin."),
          modularSynth, SLOT(newM_vca_lin()));
  newModuleVcaMix->addAction(tr("V&CA exp."),
          modularSynth, SLOT(newM_vca_exp()));
  newModuleVcaMix->addAction(tr("&Ring Modulator"),
          modularSynth, SLOT(newM_ringmod()));
  newModuleVcaMix->addAction(tr("&Mixer 2 -> 1"),
          modularSynth, SLOT(newM_mix_2()));
  newModuleVcaMix->addAction(tr("M&ixer 4 -> 1"),
          modularSynth, SLOT(newM_mix_4()));
  newModuleVcaMix->addAction(tr("Mi&xer 8 -> 1"),
          modularSynth, SLOT(newM_mix_8()));
  newModuleVcaMix->addAction(tr("&Stereo Mixer 2"),
          modularSynth, SLOT(newM_stereomix_2()));
  newModuleVcaMix->addAction(tr("S&tereo Mixer 4"),
          modularSynth, SLOT(newM_stereomix_4()));
  newModuleVcaMix->addAction(tr("St&ereo Mixer 8"),
          modularSynth, SLOT(newM_stereomix_8()));
  newModuleVcaMix->addAction(tr("VC &Panning"),
          modularSynth, SLOT(newM_vcpanning()));
  newModuleVcaMix->addAction(tr("VC S&witch"),
          modularSynth, SLOT(newM_vcswitch()));

  /*CV Operations*/
  QMenu *newModuleCvOps = newModulePopup->addMenu(tr("&CV Operations"));
  newModuleCvOps->addAction(tr("&CVS"),
          modularSynth, SLOT(newM_cvs()));
  newModuleCvOps->addAction(tr("&Slew Limiter"),
          modularSynth, SLOT(newM_slew()));
  newModuleCvOps->addAction(tr("&Noise/ Random"),
          modularSynth, SLOT(newM_noise()));
  newModuleCvOps->addAction(tr("N&oise/Random 2"),
          modularSynth, SLOT(newM_noise2()));
  newModuleCvOps->addAction(tr("Sa&mple && Hold"),
          modularSynth, SLOT(newM_sh()));
  newModuleCvOps->addAction(tr("&Quantizer"),
          modularSynth, SLOT(newM_quantizer()));
  newModuleCvOps->addAction(tr("Q&uantizer 2"),
          modularSynth, SLOT(newM_vquant()));
  newModuleCvOps->addAction(tr("Sc&ala Quantizer"),
          modularSynth, SLOT(newM_scquantizer()));
  newModuleCvOps->addAction(tr("C&onverter"),
          modularSynth, SLOT(newM_conv()));
  newModuleCvOps->addAction(tr("&Hysteresis"),
          modularSynth, SLOT(newM_hysteresis()));

  newModulePopup->addAction(tr("Co&mment"), modularSynth, SLOT(new_textEdit()));

  modulePopup->addAction(tr("&Show Ladspa Browser..."), modularSynth,
          SLOT(displayLadspaPlugins()));

  midiMenu->addAction(tr("&Control Center..."), modularSynth,
          SLOT(displayMidiController()));
  midiMenu->addAction(tr("&Parameter View..."), modularSynth,
          SLOT(displayParameterView()));
  midiMenu->addAction(tr("Pre&ferences..."), this,
          SLOT(displayPreferences()));

  helpMenu->addAction(tr("&About AlsaModularSynth..."), this,
          SLOT(helpAboutAms()));
  helpMenu->addAction(tr("About &Qt..."), this,
          SLOT(helpAboutQt()));

  connect(filePopup, SIGNAL(aboutToShow()), this,
          SLOT(setupRecentFilesMenu()));
  connect(fileRecentlyOpenedFiles, SIGNAL(triggered(QAction*)), this,
        SLOT(recentFileActivated(QAction*)));


  if (pipe(pipeFd) < 0)
    return;

  QSocketNotifier *sigNotifier = new QSocketNotifier(pipeFd[0],
          QSocketNotifier::Read, this);
  QObject::connect(sigNotifier, SIGNAL(activated(int)), this,
          SLOT(unixSignal(int)));

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = sighandler;
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGUSR1, &action, NULL);

  readConfig();
  updateWindowTitle();

  if (mso.havePresetPath) {
    if (mso.verbose)
        qWarning("%s", QObject::tr("Preset path now %1").arg(mso.presetPath)
            .toUtf8().constData());
    modularSynth->setPatchPath(mso.presetPath);
  }
  modularSynth->go(mso.forceJack, mso.forceAlsa);

  // autoload patch file
  if (mso.havePreset) {
    if (mso.verbose)
        qWarning("%s", QObject::tr("Loading preset %1").arg(mso.presetName)
            .toUtf8().constData());
    openFile(mso.presetName);
  }

  if (mso.noGui)
      hide();
  else
      show();
}


MainWindow::~MainWindow()
{
    if (synthoptions->verbose)
        qWarning("%s", QObject::tr("Closing synthesizer...")
                .toUtf8().constData());
    modularSynth->clearConfig(false);
    writeConfig();

    // remove file lock
    struct flock lock = {F_WRLCK, SEEK_SET, 0, 0, 0};
    if (fcntl(rcFd, F_UNLCK, &lock) == -1) {
        qWarning("%s", QObject::tr("Could not unlock preferences file.")
                .toUtf8().constData());
    }
}

/*handle UNIX system signals*/
void MainWindow::sighandler(int s)
{
    ssize_t result;
    int pipeMessage = s;

    result = write(pipeFd[1], &pipeMessage, sizeof(pipeMessage));
    if (result == -1)
        qWarning("Error writing to pipe: %d", errno);
}

/*handle incoming unix signal messages*/
void MainWindow::unixSignal(int fd)
{
    int message;
    ssize_t result;

    result = read(fd, &message, sizeof(message));
    if (result == -1) {
        qWarning("Error reading signal message pipe: %d", errno);
        return;
    }

    switch(message) {
        case SIGINT:
            qApp->closeAllWindows();
            break;

        case SIGUSR1:
            saveFile();
            break;

        default:
            qWarning("Unexpected signal received: %d", message);
            break;
    }
}

/*check for changed file data*/
bool MainWindow::isModified()
{
    return modularSynth->isModified();
}

int MainWindow::querySaveChanges()
{
    QString queryStr;

    if (fileName.isEmpty())
        queryStr = tr("Unnamed file was changed.\nSave changes?");
    else
        queryStr = tr("File '%1' was changed.\n"
                "Save changes?").arg(fileName);

    return QMessageBox::warning(this, tr("Save changes"), queryStr,
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
}


void MainWindow::chooseFile()
{
    QString fn = QFileDialog::getOpenFileName(this,
        tr("Open patch file"), modularSynth->getPatchPath(),
        tr("AlsaModularSynth patch files") + " (*" + PATCHEXT + ")");

    if (fn.isEmpty())
        return;
    else
        openFile(fn);
}


void MainWindow::chooseDemoFile()
{
    QString fn = QFileDialog::getOpenFileName(this,
        tr("Open demo patch file"), DEMOFILEPATH,
        tr("AlsaModularSynth patch files") + " (*" + PATCHEXT + ")");

    if (fn.isEmpty())
        return;
    else
        openFile(fn);
}


void MainWindow::chooseDemoInstrumentFile()
{
    QString fn = QFileDialog::getOpenFileName(this,
        tr("Open demo instrument file"), INSTRUMENTFILEPATH,
        tr("AlsaModularSynth patch files") + " (*" + PATCHEXT + ")");

    if (fn.isEmpty())
        return;
    else
        openFile(fn);
}


void MainWindow::fileNew()
{
    if (isSave())
        newFile();
}

void MainWindow::newFile()
{
    modularSynth->clearConfig(true);

    fileName = "";
    updateWindowTitle();
}


void MainWindow::fileOpen()
{
    if (isSave())
        chooseFile();
}


void MainWindow::fileOpenDemo()
{
    if (isSave())
        chooseDemoFile();
}


void MainWindow::fileOpenDemoInstrument()
{
    if (isSave())
        chooseDemoInstrumentFile();
}


bool MainWindow::isSave()
{
    bool result = false;

    if (isModified()) {
        int choice = querySaveChanges();
        switch (choice) {
            case QMessageBox::Yes:
                if (saveFile())
                    result = true;
                break;
            case QMessageBox::No:
                result = true;
                break;
            case QMessageBox::Cancel:
            default:
                break;
        }
    }
    else
        result = true;
    return result;
}


void MainWindow::openFile(const QString& fn)
{
    QFile f(fn);

    if (!f.open(QIODevice::ReadOnly)) {
        qWarning("%s", tr("Could not read file '%1'").arg(fn)
                .toUtf8().constData());
        return;
    }

    modularSynth->setPatchPath(fn.left(fn.lastIndexOf('/')));
    fileName = fn;
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    modularSynth->load(ts);
    f.close();

    addRecentlyOpenedFile(fileName, recentFiles);
    updateWindowTitle();
}


void MainWindow::fileSave()
{
    saveFile();
}


bool MainWindow::saveFile()
{
    if (fileName.isEmpty()) {
        fileSaveAs();
        return true;
    }

    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly)) {
        qWarning("%s", tr("Could not open file '%1'").arg(fileName)
                .toUtf8().constData());
        return false;
    }

    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    modularSynth->save(ts);

    f.close();
    updateWindowTitle();
    return true;
}


void MainWindow::fileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(this,
            tr("Save patch file"), modularSynth->getPatchPath(),
            tr("AlsaModularSynth patch files") + " (*" + PATCHEXT + ")");

    if (!fn.isEmpty()) {
        /*check for file extension*/
        if (!fn.endsWith(PATCHEXT))
            fn.append(PATCHEXT);

        modularSynth->setPatchPath(fn.left(fn.lastIndexOf('/')));

        fileName = fn;
        saveFile();
    }
    else
        qWarning("%s", tr("Saving aborted").toUtf8().constData());
}


void MainWindow::updateWindowTitle()
{
    QString title = QString("%1 - (%2) - [%3]").
	arg(synthdata->name).
	arg(modularSynth->getSynthDataPoly()).
	arg(fileName.isEmpty() ? tr("noname") : fileName);
    setWindowTitle(title);
}


void MainWindow::closeEvent(QCloseEvent *e)
{
  if (isSave())
    e->accept();
  else
    e->ignore();
}

void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::readConfig()
{
    QString s;
    QFile file;

    if (!file.open(rcFd, QIODevice::ReadOnly)) {
        qWarning("%s", "Could not open preferences file.");
        return;
    }
    if (!file.seek(0)) {
        qWarning("%s", "Could not seek start of preferences file.");
        file.close();
        return;
    }
    QTextStream ts(&file);

    while (!ts.atEnd()) {
        s = ts.readLine();
        if (s.startsWith(CF_RECENTFILE))
            appendRecentlyOpenedFile(s.section(' ', 1), recentFiles);
        else if (s.startsWith(CF_RESTOREGEOMETRY))
               restoregeometry = s.section(' ', 1, 1).toInt();
        else if (s.startsWith(CF_GEOMETRY) && restoregeometry) {
                QStringList tokens = s.split(' ');
                if (tokens.count() < 5) {
                    qWarning("Geometry parameterlist too short.");
                    continue;
                }
                int x = tokens[1].toInt();
                int y = tokens[2].toInt();
                int width = tokens[3].toInt();
                int height = tokens[4].toInt();
                this->setGeometry(x, y, width, height);
            }
        else
            modularSynth->loadPreference(s);
    }
    file.close();

    modularSynth->refreshColors();
}

void MainWindow::writeConfig()
{
    QFile file;

    if (!file.open(rcFd, QIODevice::WriteOnly)) {
        qWarning("%s", "Could not open preferences file.");
        return;
    }

    if (!file.resize(0)) {
        qWarning("%s", "Could not resize preferences file.");
        file.close();
        return;
    }

    QTextStream ts(&file);
    modularSynth->savePreferences(ts);

    // save recently opened files
    if (recentFiles.count() > 0) {
        QStringList::Iterator it = recentFiles.begin();
        for (; it != recentFiles.end(); ++it) {
            ts << CF_RECENTFILE << ' ' << *it << endl;
        }
    }
    ts << CF_RESTOREGEOMETRY << ' ' << restoregeometry << endl;
    if (!this->isFullScreen())
        ts << CF_GEOMETRY << ' '
            << this->geometry().x() << ' '
            << this->geometry().y() << ' '
            << this->geometry().width() << ' '
            << this->geometry().height() << endl;

    file.close();
}

void MainWindow::setupRecentFilesMenu()
{
    fileRecentlyOpenedFiles->clear();

    if (!hiderecentfiles && (recentFiles.count() > 0)) {
        fileRecentlyOpenedFiles->setEnabled(true);
        QStringList::Iterator it = recentFiles.begin();
        for (; it != recentFiles.end(); ++it) {
            fileRecentlyOpenedFiles->addAction(*it);
        }
    } else {
        fileRecentlyOpenedFiles->setEnabled(false);
    }
}

void MainWindow::recentFileActivated(QAction *action)
{
    if (!action->text().isEmpty()) {
        if (isSave())
            openFile(action->text());
    }
}

void MainWindow::addRecentlyOpenedFile(const QString &fn, QStringList &lst)
{
    QFileInfo fi(fn);
    if (lst.contains(fi.absoluteFilePath()))
        return;
    if (lst.count() >= 6 )
        lst.removeLast();

    lst.prepend(fi.absoluteFilePath());
}

void MainWindow::appendRecentlyOpenedFile(const QString &fn, QStringList &lst)
{
    QFileInfo fi(fn);
    if (lst.contains(fi.absoluteFilePath()))
        return;
    if (lst.count() >= 6 )
        lst.removeFirst();

    lst.append(fi.absoluteFilePath());
}

#ifdef JACK_SESSION
void MainWindow::handleJackSessionEvent(int jsa)
{
    if (synthoptions->verbose)
        qWarning("JACK session action: %d", jsa);

    switch (jsa) {
        case -1:
            qApp->closeAllWindows();
            break;
        case SynthData::jsaSave:
        case SynthData::jsaSaveAndQuit:
            fileName = modularSynth->getJackSessionFilename();
            updateWindowTitle();
            if (!saveFile())
                qCritical("JACK session save file error");
            break;
        default:
            qWarning("Unsupported JACK session action: %d", jsa);
            break;
    }
}
#endif


/*show preferences dialog*/
void MainWindow::displayPreferences()
{
    if (prefDialog == NULL) {
        prefDialog = new PrefWidget(this);

        connect(prefDialog, SIGNAL(prefChanged()),
                this, SLOT(applyPreferences()));
    }
    prefDialog->show();
    prefDialog->raise();
    prefDialog->activateWindow();

    prefDialog->setRememberGeometry(restoregeometry);
    prefDialog->setHideRecentFiles(hiderecentfiles);

    prefDialog->setBackgroundColor(modularSynth->getBackgroundColor());
    prefDialog->setModuleBackgroundColor(
            modularSynth->getModuleBackgroundColor());
    prefDialog->setModuleBorderColor(modularSynth->getModuleBorderColor());
    prefDialog->setModuleFontColor(modularSynth->getModuleFontColor());
    prefDialog->setPortFontColor(modularSynth->getPortFontColor());
    prefDialog->setCableColor(modularSynth->getCableColor());
    prefDialog->setJackColor(modularSynth->getJackColor());

    prefDialog->setMidiControllerMode(modularSynth->getMidiControllerMode());

    prefDialog->setModuleMoveMode(modularSynth->getModuleMoveMode());
    prefDialog->setEnableModuleGrid(modularSynth->getEnableModuleGrid());
    prefDialog->setGridMesh(modularSynth->getModuleGrid());

    if (prefDialog->exec() == QDialog::Accepted)
        applyPreferences();
}

/*get prefered settings from dialog and apply changes*/
void MainWindow::applyPreferences()
{
    if (prefDialog == NULL)
        return;

    /*colors*/
    modularSynth->setBackgroundColor(prefDialog->getBackgroundColor());
    modularSynth->setModuleBackgroundColor(
            prefDialog->getModuleBackgroundColor());
    modularSynth->setModuleBorderColor(prefDialog->getModuleBorderColor());
    modularSynth->setModuleFontColor(prefDialog->getModuleFontColor());
    modularSynth->setPortFontColor(prefDialog->getPortFontColor());
    modularSynth->setCableColor(prefDialog->getCableColor());
    modularSynth->setJackColor(prefDialog->getJackColor());

    /*midi*/
    modularSynth->setMidiControllerMode(prefDialog->getMidiControllerMode());

    /*editing*/
    modularSynth->setModuleMoveMode(prefDialog->getModuleMoveMode());
    modularSynth->setEnableModuleGrid(prefDialog->getEnableModuleGrid());
    modularSynth->setModuleGrid(prefDialog->getGridMesh());
    restoregeometry = prefDialog->getRememberGeometry();
    if (prefDialog->getHideRecentFiles() != hiderecentfiles) {
        hiderecentfiles = prefDialog->getHideRecentFiles();
        setupRecentFilesMenu();
    }

    /*at least*/
    modularSynth->refreshColors();
}

/*show about ams dialog*/
void MainWindow::helpAboutAms()
{
    AboutDialog* ad = new AboutDialog(this);
    if (ad == NULL)
        return;

    ad->exec();
    delete ad;
}
