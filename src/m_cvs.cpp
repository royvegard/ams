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
#include "m_cvs.h"
#include "port.h"

M_cvs::M_cvs(QWidget* parent)
  : Module(M_type_cvs, MODULE_CVS_CV_COUNT, parent, tr("CV Source"))
{
  QString qs;
  int l1;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_CVS_HEIGHT);
  for (l1 = 0; l1 < MODULE_CVS_CV_COUNT; l1++) {
    qs.sprintf("CV %d", l1);
    port_cv_out[l1] = new Port(qs, PORT_OUT, l1, this);
  }

  for (l1 = 0; l1 < MODULE_CVS_CV_COUNT; l1++) {
    cv[l1] = 0;
    cv_fine[l1] = 0;
    qs.sprintf("&CV %d", l1);
    configDialog->addSlider(qs, cv[l1], 0, 5);
    qs.sprintf("CV %d &Fine", l1);
    configDialog->addSlider(qs, cv_fine[l1], -0.5, 0.5);
  }
}

void M_cvs::generateCycle() {

  int l1, l3;
  unsigned int l2;

    for (l3 = 0; l3 < MODULE_CVS_CV_COUNT; l3++) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[l3][l1][l2] = cv[l3] + cv_fine[l3];
        }
      }
    }
}

