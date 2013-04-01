/* 
  Voltage-controlled delay - derived from m_delay.cpp

  Copyright (C) 2011 Bill Yerazunis <yerazunis@yahoo.com>

  This file is part of ams.

  ams is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2 as
  published by the Free Software Foundation.

  ams is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ams.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef M_VCDELAY_H
#define M_VCDELAY_H

#include "module.h"


#define MODULE_VCDELAY_HEIGHT                75
#define MAX_VCDELAY_FRAMES                32768

class M_vcdelay : public Module
{
    Q_OBJECT 

    float delay;
    float vmod;
    int read_ofs;
    Port *port_M_in, *port_M_delay, *port_out;
    
  public: 
    float **inData, **delayData, **buf;       
                            
  public:
    M_vcdelay(QWidget* parent=0);
    ~M_vcdelay();

    void generateCycle();
};
  
#endif
