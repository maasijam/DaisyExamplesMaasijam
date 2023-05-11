// Copyright 2016 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Settings storage.

#ifndef PLAITS_SETTINGS_H_
#define PLAITS_SETTINGS_H_

#include "stmlib/stmlib.h"
#include "../daisy_hank.h"

namespace plaits {

using namespace daisy;

#define FLASH_BLOCK 4096

enum PlaitsPatched {
  TIMBRE_PATCHED,
  FM_PATCHED,
  MORPH_PATCHED,
  TRIG_PATCHED,
  LEVEL_PATCHED,
  PATCHED_LAST
};

struct State {

  State() : 
  scale(0), 
  slotChord{0,0,0,0}, 
  repeats(0),
  direction(0)  {}

  
  uint8_t scale;
  uint8_t slotChord[4];
  uint8_t repeats;
  uint8_t direction;
  


  /**@brief checks sameness */
    bool operator==(const State &rhs)
    {
        if(scale != rhs.scale)
        {
            return false;
        } else if(repeats != rhs.repeats)
        {
            return false;
        } else if(direction != rhs.direction)
        {
            return false;
        } else
        {
            for(int i = 0; i < 4; i++)
            {
                if(slotChord[i] != rhs.slotChord[i])
                    return false;
            }
        }
        return true;
    }

    /** @brief Not equal operator */
    bool operator!=(const State &rhs) { return !operator==(rhs); }
};

class Settings {
 public:
  Settings() { }
  ~Settings() { }
  
  void Init(DaisyHank* hw);

  void SaveState();
  void LoadState();
  void RestoreState();

    /** @brief Sets the cv offset from an externally array of data */
  inline void SetStateData(State statedata);

  /** @brief Sets the cv offset from an externally array of data */
  inline void GetStateData(State &statedata);


  inline const State& state() const {
    return state_;
  }

  inline State* mutable_state() {
    return &state_;
  }
  
 private:
  State state_;
  DaisyHank* hw_;
  

};

}  // namespace plaits

#endif  // PLAITS_SETTINGS_H_
