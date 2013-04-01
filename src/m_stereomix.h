#ifndef M_STEREOMIX_H
#define M_STEREOMIX_H

#include "module.h"
#include "midiguicomponent.h"

#define MODULE_STEREOMIX_WIDTH                115
#define MODULE_STEREOMIX_HEIGHT                40
#define MAX_STEREOMIX_IN                       16 
       
class M_stereomix : public Module
{
    Q_OBJECT

  class MCableMute;
  class MCableSolo;

    QList<Port*> in_port_list;
    Port *port_out[2];
    float gain;
    float mixer_gain[MAX_STEREOMIX_IN], pan[MAX_STEREOMIX_IN];
    float mute[MAX_STEREOMIX_IN], solo[MAX_STEREOMIX_IN];
    bool ignore_check;
  unsigned solo_index;

  void soloed(float &s);
  void updateSolos(MidiGUIcomponent *sender);

public: 
    float **inData[MAX_STEREOMIX_IN];
    unsigned in_channels;
                            
  public:
    M_stereomix(int p_in_channels, QWidget* parent=0);

    void generateCycle();

};
  
#endif
