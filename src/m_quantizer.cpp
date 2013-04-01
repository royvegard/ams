#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_quantizer.h"
#include "port.h"

M_quantizer::M_quantizer(QWidget* parent)
  : Module(M_type_quantizer, 2, parent, tr("Quantizer"))
{
  QString qs;
  int l1;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_QUANTIZER_HEIGHT);
  port_M_in = new Port(tr("In"), PORT_IN, 0, this);
  port_M_trigger = new Port(tr("Trigger"), PORT_IN, 1, this);
  port_M_transpose = new Port(tr("Transpose"), PORT_IN, 2, this);
  cv.out_off = 95;
  port_out = new Port("Out", PORT_OUT, 0, this);
  port_trigger_out = new Port(tr("Trigger Out"), PORT_OUT, 1, this);
  quantum = QUANT_12;
  QStringList quantumNames ;
  quantumNames <<
    tr("1/12") <<
    tr("1/6") <<
    tr("Major Scale") <<
    tr("Minor Scale") <<
    tr("Major Chord") <<
    tr("Minor Chord") <<
    tr("Major 7 Chord") <<
    tr("Minor 7 Chord") <<
    tr("Major 6 Chord") <<
    tr("Minor 6 Chord") <<
    tr("Pentatonic");
  configDialog->addComboBox(tr("&Quantization"), quantum, quantumNames);

  for (l1 = 0; l1 < 12; l1++) {
    lut[0][l1] = l1;
    lut[1][l1] = (int)(l1 / 2) * 2;
  }
  lut[2][0] = 0;
  lut[2][1] = 0;
  lut[2][2] = 2;
  lut[2][3] = 2;
  lut[2][4] = 4;
  lut[2][5] = 5;
  lut[2][6] = 5;
  lut[2][7] = 7;
  lut[2][8] = 7;
  lut[2][9] = 9;
  lut[2][10] = 9;
  lut[2][11] = 11;
  lut[3][0] = 0;
  lut[3][1] = 0;
  lut[3][2] = 2;
  lut[3][3] = 3;
  lut[3][4] = 3;
  lut[3][5] = 5;
  lut[3][6] = 5;
  lut[3][7] = 7;
  lut[3][8] = 8;
  lut[3][9] = 8;
  lut[3][10] = 10;
  lut[3][11] = 10;
  lut[4][0] = 0;
  lut[4][1] = 0;
  lut[4][2] = 0;
  lut[4][3] = 0;
  lut[4][4] = 4;
  lut[4][5] = 4;
  lut[4][6] = 4;
  lut[4][7] = 7;
  lut[4][8] = 7;
  lut[4][9] = 7;
  lut[4][10] = 7;
  lut[4][11] = 7;
  lut[5][0] = 0;
  lut[5][1] = 0;
  lut[5][2] = 0;
  lut[5][3] = 3;
  lut[5][4] = 3;
  lut[5][5] = 3;
  lut[5][6] = 3;
  lut[5][7] = 7;
  lut[5][8] = 7;
  lut[5][9] = 7;
  lut[5][10] = 7;
  lut[5][11] = 7;
  lut[6][0] = 0;
  lut[6][1] = 0;
  lut[6][2] = 0;
  lut[6][3] = 0;
  lut[6][4] = 4;
  lut[6][5] = 4;
  lut[6][6] = 4;
  lut[6][7] = 7;
  lut[6][8] = 7;
  lut[6][9] = 7;
  lut[6][10] = 7;
  lut[6][11] = 11;
  lut[7][0] = 0;
  lut[7][1] = 0;
  lut[7][2] = 0;
  lut[7][3] = 3;
  lut[7][4] = 3;
  lut[7][5] = 3;
  lut[7][6] = 3;
  lut[7][7] = 7;
  lut[7][8] = 7;
  lut[7][9] = 7;
  lut[7][10] = 10;
  lut[7][11] = 10;
  lut[8][0] = 0;
  lut[8][1] = 0;
  lut[8][2] = 0;
  lut[8][3] = 0;
  lut[8][4] = 4;
  lut[8][5] = 4;
  lut[8][6] = 4;
  lut[8][7] = 7;
  lut[8][8] = 7;
  lut[8][9] = 9;
  lut[8][10] = 9;
  lut[8][11] = 9;
  lut[9][0] = 0;
  lut[9][1] = 0;
  lut[9][2] = 0;
  lut[9][3] = 3;
  lut[9][4] = 3;
  lut[9][5] = 3;
  lut[9][6] = 3;
  lut[9][7] = 7;
  lut[9][8] = 7;
  lut[9][9] = 9;
  lut[9][10] = 9;
  lut[9][11] = 9;
  lut[10][0] = 0;
  lut[10][1] = 0;
  lut[10][2] = 2;
  lut[10][3] = 2;
  lut[10][4] = 2;
  lut[10][5] = 5;
  lut[10][6] = 5;
  lut[10][7] = 7;
  lut[10][8] = 7;
  lut[10][9] = 9;
  lut[10][10] = 9;
  lut[10][11] = 9;
}

void M_quantizer::generateCycle() {

  int l1, quant, lutquant, transpose;
  unsigned int l2;

    inData = port_M_in->getinputdata ();
    triggerData = port_M_trigger->getinputdata ();
    transposeData = port_M_transpose->getinputdata ();

    if (triggerData == synthdata->zeroModuleData) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            quant = (int)((100.0 + inData[l1][l2]) * 12.0);
            lutquant = lut[quantum][quant % 12] + (int)(quant / 12) * 12;
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
            data[0][l1][l2] = (float)qsig[l1] / 12.0 - 100 + (float)transpose / 12.0;
          }
         }
    } else {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
          for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
            if (!trigger[l1] && (triggerData[l1][l2] > 0.5)) {
              trigger[l1] = true;
              qsig[l1] = (int)((100.0 + inData[l1][l2]) * 12.0);
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
            data[0][l1][l2] = (float)lut[quantum][qsig[l1] % 12] / 12.0 + qsig[l1] / 12 - 100
                            + (float)transpose / 12.0;
          }
        }
    }
}

