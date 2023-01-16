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

#include "../daisy_saul.h"

namespace rinx {

using namespace daisy;
using namespace saul;


/** @brief Calibration data container for Margolis 
*/
struct CalibrationData
{
    CalibrationData() : warp_scale(60.f), warp_offset(0.f), cv_offset{0.f} {}
    float warp_scale, warp_offset;
    float cv_offset[CV_LAST];
    

    /** @brief checks sameness */
    bool operator==(const CalibrationData &rhs)
    {
        if(warp_scale != rhs.warp_scale)
        {
            return false;
        }
        else if(warp_offset != rhs.warp_offset)
        {
            return false;
        }
        else
        {
            for(int i = 0; i < CV_LAST; i++)
            {
                if(cv_offset[i] != rhs.cv_offset[i])
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
  polyphony(1), 
  model(0), 
  easter_egg(0),
  color_blind(0)  {}

  
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
  
  void Init(DaisySaul* hw);
  void SaveState();
  void LoadState();
  void RestoreState();
  //void Save();

   /** @brief Sets the cv offset from an externally array of data */
  inline void SetStateData(State statedata);

  /** @brief Sets the cv offset from an externally array of data */
  inline void GetStateData(State &statedata);
  
  /** @brief Sets the calibration data for 1V/Octave over Warp CV 
     *  typically set after reading stored data from external memory.
     */
    inline void SetWarpCalData(float scale, float offset)
    {
        voct_cal.SetData(scale, offset);
    }

    /** @brief Gets the current calibration data for 1V/Octave over Warp CV 
     *  typically used to prepare data for storing after successful calibration
     */
    inline void GetWarpCalData(float &scale, float &offset)
    {
        voct_cal.GetData(scale, offset);
    }

    /** @brief Sets the cv offset from an externally array of data */
    inline void SetCvOffsetData(float *data)
    {
        for(int i = 0; i < CV_LAST; i++)
        {
            cv_offsets_[i] = data[i];
        }
    }

    /** @brief Fills an array with the offset data currently being used */
    inline void GetCvOffsetData(float *data)
    {
        for(int i = 0; i < CV_LAST; i++)
        {
            data[i] = cv_offsets_[i];
        }
    }

  /** @brief Checks to see if calibration has been completed and needs to be saved */
  inline bool ReadyToSaveCal() const { return cal_save_flag_; }

  /** @brief signal the cal-save flag to clear once calibration data has been written to ext. flash memory */
  inline void ClearSaveCalFlag() { cal_save_flag_ = false; }
  
  inline State* mutable_state() {
    return &state_;
  }

  inline const State& state() const {
    return state_;
  }
  
  // True when no calibration data has been found on flash sector 1, that is
  // to say when the module has just been flashed.
  inline bool freshly_baked() const {
    return freshly_baked_;
  }
  
 private:
  bool freshly_baked_;
  State state_;
  uint16_t version_token_;
  DaisySaul* hw_;

  /** Cal data */
  float                  warp_v1_, warp_v3_;
  daisy::VoctCalibration voct_cal;
  float                  cv_offsets_[CV_LAST];
  

  bool cal_save_flag_;
  

};

}  // namespace rinx

#endif  // RINGS_SETTINGS_H_
