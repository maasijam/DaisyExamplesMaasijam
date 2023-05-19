#include "arp.h"

namespace arp {




void Arp::Init() {
    arpsettings.chord_slot_idx = 0;
    arpsettings.slotChange = false;
    arpsettings.arp_idx = 0;
    arpsettings.scaleIdx = 0;
    arpsettings.chordDirection = 0;
}

void Arp::Process(bool trig,float nn,float* arpnote) 
{
  //arpnote_ = arpnote;

    chordLength_ = scaleChords[static_cast<int>(arpsettings.scaleIdx*3)][static_cast<int>(arpsettings.slotChordIdx[arpsettings.chord_slot_idx]*12)].num_notes;
    *arpnote = ChordSelect(nn);

    if(trig) {
        if(arpsettings.slotChange) {
            arpsettings.chord_slot_idx = (arpsettings.chord_slot_idx + 1) % 4;
        }
        arpsettings.arp_idx = (arpsettings.arp_idx + 1) % chordLength_; // advance the kArpeggio, wrapping at the end.
    }
}

int Arp::GetIdxByDirection(int idx, int chordlen)
{
    switch (static_cast<int>(arpsettings.chordDirection*5)) {
    case DOWN:
    {
        if((chordlen-1) == idx) {arpsettings.slotChange = true;} else {arpsettings.slotChange = false;}
        return (chordlen-1)-idx;
    }
    case UP_DOWN_IN:
    {
        arpsettings.slotChange = false;
        if(firstrun)
        {
            if((chordlen-1) == idx) {
                firstrun = false;
            }
            return idx;
        } else {
            if((chordlen-1) == idx) {
                firstrun = true;
                arpsettings.slotChange = true;
            } 
            return (chordlen-1)-idx;
        }
        
    }
    case UP_DOWN_EX:
    {
        arpsettings.slotChange = false;
        if(firstrun)
        {
            if((chordlen-1) == idx) {
                firstrun = false;
            }
            return idx;
        } else {
            if((chordlen-2) == idx) {
                firstrun = true;
                arpsettings.slotChange = true;
            }
            return (chordlen-2)-idx;
        }
    }
    case RAND:
    {
        //return irand(0, (chordlen-1));
        if((chordlen-1) == idx) {arpsettings.slotChange = true;} else {arpsettings.slotChange = false;}
        return irand(0,chordlen);
    }
    default:
    {
        if((chordlen-1) == idx) {arpsettings.slotChange = true;} else {arpsettings.slotChange = false;}
        return idx;
    }
      
    } 
}

// select scale, and pass midi note as nn 
float Arp::ChordSelect(float nn){
	float freq;
    int cLen = scaleChords[static_cast<int>(arpsettings.scaleIdx*3)][static_cast<int>(arpsettings.slotChordIdx[arpsettings.chord_slot_idx]*12)].num_notes;
    int cIdx = GetIdxByDirection(arpsettings.arp_idx,cLen);
	freq = nn + scaleChords[static_cast<int>(arpsettings.scaleIdx*3)][static_cast<int>(arpsettings.slotChordIdx[arpsettings.chord_slot_idx]*12)].notes[cIdx];	
	return freq;

}

// Persistent filtered Value for smooth delay time changes.
//float smooth_time;

int Arp::irand(int min, int max) {
    return rand()%(max-min + 1) + min;
}
  
}  // namespace plaits
