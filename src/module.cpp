#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#include "configdialog.h"
#include "intmidislider.h"
#include "main.h"
#include "midicheckbox.h"
#include "midicombobox.h"
#include "midicontrollable.h"
#include "midislider.h"
#include "modularsynth.h"
#include "module.h"
#include "port.h"
#include "synthdata.h"


size_t Module::portmemAllocated;
Module::CtorVar Module::cv;

Module::Module(M_typeEnum M_type, int outPortCount, QWidget* parent,
        const QString &name)
  : Box(parent, name)
  , alive(true)
  , connections(0)
  , data(NULL)
  , cycleReady(false)
  , M_type(M_type)
  , outPortCount(outPortCount)
{
  cv.reset();

  synthdata->incModuleCount();
  moduleID = synthdata->getModuleID();
  getColors();
  synthdata->moduleList.append(this);
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH,
          MODULE_DEFAULT_HEIGHT);

  configDialog = new ConfigDialog(*this);
  configDialog->setWindowTitle(name + " ID " + QString::number(moduleID));
  QObject::connect(configDialog, SIGNAL(removeModuleClicked()),
          this, SLOT(removeThisModule()));
  if (outPortCount > 0)
    portMemAlloc(outPortCount, true);
}

void Module::portMemAlloc(int outports, bool poly)
{
    // TODO Caution, if poly is changed
    outPortCount = outports;
    data = (float ***)malloc(outPortCount * sizeof(float **));
    int voices = poly ? synthdata->poly : 1;

    for (int l1 = 0; l1 < outPortCount; ++l1) {
        data[l1] = (float **)malloc(synthdata->poly * sizeof(float *));
        size_t size = voices * synthdata->periodsize * sizeof(float);
        data[l1][0] = (float *)malloc(size);
        memset(data[l1][0], 0, size);
        portmemAllocated += size;
        for (int l2 = 1; l2 < synthdata->poly; ++l2)
            data[l1][l2] = data[l1][l2 - 1] +
                (poly ? synthdata->periodsize : 0);
    }
}

void Module::Delete()
{
  int k;

  pthread_mutex_lock(&synthdata->rtMutex);
  switch (M_type) {
  default:
    break;
  case M_type_pcmout:
    k = synthdata->find_play_mod(this);
    if (k >= 0)
      synthdata->set_play_mod(k, 0);
    break;
  case M_type_pcmin:
    k = synthdata->find_capt_mod(this);
    if (k >= 0)
      synthdata->set_capt_mod(k, 0);
    break;
  case M_type_wavout:
    synthdata->wavoutModuleList.removeAll((M_wavout *)this);
    break;
  case M_type_scope:
    synthdata->scopeModuleList.removeAll((M_scope *)this);
    break;
  case M_type_midiout:
    synthdata->midioutModuleList.removeAll((M_midiout *)this);
    break;
#ifdef OUTDATED_CODE
  case M_type_spectrum:
    synthdata->spectrumModuleList.remove((QObject *)m);
    break;
#endif
  }
  pthread_mutex_unlock(&synthdata->rtMutex);

  bool updateActiveMidiControllers = true;
  alive = false;
  for (typeof(midiControllables.begin()) mcAI = midiControllables.begin();
       mcAI != midiControllables.end();
       ++mcAI)
    (*mcAI)->disconnect(&updateActiveMidiControllers);

  synthdata->midiWidget->removeModule(this);

  deleteLater();
}

Module::~Module()
{
    int l1;

    delete configDialog;

    qDeleteAll(midiControllables.begin(), midiControllables.end());

    for (l1 = 0; l1 < portList.count(); ++l1) {
        Port *port = portList.at(l1);
        if (port != NULL)
            port->removeAllConnectedPorts();
    }

    for (l1 = 0; l1 < outPortCount; ++l1) {
        free(data[l1][0]);
        portmemAllocated -= synthdata->poly * synthdata->periodsize
            * sizeof(float);
        free(data[l1]);
    }

    free(data);
}

void Module::addPort(Port *p)
{
    portList.append(p);
    if (p->isInPort()) {
        p->move(0, cv.in_off + cv.in_index * cv.step);
        cv.in_index++;
    } else {
        p->move(width() - p->width(), cv.out_off + cv.out_index * cv.step);
        cv.out_index++;
    }
    connect(p, SIGNAL(portClicked(Port*)), this, SIGNAL(portSelected(Port*)));
    connect(p, SIGNAL(portDisconnected()), this, SIGNAL(portDisconnected()));
}

void Module::paintCablesToConnectedPorts(QPainter& painter)
{
    int inportx, inporty, outportx, outporty;

    for (int i = 0; i < portList.count(); ++i) {
        Port* inport = portList.at(i);
        Port* outport = inport->needsConnectionToPort();
        if (outport == NULL)
            continue;

        QColor cableColor, jackColor;
        QPen pen;
        QPainterPath path;
        int xShift = 30;
        cableColor = inport->cableColor;
        jackColor = inport->jackColor;

        // calculate cable path
        inportx = inport->pos().x() + x() - 10;
        outportx = outport->pos().x() + outport->width() +
            outport->module->x() + 10;
        inporty = inport->pos().y() + y() + inport->height()/2;
        outporty = outport->pos().y() + outport->module->y() +
            outport->height()/2;
        if (outportx > inportx)
            xShift += (outportx - inportx) >> 3;

        path.moveTo(inportx, inporty);
        path.cubicTo(inportx - xShift, inporty + 3,
                outportx + xShift, outporty + 3, outportx, outporty);

        // paint cable
        pen.setWidth(5);
        pen.setColor(cableColor.dark(120));
        painter.strokePath(path, pen);

        pen.setWidth(3);
        pen.setColor(cableColor);
        painter.strokePath(path, pen);

        pen.setWidth(1);
        pen.setColor(cableColor.light(120));
        painter.strokePath(path, pen);

        // paint jack
        painter.fillRect(inportx, inporty - 3, 11, 7,
                QBrush(jackColor.dark(120)));
        painter.fillRect(outportx - 11, outporty - 3, 11, 7,
                QBrush(jackColor.dark(120)));
        painter.fillRect(inportx, inporty - 2, 11, 5,
                QBrush(jackColor));
        painter.fillRect(outportx - 11, outporty - 2, 11, 5,
                QBrush(jackColor));
        painter.fillRect(inportx, inporty - 1, 11, 3,
                QBrush(jackColor.light(120)));
        painter.fillRect(outportx - 11, outporty - 1, 11, 3,
                QBrush(jackColor.light(120)));
    }
}

void Module::paint(QPainter &p)
{
  QString  qs;

  p.setPen(colorBorder);
  for (int i = 0; i < 3; ++i)
  {
      p.setPen(colorBorder.light(100 + 15 * i));
      p.drawRect(i, i, width() - 2 * i - 1, height() - 2 * i - 1);
  }
  p.setPen(colorFont);
  p.setFont(synthdata->bigFont);
  p.drawText(10, 20, objectName());
  p.setFont(synthdata->smallFont);
  qs = tr("ID %1").arg(moduleID);
  p.drawText(10, 32, qs);
}

void Module::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  paint(p);
}

void Module::showConfigDialog(const QPoint& pos)
{
    configDialog->move(pos);
    configDialog->show();
    configDialog->raise();
}

void Module::removeThisModule()
{
  emit removeModule();
}

void Module::save(QTextStream& ts)
{
  saveConnections(ts);
  saveParameters(ts);
  saveBindings(ts);
  saveConfigDialog(ts);
}

int Module::connected()
{
    return connections;
}

void Module::saveConnections(QTextStream& ts)
{
    Port *inport;
    Port *outport;

    for (int l1 = 0; l1 < portList.count(); ++l1) {
        inport = portList.at(l1);
        outport = inport->needsConnectionToPort();
        if (outport != NULL) {
            ts << "ColorP "
                << inport->index << ' '
                << outport->index << ' '
                << inport->module->moduleID << ' '
                << outport->module->moduleID << ' '
                << inport->jackColor.red() << ' '
                << inport->jackColor.green() << ' '
                << inport->jackColor.blue() << ' '
                << inport->cableColor.red() << ' '
                << inport->cableColor.green() << ' '
                << inport->cableColor.blue() << endl;
        }
    }
}

void Module::saveParameters(QTextStream& ts)
{
  int l1, l2, l3;

  for (l1 = 0; l1 < configDialog->midiSliderList.count(); ++l1) {
    MidiControllableFloat &mcAbleF = dynamic_cast<MidiControllableFloat &>(configDialog->midiSliderList.at(l1)->mcAble);
    ts << "FSlider " << moduleID << ' ' << l1 << ' '
        << mcAbleF.sliderVal() << ' '
        << mcAbleF.getLog() << ' '
        << mcAbleF.sliderMin() << ' '
        << mcAbleF.sliderMax() << ' '
        << mcAbleF.midiSign << endl;
  }
  for (l1 = 0; l1 < configDialog->intMidiSliderList.count(); ++l1)
    ts << "ISlider " << moduleID << ' ' << l1 << ' '
        << configDialog->intMidiSliderList.at(l1)->mcAble.sliderVal() << ' '
        << configDialog->intMidiSliderList.at(l1)->mcAble.midiSign << endl;

  for (l1 = 0; l1 < configDialog->floatIntMidiSliderList.count(); ++l1)
      ts << "LSlider " << moduleID << ' ' << l1 << ' '
          << configDialog->floatIntMidiSliderList.at(l1)->mcAble.sliderVal() << ' '
          << configDialog->floatIntMidiSliderList.at(l1)->mcAble.midiSign << endl;

  for (l1 = 0; l1 < configDialog->midiComboBoxList.count(); ++l1)
    ts << "ComboBox " << moduleID << ' ' << l1 << ' '
        << configDialog->midiComboBoxList.at(l1)->mcAble.getValue() << ' '
        << configDialog->midiComboBoxList.at(l1)->mcAble.midiSign << endl;

  for (l1 = 0; l1 < configDialog->midiCheckBoxList.count(); ++l1)
    ts << "CheckBox " << moduleID << ' ' << l1 << ' '
        << configDialog->midiCheckBoxList.at(l1)->mcAble.getValue() << ' '
        << configDialog->midiCheckBoxList.at(l1)->mcAble.midiSign << endl;

  for (l1 = 0; l1 < configDialog->functionList.count(); ++l1) {
    ts << "Function " << moduleID << ' ' << l1 << ' '
        << configDialog->functionList.at(l1)->functionCount << ' '
        << configDialog->functionList.at(l1)->pointCount << endl;

    for (l2 = 0; l2 < configDialog->functionList.at(l1)->functionCount; l2++)
      for (l3 = 0; l3 < configDialog->functionList.at(l1)->pointCount; l3++) {
        ts << "Point " << moduleID << ' ' << l1 << ' ' << l2 << ' ' << l3 << ' '
            << configDialog->functionList.at(l1)->getPoint(l2, l3).x() << ' '
            << configDialog->functionList.at(l1)->getPoint(l2, l3).y() << endl;
      }

  }
}

void Module::saveBindings(QTextStream& ts)
{
  int l1, l2;

  for (l1 = 0; l1 < configDialog->midiSliderList.count(); ++l1) {
     for (l2 = 0; l2 < configDialog->midiSliderList.at(l1)
             ->mcAble.midiControllerList.count(); ++l2) {
       ts << "FSMIDI " << moduleID << ' ' << l1 << ' '
           << configDialog->midiSliderList.at(l1)
           ->mcAble.midiControllerList.at(l2).type() << ' '
           << configDialog->midiSliderList.at(l1)
           ->mcAble.midiControllerList.at(l2).ch() << ' '
           << configDialog->midiSliderList.at(l1)
           ->mcAble.midiControllerList.at(l2).param() << endl;
     }
  }
  for (l1 = 0; l1 < configDialog->intMidiSliderList.count(); ++l1) {
      for (l2 = 0; l2 < configDialog->intMidiSliderList.at(l1)
              ->mcAble.midiControllerList.count(); ++l2) {
          ts << "ISMIDI " << moduleID << ' ' << l1 << ' '
              << configDialog->intMidiSliderList.at(l1)
              ->mcAble.midiControllerList.at(l2).type() << ' '
              << configDialog->intMidiSliderList.at(l1)
              ->mcAble.midiControllerList.at(l2).ch() << ' '
              << configDialog->intMidiSliderList.at(l1)
              ->mcAble.midiControllerList.at(l2).param() << endl;
    }
  }
  for (l1 = 0; l1 < configDialog->floatIntMidiSliderList.count(); ++l1) {
      for (l2 = 0; l2 < configDialog->floatIntMidiSliderList.at(l1)
              ->mcAble.midiControllerList.count(); ++l2) {
          ts << "LSMIDI " << moduleID << ' ' << l1 << ' '
              << configDialog->floatIntMidiSliderList.at(l1)
              ->mcAble.midiControllerList.at(l2).type() << ' '
              << configDialog->floatIntMidiSliderList.at(l1)
              ->mcAble.midiControllerList.at(l2).ch() << ' '
              << configDialog->floatIntMidiSliderList.at(l1)
              ->mcAble.midiControllerList.at(l2).param() << endl;
    }
  }
  for (l1 = 0; l1 < configDialog->midiComboBoxList.count(); ++l1) {
      for (l2 = 0; l2 < configDialog->midiComboBoxList.at(l1)
              ->mcAble.midiControllerList.count(); ++l2) {
          ts << "CMIDI " << moduleID << ' ' << l1 << ' '
              << configDialog->midiComboBoxList.at(l1)
              ->mcAble.midiControllerList.at(l2).type() << ' '
              << configDialog->midiComboBoxList.at(l1)
              ->mcAble.midiControllerList.at(l2).ch() << ' '
              << configDialog->midiComboBoxList.at(l1)
              ->mcAble.midiControllerList.at(l2).param() << endl;
    }
  }
  for (l1 = 0; l1 < configDialog->midiCheckBoxList.count(); ++l1) {
      for (l2 = 0; l2 < configDialog->midiCheckBoxList.at(l1)
              ->mcAble.midiControllerList.count(); ++l2) {
          ts << "TMIDI " << moduleID << ' ' << l1 << ' '
              << configDialog->midiCheckBoxList.at(l1)
              ->mcAble.midiControllerList.at(l2).type() << ' '
              << configDialog->midiCheckBoxList.at(l1)
              ->mcAble.midiControllerList.at(l2).ch() << ' '
              << configDialog->midiCheckBoxList.at(l1)
              ->mcAble.midiControllerList.at(l2).param() << endl;
    }
  }
}

/*Write visibility, position and size parameters of config dialog*/
void Module::saveConfigDialog(QTextStream& ts)
{

    ts << "ConfigDialog " << configDialog->isVisible() << ' '
        << configDialog->geometry().x() << ' '
        << configDialog->geometry().y() << ' '
        << configDialog->geometry().width() << ' '
        << configDialog->geometry().height() << endl;
}

/*Read visibility, position and size parameters of config dialog*/
void Module::readConfigDialog(QString& line)
{
    QStringList tokens = line.split(' ');
    if (tokens.count() < 6) {
        qWarning("Config dialog geometry parameterlist too short "
                "(module = %d).", moduleID);
        return;
    }
    bool isvisible = (tokens[1].toInt() > 0);
    int x = tokens[2].toInt();
    int y = tokens[3].toInt();
    int width = tokens[4].toInt();
    int height = tokens[5].toInt();
    configDialog->setGeometry(x, y, width, height);
    if (isvisible)
        configDialog->show();
}

void Module::getColors(void)
{
  QColor alphaBack(synthdata->colorModuleBackground);
  alphaBack.setAlpha(203);
  setPalette(QPalette(alphaBack, alphaBack));
  colorBorder = synthdata->colorModuleBorder;
  colorFont = synthdata->colorModuleFont;

  // update also port colors
  for (int l2 = 0; l2 < portList.count(); ++l2) {
      Port* port = portList.at(l2);
      port->setPalette(QPalette(
                  synthdata->colorModuleBackground,
                  synthdata->colorModuleBackground));
      if (port->isInPort())
          port->cableColor = synthdata->colorCable;
      else
          port->jackColor= synthdata->colorJack;
  }
}

float **Module::getData(int index)
{
    if (!cycleReady) {
        cycleReady = true;
        generateCycle();
    }
    return data[index];
}

void Module::incConnections()
{
  if (connections++ == 0)
    configDialog->removeButtonShow(false);
}

void Module::decConnections()
{
  if (--connections == 0)
    configDialog->removeButtonShow(true);
}

bool Module::hasModuleId(int id)
{
    return (moduleID == id);
}

void Module::setModuleId(int id)
{
    QString qs, qs2;

    if (moduleID != id) {
        moduleID = id;
        qs = configDialog->windowTitle();
        qs2 = qs.left(qs.lastIndexOf(' '));
        qs.sprintf(" %d", moduleID);
        configDialog->setWindowTitle(qs2+qs);
    }
}

MidiControllableBase* Module::getMidiControlableBase(int idx)
{
    MidiControllableBase* mcb = NULL;

    if ((idx + 1) > midiControllables.count())
        qWarning("MidiControllableBase index out of range (value = %d)", idx);
    else
        mcb = midiControllables.at(idx);
    return mcb;
}

Port* Module::getPortAt(int idx)
{
    Port* p = NULL;

    if ((idx + 1) > portList.count()) {
        qWarning("Port index out of range (value = %d).", idx);
    }
    else
        p = portList.at(idx);

    return p;
}

Port* Module::getPortWithIndex(int idx)
{
    Port* p = NULL;

    for (int i = 0; i < portList.count(); ++i) {
        p = portList.at(i);
        if (p != NULL && p->hasIndex(idx)) {
            break;
        }
    }
    if (p == NULL)
        qWarning("No port with index %d found.", idx);

    return p;
}

Port* Module::getInPortWithIndex(int idx)
{
    Port* p = NULL;

    for (int i = 0; i < portList.count(); ++i) {
        p = portList.at(i);
        if (p != NULL && p->hasIndex(idx) && p->isInPort()) {
            break;
        }
    }
    if (p == NULL)
        qWarning("No input port with index %d found.", idx);

    return p;
}

Port* Module::getOutPortWithIndex(int idx)
{
    Port* p = NULL;

    for (int i = 0; i < portList.count(); ++i) {
        p = portList.at(i);
        if (p != NULL && p->hasIndex(idx) && !p->isInPort()) {
            break;
        }
    }
    if (p == NULL)
        qWarning("No output port with index %d found.", idx);

    return p;
}

void Module::setJackColor(QColor& color)
{
    Port* p = NULL;

    for (int i = 0; i < portList.count(); ++i) {
        p = portList.at(i);
        if (p != NULL && p->isInPort()) {
            p->setJackColor(color);
        }
    }
}

void Module::setCableColor(QColor& color)
{
    Port* p = NULL;

    for (int i = 0; i < portList.count(); ++i) {
        p = portList.at(i);
        if (p != NULL && p->isInPort()) {
            p->setCableColor(color);
        }
    }
}
