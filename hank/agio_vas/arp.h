#ifndef ARP_H_
#define ARP_H_

#include <stdlib.h>
#include "scales_chords.h"

namespace arp {

const int arpSlots = 4;
const int maxRepeats = 8;

   
// direction - to be implemented later
enum arpdirection {
    UP,
    DOWN,
    UP_DOWN_IN,
    UP_DOWN_EX,
    RAND,
    LAST_DIR
};




enum chordname {
    I,
    IS2,
    IS4,
    I7,
    IV,
    IV7,
    V,
    V7,
    VI,
    VI7,
    II,
    III,
    LAST_CHORD
};

struct ArpSettings {
    float slotChordIdx[4];
    float scaleIdx;
    float chordRepeats;
    float chordDirection;
    int chord_slot_idx;
    int arp_idx;
    bool slotChange;
};






class Arp {
 public:
  Arp() { }
  ~Arp() { }
  
  ArpSettings arpsettings;
  
  
  void Init();
  float GetArpNote(float nn);
  void Trig();
  int curRepeats;
   
  private:
  int GetIdxByDirection(int idx);
  int irand(int min, int max) ;
  bool firstrun_;
  float* arpnote_;
  int curChordLength_;
  float curArpNote_;
  int intRepeats;

};


} // namespace arps

#endif  // ARP_H_