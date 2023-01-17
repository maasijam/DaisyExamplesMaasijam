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
// Filtering and scaling of ADC values + input calibration.

#ifndef RINGS_CV_SCALER_H_
#define RINGS_CV_SCALER_H_

#include "stmlib/stmlib.h"

#include "../daisy_saul.h"

#include "settings.h"

namespace rinx {

enum Law {
  LAW_LINEAR,
  LAW_QUADRATIC_BIPOLAR,
  LAW_QUARTIC_BIPOLAR
};

struct ChannelSettings {
  Law law;
  bool remove_offset;
  float lp_coefficient;
};

enum Channels
{
    ADC_CHANNEL_CV_FREQUENCY,
    ADC_CHANNEL_CV_STRUCTURE,
    ADC_CHANNEL_CV_BRIGHTNESS,
    ADC_CHANNEL_CV_DAMPING,
    ADC_CHANNEL_CV_POSITION,
    ADC_CHANNEL_CV_V_OCT,
    ADC_CHANNEL_POT_FREQUENCY,
    ADC_CHANNEL_POT_STRUCTURE,
    ADC_CHANNEL_POT_BRIGHTNESS,
    ADC_CHANNEL_POT_DAMPING,
    ADC_CHANNEL_POT_POSITION,
    ADC_CHANNEL_ATTENUVERTER_FREQUENCY,
    ADC_CHANNEL_ATTENUVERTER_STRUCTURE,
    ADC_CHANNEL_ATTENUVERTER_BRIGHTNESS,
    ADC_CHANNEL_ATTENUVERTER_DAMPING,
    ADC_CHANNEL_ATTENUVERTER_POSITION,
    CHAN_LAST
};

struct Patch;
struct PerformanceState;

class CvScaler {
 public:
  CvScaler() { }
  ~CvScaler() { }
  
  void Init();
  void Read(Patch* patch, PerformanceState* performance_state);

  
  inline bool ready_for_calibration() const {
    return true;
  }
  
  inline bool easter_egg() const {
   /* return adc_lp_[ADC_CHANNEL_POT_FREQUENCY] < 0.1f &&
        adc_lp_[ADC_CHANNEL_POT_STRUCTURE] > 0.9f &&
        adc_lp_[ADC_CHANNEL_POT_BRIGHTNESS] < 0.1f &&
        adc_lp_[ADC_CHANNEL_POT_POSITION] > 0.9f &&
        adc_lp_[ADC_CHANNEL_POT_DAMPING] > 0.4f &&
        adc_lp_[ADC_CHANNEL_POT_DAMPING] < 0.6f &&
        adc_lp_[ADC_CHANNEL_ATTENUVERTER_BRIGHTNESS] < -1.00f &&
        adc_lp_[ADC_CHANNEL_ATTENUVERTER_FREQUENCY] > 1.00f &&
        adc_lp_[ADC_CHANNEL_ATTENUVERTER_DAMPING] < -1.00f &&
        adc_lp_[ADC_CHANNEL_ATTENUVERTER_STRUCTURE] > 1.00f &&
        adc_lp_[ADC_CHANNEL_ATTENUVERTER_POSITION] < -1.00f;*/
        return false;
  }
  
  inline void CalibrateV1(float v1) {
    //cv_c1_ = adc_.float_value(ADC_CHANNEL_CV_V_OCT);
    warp_v1_ = v1;
  }

  /*inline void CalibrateOffsets() {
    for (size_t i = 0; i < ADC_CHANNEL_NUM_OFFSETS; ++i) {
      calibration_data_->offset[i] = adc_.float_value(i);
    }
  }*/
  
  inline void CalibrateV3(float v3) {
     warp_v3_ = v3;
        voct_cal.Record(warp_v1_, warp_v3_);
        cal_save_flag_ = true;
  }
  
  /*inline void StartNormalizationCalibration() {
    normalization_probe_enabled_ = false;
    normalization_probe_forced_state_ = false;
  }
  
  inline void CalibrateLow() {
    cv_low_ = adc_.float_value(ADC_CHANNEL_CV_V_OCT);
    normalization_probe_forced_state_ = true;
  }
  
  inline bool CalibrateHigh() {
    float threshold = (cv_low_ + adc_.float_value(ADC_CHANNEL_CV_V_OCT)) * 0.5f;
    bool within_range = threshold >= 0.7f && threshold < 0.8f;
    if (within_range) {
      calibration_data_->normalization_detection_threshold = threshold;
    }
    normalization_probe_enabled_ = true;
    return within_range;
  }*/
  
  /*inline uint8_t adc_value(size_t index) const {
    return adc_.value(index) >> 8;
  }
  
  inline bool gate_value() const {
    return trigger_input_.value();
  }*/

  /*inline uint8_t normalization(size_t index) const {
    switch (index) {
      case 0:
        return fm_cv_ * 3.3f > 0.8f ? 255 : 0;
        break;
        
      case 1:
        return normalization_detector_trigger_.normalized() ? 255 : 0;
        break;

      case 2:
        return normalization_detector_v_oct_.normalized() ? 255 : 0;
        break;
      
      default:
        return normalization_detector_exciter_.normalized() ? 255 : 0;
        break;
    }
  }*/

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

  
 private:
  
  //Adc adc_;
  CalibrationData* calibration_data_;
  //TriggerInput trigger_input_;
  

  int32_t inhibit_strum_;
  
  float adc_lp_[CV_LAST];
  float transpose_;
  float fm_cv_;
  float cv_c1_;
  float cv_low_;
  int32_t chord_;
  
  
  static ChannelSettings channel_settings_[CHAN_LAST];
  
  /** Cal data */
    float                  warp_v1_, warp_v3_;
    daisy::VoctCalibration voct_cal;
    float                  cv_offsets_[CV_LAST];
    

    bool cal_save_flag_;
};

}  // namespace rinx

#endif  // RINGS_CV_SCALER_H_
