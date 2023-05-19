#ifndef ARP_H_
#define ARP_H_

#include <stdlib.h>

namespace arp {




    
// direction - to be implemented later
enum arpdirection {
    UP,
    DOWN,
    UP_DOWN_IN,
    UP_DOWN_EX,
    RAND,
    LAST_DIR
};

enum scale {
  MAJOR,
  MINOR,
  DORIAN,
  LAST_SCALE
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


struct Chord {
  int num_notes;
  int notes[12];  
};


const Chord scaleChords[3][12] = {
  // MAJOR
  {
  // I
  { 3, {0,4,7}},
  // IS2
  { 3, {0,2,7}},
  // IS4
  { 3, {0,5,7}},
  // I7
  { 4, {0,4,7,11}},
  // IV
  { 3, {0,5,9}},
  // IV7
  { 4, {0,4,5,9}},
  // V
  { 3, {2,7,11}},
  // V7
  { 4, {2,5,7,11}},
  // VI
  { 3, {0,4,9}},
  // VI7
  { 4, {0,4,7,9}},
  // II
  { 3, {2,5,9}},
  // III
  { 3, {4,7,11}}
},
//MINOR
{
  // I
  { 3, {0,3,7}},
  // IS2
  { 3, {0,2,7}},
  // IS4
  { 3, {0,5,7}},
  // I7
  { 4, {0,3,7,10}},
  // IV
  { 3, {0,5,8}},
  // IV7
  { 4, {0,3,5,8}},
  // V
  { 3, {2,7,10}},
  // V7
  { 4, {2,5,7,10}},
  // VI
  { 3, {0,3,8}},
  // VI7
  { 4, {0,3,7,8}},
  // II
  { 3, {2,5,8}},
  // III
  { 3, {3,7,10}}
},
//DORIAN
{
  // I
  { 3, {0,3,7}},
  // IS2
  { 3, {0,2,7}},
  // IS4
  { 3, {0,5,7}},
  // I7
  { 4, {0,3,7,10}},
  // IV
  { 3, {0,5,9}},
  // IV7
  { 4, {0,3,5,9}},
  // V
  { 3, {2,7,10}},
  // V7
  { 4, {2,5,7,10}},
  // VI
  { 3, {0,3,9}},
  // VI7
  { 4, {0,3,7,9}},
  // II
  { 3, {2,5,9}},
  // III
  { 3, {3,7,10}}
}
};

class Arp {
 public:
  Arp() { }
  ~Arp() { }
  
  ArpSettings arpsettings;
  
  
  void Init();
  void Process(bool trig,float nn, float *arpnote);
   
  private:
  int GetIdxByDirection(int idx, int chordlen);
  float ChordSelect(float nn);
  int irand(int min, int max) ;
  bool firstrun;
  float* arpnote_;
  int chordLength_;

};


} // namespace arps

#endif  // ARP_H_