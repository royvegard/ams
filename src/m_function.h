#ifndef M_FUNCTION_H
#define M_FUNCTION_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>


#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
//#include <q3pointarray.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"
#include "function.h"

#define MODULE_FUNCTION_WIDTH                 85
#define MODULE_FUNCTION_HEIGHT                40



class M_function:public Module {
  Q_OBJECT private:
    QList < Port * >out_port_list;
    Port *port_in;
    float zoom;
    float y[MAX_FUNCTIONS][MAXPOLY], old_y[MAX_FUNCTIONS][MAXPOLY];
    tFunction point;
    int i[MAXPOLY][MAX_FUNCTIONS];
    int zoomIndex, editIndex;
    int mode;

  public:
    float **inData;

  public:
     M_function(int p_functionCount, QWidget * parent = 0);

    void generateCycle();

    public slots: void updateZoom(int p_zoomIndex);
    void updateMouseLabels(int x, int y);
};

#endif
