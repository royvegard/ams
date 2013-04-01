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
#include <QPolygon>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "midicombobox.h"
#include "m_function.h"
#include "port.h"


M_function::M_function(int p_functionCount, QWidget * parent)
:Module(M_type_function, p_functionCount, parent, tr("Function"))
{
    QString qs;
    QHBoxLayout *hbox;
    int l1, l2;

    setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_FUNCTION_WIDTH,
                MODULE_FUNCTION_HEIGHT + 20 + 20 * outPortCount);
    port_in = new Port(tr("In"), PORT_IN, 0, this);
    cv.out_off = 55;
    for (l1 = 0; l1 < outPortCount; l1++) {
        for (l2 = 0; l2 < MAXPOLY; l2++) {
            i[l2][l1] = 1;
            y[l1][l2] = 0;
            old_y[l1][l2] = 0;
        }

        for (l2 = 0; l2 < MAX_POINTS; l2++) {
            point[l1][l2].x = l2 - 5;
            point[l1][l2].y = (float) l1 / 10;
        }
        qs = tr("Out %1").arg(l1);
        Port *audio_out_port =
            new Port(qs, PORT_OUT, out_port_list.count(), this);
        out_port_list.append(audio_out_port);
    }
    qs = tr("Function %1 -> 1 ID %2").arg(outPortCount).arg(moduleID);
    mode = 0;
    editIndex = 0;
    configDialog->setAddStretch(-1);
    configDialog->addFunction(outPortCount, &mode, &editIndex, point,
                              MAX_POINTS);
    zoomIndex = 0;
    zoom = 1.0;
    QStringList zoomNames;
    zoomNames << "   1  " << "   2  " << "   4  " << "   8  ";
    QStringList modeNames;
    modeNames <<
        tr("Move Point") <<
        tr("Shift X") <<
        tr("Shift Y") <<
        tr("Scale X") << tr("Scale Y") << tr("Reset") << tr("Linear");
    QStringList editNames;
    editNames << "All";
    for (l1 = 0; l1 < outPortCount; l1++) {
        qs.sprintf("%d", l1);
        editNames << qs;
    }
    hbox = configDialog->addHBox();
    configDialog->addLabel(tr("Mouse X: _____ Y: _____"), hbox);
    hbox = configDialog->addHBox();
    configDialog->addComboBox(tr("&Mode"), mode, modeNames, hbox);
    configDialog->addComboBox(tr("&Edit function"), editIndex, editNames,
                              hbox);
    configDialog->addComboBox(tr("&Zoom"), zoomIndex, zoomNames, hbox);
    connect(configDialog->midiComboBoxList.at(2)->comboBox,
            SIGNAL(currentIndexChanged(int)), this, SLOT(updateZoom(int)));
    connect(configDialog->midiComboBoxList.at(1)->comboBox,
            SIGNAL(currentIndexChanged(int)),
            configDialog->functionList.at(0),
            SLOT(highlightFunction(int)));
    connect(configDialog->functionList.at(0), SIGNAL(mousePos(int, int)),
            this, SLOT(updateMouseLabels(int, int)));
}

void M_function::generateCycle()
{
    int l1, l2, l3, k, len, l2_out;
    int pointCount;
    Function *cf;
    float xg, dy;

    inData = port_in->getinputdata();
    cf = configDialog->functionList.at(0);
    pointCount = configDialog->functionList.at(0)->pointCount;
    for (l3 = 0; l3 < outPortCount; l3++) {
        for (l1 = 0; l1 < synthdata->poly; l1++) {
            len = synthdata->cyclesize;
            l2 = -1;
            l2_out = 0;
            do {
                k = (len > 24) ? 16 : len;
                l2 += k;
                len -= k;
                xg = inData[l1][l2];
                while (xg < cf->f[0][l3][i[l1][l3]])
                    i[l1][l3]--;
                while (xg >= cf->f[0][l3][i[l1][l3] + 1])
                    i[l1][l3]++;
                if (i[l1][l3] < 1) {
                    y[l3][l1] = cf->f[1][l3][1];
                }
                else if (i[l1][l3] >= pointCount) {
                    y[l3][l1] = cf->f[1][l3][pointCount];
                }
                else {
                    y[l3][l1] =
                        cf->f[1][l3][i[l1][l3]] + (xg -
                                                   cf->f[0][l3][i[l1][l3]])
                        * (cf->f[1][l3][i[l1][l3] + 1] -
                           cf->f[1][l3][i[l1][l3]])
                        / (cf->f[0][l3][i[l1][l3] + 1] -
                           cf->f[0][l3][i[l1][l3]]);
                }
                dy = (y[l3][l1] - old_y[l3][l1]) / (double) k;
                while (k--) {
                    old_y[l3][l1] += dy;
                    data[l3][l1][l2_out++] = old_y[l3][l1];
                }
            } while (len);
        }
    }
}

void M_function::updateZoom(int zoomIndex)
{
    // zoomIndex is may _not_ be already set in MidiComboBox event handler
    zoom = pow(2.0, zoomIndex);
    configDialog->functionList.at(0)->setZoom(zoom);
}

void M_function::updateMouseLabels(int x, int y)
{
    QString qs;

    qs.sprintf("Mouse X: %6.3f  Y: %6.3f",
               (float) (x - FUNCTION_CENTER_X) / (float) FUNCTION_SCALE,
               (float) (FUNCTION_CENTER_Y - y) / (float) FUNCTION_SCALE);
    configDialog->labelList.at(0)->setText(qs);
}
