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
#include <QPaintEvent>
#include <alsa/asoundlib.h>
#include <ladspa.h>
#include "synthdata.h"
#include "m_ladspa.h"
#include "port.h"

#include "../pixmaps/ladspa_logo.xpm"


M_ladspa::M_ladspa(QWidget* parent, int ladspaDesFuncIndex, int n,
        bool poly, bool extCtrlPorts)
  : Module(M_type_ladspa, 0, parent, QString(poly ? tr("Poly ") : "")
          + synthdata->ladspaLib.at(ladspaDesFuncIndex).desc.at(n)->Label)
  , ladspa_dsc(synthdata->ladspaLib.at(ladspaDesFuncIndex).desc.at(n))
  , ladspaPoly(poly ? synthdata->poly : 1)
  , ladspaDesFuncIndex(ladspaDesFuncIndex)
  , isPoly(poly)
{
    QString qs;
    int l1, l2, itmp, port_ofs;
    int audio_in_index = 0, audio_out_index = 0,
        ctrl_in_index = 0, ctrl_out_index = 0,
        control_port_count = 0;
    float control_min, control_max;
    bool tabMode;
    QVBoxLayout *ladspaTab;

    setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_LADSPA_WIDTH,
            MODULE_LADSPA_HEIGHT);
    logo = QPixmap(ladspa_logo_xpm);
    ladspaTab = 0;
    hasExtCtrlPorts = extCtrlPorts;
    //  fprintf(stderr, "new LADSPA module, Poly: %d\n", (int)isPoly);
    rate_factor = 1.0;

    ladspa_audio_in_count = 0;
    ladspa_audio_out_count = 0;
    ladspa_ctrl_in_count = 0;
    ladspa_ctrl_out_count = 0;

    for (unsigned l1 = 0; l1 < ladspa_dsc->PortCount; l1++) {
        if (LADSPA_IS_PORT_AUDIO(ladspa_dsc->PortDescriptors[l1])) {
            if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
                ladspa_audio_in_count++;
            }
            if (LADSPA_IS_PORT_OUTPUT(ladspa_dsc->PortDescriptors[l1])) {
                ladspa_audio_out_count++;
            }
        }
        if (LADSPA_IS_PORT_CONTROL(ladspa_dsc->PortDescriptors[l1])) {
            if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
                ladspa_ctrl_in_count++;
            }
            if (LADSPA_IS_PORT_OUTPUT(ladspa_dsc->PortDescriptors[l1])) {
                ladspa_ctrl_out_count++;
            }
        }
    }
    if (ladspa_audio_in_count > MAX_AUDIOPORTS)
        ladspa_audio_in_count = MAX_AUDIOPORTS;
    if (ladspa_audio_out_count > MAX_AUDIOPORTS)
        ladspa_audio_out_count = MAX_AUDIOPORTS;
    if (ladspa_ctrl_in_count > MAX_CONTROLPORTS)
        ladspa_ctrl_in_count = MAX_CONTROLPORTS;
    if (ladspa_ctrl_out_count > MAX_CONTROLPORTS)
        ladspa_ctrl_out_count = MAX_CONTROLPORTS;
    portMemAlloc(ladspa_audio_out_count + ladspa_ctrl_out_count, poly);

    tabMode = ladspa_ctrl_in_count > MAX_LADPSA_CONTROLS_PER_TAB;

    for (l1 = 0; l1 < ladspaPoly; l1++) {
        for (l2 = 0; l2 < ladspa_audio_in_count; l2++) {
            ladspaDataIn[l2][l1] =
                (LADSPA_Data *)malloc(synthdata->periodsize * sizeof(LADSPA_Data));
            memset(ladspaDataIn[l2][l1], 0,
                    synthdata->periodsize * sizeof(LADSPA_Data));
        }
        ladspa_handle[l1] = ladspa_dsc->instantiate(ladspa_dsc, synthdata->rate);
    }
    if (ladspa_dsc->activate)
        for (l1 = 0; l1 < ladspaPoly; l1++)
            ladspa_dsc->activate(ladspa_handle[l1]);

    configDialog->addLabel(tr("Name: ") + ladspa_dsc->Name);
    configDialog->addLabel(tr("Author: ") + ladspa_dsc->Maker);
    configDialog->addLabel(tr("Copyright: ") + ladspa_dsc->Copyright);
    if (tabMode)
        configDialog->initTabWidget();

    port_ofs = 35;

    for (unsigned l1 = 0; l1 < ladspa_dsc->PortCount; l1++) {
        if (LADSPA_IS_PORT_AUDIO(ladspa_dsc->PortDescriptors[l1])) {
            if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
                Port *audio_in_port = new Port(ladspa_dsc->PortNames[l1],
                        PORT_IN, in_port_list.count() +
                        in_ctrl_port_list.count(), this);
                in_port_list.append(audio_in_port);
                for (l2 = 0; l2 < ladspaPoly; l2++)
                    ladspa_dsc->connect_port(ladspa_handle[l2], l1,
                            ladspaDataIn[audio_in_index][l2]);

                audio_in_index++;
            }
            if (LADSPA_IS_PORT_OUTPUT(ladspa_dsc->PortDescriptors[l1])) {
                Port *audio_out_port = new Port(ladspa_dsc->PortNames[l1],
                        PORT_OUT, out_port_list.count() +
                        out_ctrl_port_list.count(), this);
                out_port_list.append(audio_out_port);
                for (l2 = 0; l2 < ladspaPoly; l2++)
                    ladspa_dsc->connect_port(ladspa_handle[l2], l1,
                            data[out_port_list.count() +
                            out_ctrl_port_list.count() - 1][l2]);

                audio_index[audio_out_index] = out_port_list.count() +
                    out_ctrl_port_list.count() - 1;
                // fprintf(stderr, "audio_index[audio_out_index] = audio_index[%d] = %d\n", audio_out_index, audio_index[audio_out_index]);
                audio_out_index++;
            }
        }
        if (LADSPA_IS_PORT_CONTROL(ladspa_dsc->PortDescriptors[l1])) {
            controlPortRate[control_port_count] = false;
            if (hasExtCtrlPorts) {
                if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
                    Port *ctrl_in_port = new Port(ladspa_dsc->PortNames[l1],
                            PORT_IN, in_port_list.count() +
                            in_ctrl_port_list.count(), this);
                    in_ctrl_port_list.append(ctrl_in_port);
                    // fprintf(stderr, "input: %s\n", ladspa_dsc->PortNames[l1]);
                }
                if (LADSPA_IS_PORT_OUTPUT(ladspa_dsc->PortDescriptors[l1])) {
                    Port *ctrl_out_port = new Port(ladspa_dsc->PortNames[l1],
                            PORT_OUT, out_port_list.count() +
                            out_ctrl_port_list.count(), this);
                    out_ctrl_port_list.append(ctrl_out_port);
                    if (LADSPA_IS_HINT_SAMPLE_RATE(
                                ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                        controlPortRate[ctrl_out_index] = true;
                    }
                    for (l2 = 0; l2 < ladspaPoly; l2++)
                        ladspa_dsc->connect_port(ladspa_handle[l2], l1,
                                &control_out[ctrl_out_index]);

                    // fprintf(stderr, "output: %s\n", ladspa_dsc->PortNames[l1]);
                    ctrl_index[ctrl_out_index] = out_port_list.count() +
                        out_ctrl_port_list.count() - 1;
                    ctrl_out_index++;
                }
            } else {
                if (LADSPA_IS_PORT_OUTPUT(ladspa_dsc->PortDescriptors[l1])) {
                    for (l2 = 0; l2 < ladspaPoly; l2++)
                        ladspa_dsc->connect_port(ladspa_handle[l2], l1,
                                &control_out[ctrl_out_index]);
                    ctrl_out_index++;
                }
            }
            if (LADSPA_IS_PORT_INPUT(ladspa_dsc->PortDescriptors[l1])) {
                if (tabMode && ((ctrl_in_index % MAX_LADPSA_CONTROLS_PER_TAB) == 0)) {
                    if (ctrl_in_index + MAX_LADPSA_CONTROLS_PER_TAB < ladspa_ctrl_in_count) {
                        qs.sprintf("%d-%d", ctrl_in_index + 1,
                                ctrl_in_index + MAX_LADPSA_CONTROLS_PER_TAB);
                    } else {
                        qs.sprintf("%d-%d", ctrl_in_index + 1,
                                ladspa_ctrl_in_count);
                    }
                    ladspaTab = configDialog->addVBoxTab(qs);
                }
                if (LADSPA_IS_HINT_TOGGLED(
                            ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                    configDialog->addCheckBox(ladspa_dsc->PortNames[l1],
                            control_gui[ctrl_in_index], ladspaTab);
                    control_data[ctrl_in_index] = 0;

                    for (l2 = 0; l2 < ladspaPoly; l2++)
                        ladspa_dsc->connect_port(ladspa_handle[l2], l1,
                                &control_data[ctrl_in_index]);

                    ctrl_in_index++;
                } else {
                    control_min = 0;
                    control_max = 1;
                    if (LADSPA_IS_HINT_BOUNDED_BELOW(
                                ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                        control_min = ladspa_dsc->PortRangeHints[l1].LowerBound;
                    }
                    if (LADSPA_IS_HINT_BOUNDED_ABOVE(
                                ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                        control_max = ladspa_dsc->PortRangeHints[l1].UpperBound;
                    }
                    if (LADSPA_IS_HINT_SAMPLE_RATE(
                                ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                        // TODO implement this
                        rate_factor = synthdata->rate;
                        controlPortRate[ctrl_in_index] = true;
                    } else {
                        rate_factor = 1.0;
                    }
                    control_gui[ctrl_in_index] = control_min * rate_factor;
                    control_data[ctrl_in_index] = control_min * rate_factor;
                    control_data_min[ctrl_in_index] = control_min * rate_factor;
                    control_data_max[ctrl_in_index] = control_max * rate_factor;
                    if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(
                                ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                        control_gui[ctrl_in_index] = control_max * rate_factor;
                        control_data[ctrl_in_index] = control_max * rate_factor;
                    }
                    if (LADSPA_IS_HINT_LOGARITHMIC(
                                ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                        if (LADSPA_IS_HINT_DEFAULT_LOW(
                                    ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                            if (control_min <=0 ) control_min = 1e-4;
                            if (control_max <=0 ) control_max = 1e-4;
                            control_gui[ctrl_in_index] =
                                exp(log(control_min) * 0.75 +
                                        log(control_max) * 0.25) * rate_factor;
                            control_data[ctrl_in_index] =
                                exp(log(control_min) * 0.75 +
                                        log(control_max) * 0.25) * rate_factor;
                        }
                        if (LADSPA_IS_HINT_DEFAULT_MIDDLE(
                                    ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                            if (control_min <=0 ) control_min = 1e-4;
                            if (control_max <=0 ) control_max = 1e-4;
                            control_gui[ctrl_in_index] =
                                exp(log(control_min) * 0.5 +
                                        log(control_max) * 0.5) * rate_factor;
                            control_data[ctrl_in_index] =
                                exp(log(control_min) * 0.5 +
                                        log(control_max) * 0.5) * rate_factor;
                        }
                        if (LADSPA_IS_HINT_DEFAULT_HIGH(
                                    ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                            if (control_min <=0 ) control_min = 1e-4;
                            if (control_max <=0 ) control_max = 1e-4;
                            control_gui[ctrl_in_index] =
                                exp(log(control_min) * 0.25 +
                                        log(control_max) * 0.75) * rate_factor;
                            control_data[ctrl_in_index] =
                                exp(log(control_min) * 0.25 +
                                        log(control_max) * 0.75) * rate_factor;
                        }
                    } else {
                        if (LADSPA_IS_HINT_DEFAULT_LOW(
                                    ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                            control_gui[ctrl_in_index] =
                                (control_min * 0.75 +
                                 control_max * 0.25) * rate_factor;
                            control_data[ctrl_in_index] =
                                (control_min * 0.75 +
                                 control_max * 0.25) * rate_factor;
                        }
                        if (LADSPA_IS_HINT_DEFAULT_MIDDLE(
                                    ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                            control_gui[ctrl_in_index] =
                                (control_min * 0.5 +
                                 control_max * 0.5) * rate_factor;
                            control_data[ctrl_in_index] =
                                (control_min * 0.5 +
                                 control_max * 0.5) * rate_factor;
                        }
                        if (LADSPA_IS_HINT_DEFAULT_HIGH(
                                    ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                            control_gui[ctrl_in_index] =
                                (control_min * 0.25 +
                                 control_max * 0.75) * rate_factor;
                            control_data[ctrl_in_index] =
                                (control_min * 0.25 +
                                 control_max * 0.75) * rate_factor;
                        }
                    }
                    if (LADSPA_IS_HINT_DEFAULT_0(
                                ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                        control_gui[ctrl_in_index] = 0;
                        control_data[ctrl_in_index] = 0;
                    }
                    if (LADSPA_IS_HINT_DEFAULT_1(
                                ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                        control_gui[ctrl_in_index] = rate_factor;
                        control_data[ctrl_in_index] = rate_factor;
                    }
                    if (LADSPA_IS_HINT_DEFAULT_100(
                                ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                        control_gui[ctrl_in_index] = 100.0 * rate_factor;
                        control_data[ctrl_in_index] = 100.0 * rate_factor;
                    }
                    if (LADSPA_IS_HINT_DEFAULT_440(
                                ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                        control_gui[ctrl_in_index] = 440.0 * rate_factor;
                        control_data[ctrl_in_index] = 440.0 * rate_factor;
                    }
                    if (LADSPA_IS_HINT_LOGARITHMIC(
                                ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                        configDialog->addSlider(
                                ladspa_dsc->PortNames[l1],
                                control_gui[ctrl_in_index],
                                control_min * rate_factor,
                                control_max * rate_factor,
                                true, ladspaTab);
                    } else {
                        if (LADSPA_IS_HINT_INTEGER(
                                    ladspa_dsc->PortRangeHints[l1].HintDescriptor)) {
                            // fprintf(stderr, "LADSPA_IS_HINT_INTEGER %s\n", ladspa_dsc->PortNames[l1]);
                            configDialog->addFloatIntSlider(
                                    ladspa_dsc->PortNames[l1],
                                    control_gui[ctrl_in_index],
                                    control_min * rate_factor,
                                    control_max * rate_factor,
                                    ladspaTab);
                        } else {
                            configDialog->addSlider(
                                    ladspa_dsc->PortNames[l1],
                                    control_gui[ctrl_in_index],
                                    control_min * rate_factor,
                                    control_max * rate_factor,
                                    false, ladspaTab);
                        }
                    }
                    for (l2 = 0; l2 < ladspaPoly; l2++)
                        ladspa_dsc->connect_port(ladspa_handle[l2], l1,
                                &control_data[ctrl_in_index]);

                    ctrl_in_index++;
                }
            }
        }
    }
    itmp = ((out_port_list.count() + out_ctrl_port_list.count())
            > (in_port_list.count() + in_ctrl_port_list.count()))
        ? (out_port_list.count() + out_ctrl_port_list.count())
        : (in_port_list.count() + in_ctrl_port_list.count());

    setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_LADSPA_WIDTH,
            MODULE_LADSPA_HEIGHT + 20 * itmp);
    pluginName.sprintf("%s", ladspa_dsc->Label);
    //  fprintf(stderr, "--> isPoly: %d  ladspa_dsc->Label: %s  moduleID: %d\n", isPoly, ladspa_dsc->Label, moduleID);
}

M_ladspa::~M_ladspa()
{
  int l1, l2;

  for (l1 = 0; l1 < ladspaPoly; l1++) {
    for (l2 = 0; l2 < ladspa_audio_in_count; l2++)
      free(ladspaDataIn[l2][l1]);

    if (ladspa_dsc->deactivate)
      ladspa_dsc->deactivate(ladspa_handle[l1]);
  }

  for (l1 = 0; l1 < ladspaPoly; l1++)
	 ladspa_dsc->cleanup(ladspa_handle[l1]);

}

void M_ladspa::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  QRect r = logo.rect();

  r.moveCenter(rect().center());
  p.drawPixmap(r, logo);

  paint(p);
}

void M_ladspa::generateCycle()
{
  int l1, l3;
  unsigned int l2;
  float ctrlVal;

  for (l3 = 0; l3 < in_port_list.count(); l3++)
      inData [l3] = in_port_list.at(l3)->getinputdata();

  if (hasExtCtrlPorts)
    {
      for (l3 = 0; l3 < in_ctrl_port_list.count(); l3++)
          inData_ctrl [l3] = in_ctrl_port_list.at(l3)->getinputdata();
    }

  if (isPoly) {
    for (l3 = 0; l3 < in_port_list.count(); l3++) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
	for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
	  ladspaDataIn[l3][l1][l2] = inData[l3][l1][l2];
	}
      }
    }
  } else {
    for (l3 = 0; l3 < in_port_list.count(); l3++) {
      for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
	ladspaDataIn[l3][0][l2] = inData[l3][0][l2];
      }
      if (synthdata->poly > 1 && inData[l3][0] != inData[l3][1])
	for (l1 = 1; l1 < synthdata->poly; l1++) {
	  for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
	    ladspaDataIn[l3][0][l2] += inData[l3][l1][l2];
	  }
	}
    }
  }
  if (hasExtCtrlPorts) {
    for (l3 = 0; l3 < in_ctrl_port_list.count(); l3++) {
      if (controlPortRate[l3]) {
	ctrlVal = synthdata->rate * inData_ctrl[l3][0][0] + control_gui[l3];
      } else {
	ctrlVal = inData_ctrl[l3][0][0] + control_gui[l3];
      }
      if (ctrlVal < control_data_min[l3]) ctrlVal = control_data_min[l3];
      if (ctrlVal > control_data_max[l3]) ctrlVal = control_data_max[l3];
      control_data[l3] = ctrlVal;
    }
    for (l1 = 0; l1 < synthdata->poly; l1++) {
      for (l3 = 0; l3 < out_ctrl_port_list.count(); l3++) {
	for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
	  data[ctrl_index[l3]][l1][l2] = control_out[l3];
	}
      }
    }
  } else {
    for (l3 = 0; l3 < ladspa_ctrl_in_count; l3++) {
      control_data[l3] = control_gui[l3];
    }
  }

  for (l1 = 0; l1 < ladspaPoly; l1++)
    ladspa_dsc->run(ladspa_handle[l1], synthdata->cyclesize);
}

