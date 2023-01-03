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
#include "../daisy_margolis.h"



namespace plaits {

using namespace daisy;

#define FLASH_BLOCK 4096

  
struct ChannelCalibrationData {
  float offset;
  float scale;
  int16_t normalization_detection_threshold;
  inline float Transform(float x) const {
    return x * scale + offset;
  }
};

struct PersistentData {
  
  PersistentData() {
  
  channel_calibration_data[0].offset = 0.025f;
  channel_calibration_data[0].scale = -1.03f;
  channel_calibration_data[0].normalization_detection_threshold = 0;
  
  channel_calibration_data[6].offset = 25.71;
  channel_calibration_data[6].scale = -60.0f;
  channel_calibration_data[6].normalization_detection_threshold = 0;

  channel_calibration_data[2].offset = 0.0f;
  channel_calibration_data[2].scale = -60.0f;
  channel_calibration_data[2].normalization_detection_threshold = -2945;

  channel_calibration_data[4].offset = 0.0f;
  channel_calibration_data[4].scale = -1.0f;
  channel_calibration_data[4].normalization_detection_threshold = 0;

  channel_calibration_data[1].offset = 0.0f;
  channel_calibration_data[1].scale = -1.6f;
  channel_calibration_data[1].normalization_detection_threshold = -2945;

  channel_calibration_data[3].offset = 0.0f;
  channel_calibration_data[3].scale = -1.6f;
  channel_calibration_data[3].normalization_detection_threshold = -2945;

  channel_calibration_data[5].offset = 0.49f;
  channel_calibration_data[5].scale = -0.6f;
  channel_calibration_data[5].normalization_detection_threshold = 21403;
  }
  
  
  ChannelCalibrationData channel_calibration_data[DaisyMargolis::CV_LAST];
  uint8_t padding[16];
  enum { tag = 0x494C4143 };  // 
  
  /**@brief checks sameness */
    bool operator==(const PersistentData &rhs)
    {
        
        return true;
    }

    /** @brief Not equal operator */
    bool operator!=(const PersistentData &rhs) { return !operator==(rhs); }

};

struct State {

  /*() : 
  engine(3), 
  lpg_colour(0), 
  decay(128),
  octave(255),
  color_blind(0) {}*/

  State() { 
  engine = 7;
  lpg_colour = 0; 
  decay  = 128;
  octave = 255;
  color_blind = 0;
  };


  uint8_t engine;
  uint8_t lpg_colour;
  uint8_t decay;
  uint8_t octave;
  uint8_t color_blind;
  uint8_t padding[3];

  enum { tag = 0x54415453 };  // STAT

  /**@brief checks sameness */
    bool operator==(const State &rhs)
    {
        
        return true;
    }

    /** @brief Not equal operator */
    bool operator!=(const State &rhs) { return !operator==(rhs); }
};

class Settings {
 public:
  Settings() { }
  ~Settings() { }
  
  void Init(DaisyMargolis* hw);

  void SavePersistentData();
  void SaveState();
  void LoadPersistentData();
  void LoadState();

  /** @brief Sets the cv offset from an externally array of data */
  inline void SetPersistentData(PersistentData pdata);
  /** @brief Sets the cv offset from an externally array of data */
  //inline void SetAttData(bool *ledatts);
    /** @brief Sets the cv offset from an externally array of data */
  inline void SetStateData(State statedata);

  /** @brief Sets the cv offset from an externally array of data */
  inline void GetPersistentData(PersistentData &pdata);
  /** @brief Sets the cv offset from an externally array of data */
  //inline void GetAttData(bool *ledatts);
  /** @brief Sets the cv offset from an externally array of data */
  inline void GetStateData(State &statedata);

  
  inline const ChannelCalibrationData& calibration_data(int channel) const {
    return persistent_data_.channel_calibration_data[channel];
  }
  
  inline ChannelCalibrationData* mutable_calibration_data(int channel) {
    return &persistent_data_.channel_calibration_data[channel];
  }

  inline const State& state() const {
    return state_;
  }

  inline State* mutable_state() {
    return &state_;
  }
  
 private:
  PersistentData persistent_data_;
  State state_;

  DaisyMargolis* hw_;
  
 
  
  DISALLOW_COPY_AND_ASSIGN(Settings);
};

}  // namespace plaits

#endif  // PLAITS_SETTINGS_H_
