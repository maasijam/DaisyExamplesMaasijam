#include "arp.h"

namespace arp {

void Arp::Init() {
    arpsettings.chord_slot_idx = 0;
    arpsettings.slotChange = false;
    arpsettings.arp_idx = 0;
    arpsettings.scaleIdx = 0;
    arpsettings.chordDirection = 0;
    firstrun_ = true;
    curRepeats = 0;
    intRepeats = 0;
}

void Arp::Trig() 
{

    if(arpsettings.slotChange) {
        arpsettings.chord_slot_idx = (arpsettings.chord_slot_idx + 1) % arpSlots;
    } 
    curChordLength_ = scaleChords[static_cast<int>(arpsettings.scaleIdx*LAST_SCALE)][static_cast<int>(arpsettings.slotChordIdx[arpsettings.chord_slot_idx]*numChords)].num_notes;
    arpsettings.arp_idx = (arpsettings.arp_idx + 1) % curChordLength_; // advance the kArpeggio, wrapping at the end.
    int arpDirIdx = GetIdxByDirection(arpsettings.arp_idx);
    curArpNote_ = scaleChords[static_cast<int>(arpsettings.scaleIdx*LAST_SCALE)][static_cast<int>(arpsettings.slotChordIdx[arpsettings.chord_slot_idx]*numChords)].notes[arpDirIdx];
   
    
}

int Arp::GetIdxByDirection(int idx)
{
    arpsettings.slotChange = false;
    intRepeats = static_cast<int>(arpsettings.chordRepeats*maxRepeats);
    if (curRepeats > intRepeats)
    {
        curRepeats = 0;
    }
    
    
    switch (static_cast<int>(arpsettings.chordDirection*LAST_DIR)) {
    case DOWN:
    {
        if((curChordLength_-1) == idx) 
        {
            if(curRepeats == intRepeats)
            {
                arpsettings.slotChange = true;
                curRepeats = 0;
            } else {
                curRepeats++;
            }
        }
        return (curChordLength_-1)-idx;
    }
    case UP_DOWN_IN:
    {
        
        if(firstrun_)
        {
            if((curChordLength_-1) == idx) {
                firstrun_ = false;
            }
            return idx;
        } else {
            if((curChordLength_-1) == idx) {
                firstrun_ = true;
                if(curRepeats == intRepeats)
                {
                    arpsettings.slotChange = true;
                    curRepeats = 0;
                } else {
                    curRepeats++;
                }
            } 
            return (curChordLength_-1)-idx;
        }
        
    }
    case UP_DOWN_EX:
    {
       
        if(firstrun_)
        {
            if((curChordLength_-1) == idx) {
                firstrun_ = false;
            }
            return idx;
        } else {
            if((curChordLength_-2) == idx) {
                firstrun_ = true;
                if(curRepeats == intRepeats)
                {
                    arpsettings.slotChange = true;
                    curRepeats = 0;
                } else {
                    curRepeats++;
                }
            }
            return (curChordLength_-2)-idx;
        }
    }
    case RAND:
    {
        if((curChordLength_-1) == idx) 
        {
            if(curRepeats == intRepeats)
            {
                arpsettings.slotChange = true;
                curRepeats = 0;
            } else {
                curRepeats++;
            }
        }
        return irand(0,curChordLength_);
    }
    default:
    {
        if((curChordLength_-1) == idx) 
        {
            if(curRepeats == intRepeats)
            {
                arpsettings.slotChange = true;
                curRepeats = 0;
            } else {
                curRepeats++;
            }
        }
        return idx;
    }
      
    } 
    
}


// select scale, and pass midi note as nn 
float Arp::GetArpNote(float nn){
	float freq;
	freq = nn + curArpNote_;	
	return freq;
}

int Arp::irand(int min, int max) {
    return rand()%(max-min + 1) + min;
}
  
}  // namespace plaits
