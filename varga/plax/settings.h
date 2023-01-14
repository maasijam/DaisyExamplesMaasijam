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

#include "../stmlib/stmlib.h"
#include "../daisy_varga.h"




namespace plaits {

using namespace daisy;
using namespace varga;


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
  engine(0), 
  lpg_colour(0), 
  decay(128),
  octave(255),
  color_blind(0)  {}

  
  uint8_t engine;
  uint8_t lpg_colour;
  uint8_t decay;
  uint8_t octave;
  uint8_t color_blind;


  /**@brief checks sameness */
    bool operator==(const State &rhs)
    {
        if(engine != rhs.engine)
        {
            return false;
        } else if(lpg_colour != rhs.lpg_colour)
        {
            return false;
        } else if(decay != rhs.decay)
        {
            return false;
        } else if(octave != rhs.octave)
        {
            return false;
        } else if(color_blind != rhs.color_blind)
        {
            return false;
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
  
  void Init(DaisyVarga* hw);

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
  DaisyVarga* hw_;
  

};

}  // namespace plaits

#endif  // PLAITS_SETTINGS_H_
