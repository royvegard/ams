#ifndef M_CVS_H
#define M_CVS_H

#include "module.h"


#define MODULE_CVS_HEIGHT               120
#define MODULE_CVS_CV_COUNT               4

class M_cvs : public Module
{
    Q_OBJECT

    Port *port_cv_out[MODULE_CVS_CV_COUNT];
    float cv[MODULE_CVS_CV_COUNT], cv_fine[MODULE_CVS_CV_COUNT];
                
  public:
    M_cvs(QWidget* parent=0);

    void generateCycle();
};
  
#endif
