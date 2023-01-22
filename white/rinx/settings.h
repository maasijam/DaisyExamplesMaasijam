// Copyright 2015 Emilie Gillet.
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

#ifndef RINGS_SETTINGS_H_
#define RINGS_SETTINGS_H_

#include "stmlib/stmlib.h"

//#include "rings/drivers/adc.h"
#include "../daisy_white.h"

namespace rings {

using namespace white;

struct CalibrationData {

  CalibrationData() : pitch_scale(60.f), pitch_offset(0.f), offset{0.f} {}
  float pitch_offset, pitch_scale;
  float offset[CV_LAST];

  /** @brief checks sameness */
    bool operator==(const CalibrationData &rhs)
    {
        if(pitch_scale != rhs.pitch_scale)
        {
            return false;
        }
        else if(pitch_offset != rhs.pitch_offset)
        {
            return false;
        }
        else
        {
            for(int i = 0; i < CV_LAST; i++)
            {
                if(offset[i] != rhs.offset[i])
                    return false;
            }
        }
        return true;
    }

    /** @brief Not equal operator */
    bool operator!=(const CalibrationData &rhs) { return !operator==(rhs); }

};

struct State {
  State() :
  polyphony(2),
  model(1),
  easter_egg(0),
  color_blind(0) {}
  
  uint8_t polyphony;
  uint8_t model;
  uint8_t easter_egg;
  uint8_t color_blind;

  /**@brief checks sameness */
    bool operator==(const State &rhs)
    {
        if(polyphony != rhs.polyphony)
        {
            return false;
        } else if(model != rhs.model)
        {
            return false;
        } else if(easter_egg != rhs.easter_egg)
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
  
  void Init(DaisyWhite* hw);
  void SaveState();
  void SaveCalibration();
  void RestoreState();

  void LoadState();
  void LoadCalibration();

    /** @brief Sets the cv offset from an externally array of data */
  inline void SetStateData(State statedata);

  /** @brief Sets the cv offset from an externally array of data */
  inline void GetStateData(State &statedata);

    /** @brief Sets the cv offset from an externally array of data */
  inline void SetCalibrationData(CalibrationData caldata);

  /** @brief Sets the cv offset from an externally array of data */
  inline void GetCalibrationData(CalibrationData &caldata);
  
  inline CalibrationData* mutable_calibration_data() {
    return &calibration_data_;
  }

  inline CalibrationData& calibration_data() {
    return calibration_data_;
  }
  
  inline void ToggleEasterEgg() {
    state_.easter_egg = !state_.easter_egg;
  }
  
  inline State* mutable_state() {
    return &state_;
  }

  inline const State& state() const {
    return state_;
  }
  

  
 private:

  CalibrationData calibration_data_;
  State state_;
  DaisyWhite* hw_;

};

}  // namespace rings

#endif  // RINGS_SETTINGS_H_
