#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <dlfcn.h>
#include <qregexp.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>


#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <QTextStream>
#include <alsa/asoundlib.h>
#include "midipushbutton.h"
#include "synthdata.h"
#include "m_scquantizer.h"
#include "port.h"

M_scquantizer::M_scquantizer(QWidget* parent, QString *p_sclname)
  : Module(M_type_scquantizer, 2, parent, tr("Scala Quantizer"))
{
  QString qs;
  int l1;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_SCQUANTIZER_HEIGHT);
  port_M_in = new Port(tr("In"), PORT_IN, 0, this);
  port_M_trigger = new Port(tr("Trigger"), PORT_IN, 1, this);
  port_M_transpose = new Port(tr("Transpose"), PORT_IN, 2, this);
  cv.out_off = 95;
  port_out = new Port(tr("Out"), PORT_OUT, 0, this);
  port_trigger_out = new Port(tr("Trigger Out"), PORT_OUT, 1, this);
  base = 0;
  lastbase = 12;
  configDialog->addIntSlider(tr("&Note Offset"), base, -36, 36);

  sclname = "No_Scale_loaded";
  configDialog->addLabel(tr("   Scale: ") + sclname);
  configDialog->addLabel("   ");
//!!   configDialog->addPushButton("Load Scale");
//   QObject::connect(configDialog->midiPushButtonList.at(0), SIGNAL(clicked()),
//                    this, SLOT(openBrowser()));
  fileDialog = NULL;
  for (l1 = 0; l1 < 12; l1++) {
    scale_lut_isRatio[l1] = false;
    scale_lut[l1] = 100.0 + (float)l1 * 100.0;
  }
  scale_lut_isRatio[12] = true;
  scale_lut[12] = 2.0;
  scale_lut_length = 12;
  dirpath.sprintf("%s", getenv("SCALA_PATH"));
  if (dirpath.length() < 1) {
    qWarning("\nSCALA_PATH not set, assuming SCALA_PATH=/usr/share/scala");
    dirpath = "/usr/share/scala";
  } else
    StdErr << "SCALA_PATH: " << dirpath << endl;

  if (p_sclname && !p_sclname->contains("No_Scale_loaded")) {
    loadScale(dirpath + "/" + *p_sclname);
  }
}

void M_scquantizer::calcScale() {

  int l1, index;
  float base_cv, base_freq;

  lastbase = base;
  base_cv = base / 12.0;
  base_freq = synthdata->exp2_table(4.0313842 + base_cv);
  fprintf(stderr, "base: %d, base_cv: %f, base_freq: %f\n", base, base_cv, base_freq);
  scale_notes[0] = base_cv;
  index = 1;
  while (index < 128) {
    for (l1 = 0; l1 < scale_lut_length; l1++) {
      if (scale_lut_isRatio[l1]) {
        scale_notes[index] = log(base_freq * scale_lut[l1])/M_LN2 - 4.0313842;
      } else {
        scale_notes[index] = base_cv + scale_lut[l1] / 1200.0;
      }
      index++;
      if (index > 127) break;
    }
    base_cv = scale_notes[index - 1];
    base_freq = synthdata->exp2_table(4.0313842 + base_cv);
  }
}

void M_scquantizer::generateCycle() {

  int l1, l3, quant, transpose;
  unsigned int l2;
  float lutquant = 0.0;

    if (base != lastbase) {
      calcScale();
    }

    inData = port_M_in->getinputdata ();
    triggerData = port_M_trigger->getinputdata ();
    transposeData = port_M_transpose->getinputdata ();

    if (triggerData == synthdata->zeroModuleData) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          quant = 1;
          for (l2 = 0; l2 < 128; l2++) {
            if (scale_notes[quant] > 4.0 + inData[l1][l2]) {
              lutquant = scale_notes[quant-1];
              break;
            } else {
              quant++;
            }
          }
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (scale_notes[quant] > 4.0 + inData[l1][l2]) {
              lutquant = scale_notes[quant-1];
            }
            if (qsig[l1] != lutquant) {
              qsig[l1] = lutquant;
              data[1][l1][l2] = 1.0;
              trigCount[l1] = 512;
            } else {
              if (trigCount[l1] > 0) {
                data[1][l1][l2] = 1;
                trigCount[l1]--;
              } else {
                data[1][l1][l2] = 0;
              }
            }
            transpose = (int)(transposeData[l1][l2] * 12.0);
            data[0][l1][l2] = (float)qsig[l1] - 4.0 + (float)(transpose + base) / 12.0;
          }
         }
    } else {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (!trigger[l1] && (triggerData[l1][l2] > 0.5)) {
              trigger[l1] = true;
              quant = 1;
              for (l3 = 0; l3 < 128; l3++) {
                if (scale_notes[quant] > 4.0 + inData[l1][l2]) {
                  break;
                } else {
                  quant++;
                }
              }
              qsig[l1] = scale_notes[quant-1];
              data[1][l1][l2] = 1.0;
              trigCount[l1] = 512;
            } else {
              if (trigger[l1] && (triggerData[l1][l2] < 0.5)) {
                trigger[l1] = false;
              }
            }
            if (trigCount[l1] > 0) {
              data[1][l1][l2] = 1;
              trigCount[l1]--;
            } else {
              data[1][l1][l2] = 0;
            }
            transpose = (int)(transposeData[l1][l2] * 12.0);
            data[0][l1][l2] = (float)qsig[l1] - 4.0 + (float)(transpose + base) / 12.0;
          }
        }
    }
}

void M_scquantizer::openBrowser() {

  if (!fileDialog) {
    fileDialog = new QFileDialog(NULL, tr("Load Scala"), dirpath, "Scala files (*.scl)");
    QObject::connect(fileDialog, SIGNAL(currentChanged(const QString &)), this, SLOT(loadScale(const QString &)));
  }
  fileDialog->show();
}

void M_scquantizer::loadScale(const QString &p_sclname) {

  QString qs, qs2, qs3;
  int index, n;

  sclname = p_sclname;
  QFile qfile(sclname);
  if (!qfile.open(QIODevice::ReadOnly)) {
    QMessageBox::information( this, "AlsaModularSynth",
            tr("Could not load Scala file '%1'").arg(sclname));
    sclname = "No_Scale_loaded";
    return;
  }
  configDialog->labelList.at(0)->setText(tr("   Scale: ") + sclname);
  QTextStream stream(&qfile);
  while (!stream.atEnd()) {
    qs = stream.readLine();
    if (!qs.contains("!"))
      break;
  }
  configDialog->labelList.at(1)->setText("   " + qs);
  StdErr << "Scale: " << qs << endl;
  while (!stream.atEnd()) {
    qs = stream.readLine();
    if (!qs.contains("!"))
      break;
  }
  index = 0;
  while (!stream.atEnd() && (index < 128)) {
    qs = stream.readLine();
    if (qs.contains("!")) {
      continue;
    }
    qs2 = qs.simplified();
    if (qs2.contains(".")) {
      if ((n = qs2.indexOf(" ")) > 0) {
        qs = qs2.left(n);
      } else {
        qs = qs2;
      }
      scale_lut_isRatio[index] = false;
      scale_lut[index] = qs.toFloat();
      index++;
    } else {
      scale_lut_isRatio[index] = true;
      if (qs.contains("/")) {
        qs = qs2.left(qs2.indexOf("/"));
        qs3 = qs2.mid(qs2.indexOf("/") + 1);
        if ((n = qs3.indexOf(" ")) > 0) {
          qs2 = qs3.left(n);
        } else {
          qs2 = qs3;
        }
        scale_lut[index] = qs.toFloat() / qs2.toFloat();
      } else {
        if ((n = qs2.indexOf(" ")) > 0) {
          qs = qs2.left(n);
        } else {
          qs = qs2;
        }
        scale_lut[index] = qs.toFloat();
      }
      index++;
    }
  }
  scale_lut_length = index;
  calcScale();
}
