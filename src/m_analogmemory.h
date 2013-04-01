/*
  Analog Memory - derived from m_vcdelay.cpp

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

#ifndef M_ANALOGMEMORY_H
#define M_ANALOGMEMORY_H

#include "module.h"


#define MODULE_ANALOGMEMORY_HEIGHT                120
#define MAX_ANALOGMEMORY_2N_FRAMES                20

class M_analogmemory : public Module
{
    Q_OBJECT 

    int addrmode;
    int cell2n;
    float writethresh;
    int offset;
    Port *port_M_in_data, *port_M_in_addr, *port_M_in_WrEna,
      *port_M_out_addr, *port_outcv;
    
  public: 
    float **inData, **inAddr, **inWrEna, **outAddr, **buf;
    int *lastwrite;
                            
  public:
    M_analogmemory(QWidget* parent=0);
    ~M_analogmemory();

    void generateCycle();
};
  
#endif
