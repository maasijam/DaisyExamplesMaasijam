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

#include "cv_scaler.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/utils/random.h"

#include "dsp/part.h"
#include "dsp/patch.h"

#include "daisy.h"
#include "../daisy_white.h"

extern white::DaisyWhite hw;

namespace torus {
  
using namespace std;
using namespace stmlib;

/* static */
ChannelSettings CvScaler::channel_settings_[CHAN_LAST] = {
  { LAW_LINEAR, true, 1.00f },  // ADC_CHANNEL_CV_FREQUENCY
   { LAW_LINEAR, true, 0.1f },  // ADC_CHANNEL_CV_STRUCTURE
   { LAW_LINEAR, true, 0.1f },  // ADC_CHANNEL_CV_BRIGHTNESS
   { LAW_LINEAR, true, 0.05f },  // ADC_CHANNEL_CV_DAMPING
   { LAW_LINEAR, true, 0.01f },  // ADC_CHANNEL_CV_POSITION
   { LAW_LINEAR, false, 1.00f },  // ADC_CHANNEL_CV_V_OCT
  { LAW_LINEAR, false, 0.01f },  // ADC_CHANNEL_POT_FREQUENCY
  { LAW_LINEAR, false, 0.01f },  // ADC_CHANNEL_POT_STRUCTURE
  { LAW_LINEAR, false, 0.01f },  // ADC_CHANNEL_POT_BRIGHTNESS
  { LAW_LINEAR, false, 0.01f },  // ADC_CHANNEL_POT_DAMPING
  { LAW_LINEAR, false, 0.01f },  // ADC_CHANNEL_POT_POSITION
  { LAW_QUARTIC_BIPOLAR, false, 0.005f },  // ADC_CHANNEL_ATTENUVERTER_FREQUENCY
  { LAW_QUADRATIC_BIPOLAR, false, 0.005f },  //ADC_CHANNEL_ATTENUVERTER_STRUCTURE,
  { LAW_QUADRATIC_BIPOLAR, false, 0.005f },  // ADC_CHANNEL_ATTENUVERTER_BRIGHTNESS,
  { LAW_QUADRATIC_BIPOLAR, false, 0.005f },  // ADC_CHANNEL_ATTENUVERTER_DAMPING,
  { LAW_QUADRATIC_BIPOLAR, false, 0.005f },  // ADC_CHANNEL_ATTENUVERTER_POSITION,
};

void CvScaler::Init() {
  transpose_ = 0.0f;
  
  inhibit_strum_ = 0;
  fm_cv_ = 0.0f;

  
}

#define ATTENUVERT(destination, NAME, min, max) \
  { \
    float value = adc_lp_[ADC_CHANNEL_CV_ ## NAME]; \
    value *= adc_lp_[ADC_CHANNEL_ATTENUVERTER_ ## NAME]; \
    value += adc_lp_[ADC_CHANNEL_POT_ ## NAME]; \
    CONSTRAIN(value, min, max) \
    destination = value; \
  }

void CvScaler::Read(Patch* patch, PerformanceState* performance_state) {
  // Process all CVs / pots.
  int cv_map[6] = {0,1,2,3,4,5};
  int pot_map[10] = {0,3,1,2,5,9,10,8,11,6};
  float value;
  for (size_t i = 0; i < CHAN_LAST; i++) {
    const ChannelSettings& settings = channel_settings_[i];
    //float value = hw.controls[i].Value();
    if(i < 6) {
        value = hw.cv[cv_map[i]].Value();
        //value = hw.GetCvValue(cv_map[i]);
        
    } else {
        value = hw.knob[pot_map[i-6]].Value();
        //value = hw.GetKnobValue(pot_map[i-6]);
    }

    switch (settings.law) {
      case LAW_QUADRATIC_BIPOLAR:
        {
          value = value - 0.5f;
          float value2 = value * value * 4.0f * 3.3f;
          value = value < 0.0f ? -value2 : value2;
        }
        break;

      case LAW_QUARTIC_BIPOLAR:
        {
          value = value - 0.5f;
          float value2 = value * value * 4.0f;
          float value4 = value2 * value2 * 3.3f;
          value = value < 0.0f ? -value4 : value4;
        }
        break;

      default:
        break;
    }

    adc_lp_[i] += settings.lp_coefficient * (value - adc_lp_[i]);
  }
  
   ATTENUVERT(patch->structure, STRUCTURE, 0.0f, 0.9995f);
   ATTENUVERT(patch->brightness, BRIGHTNESS, 0.0f, 1.0f);
   ATTENUVERT(patch->damping, DAMPING, 0.0f, 1.0f);
   ATTENUVERT(patch->position, POSITION, 0.0f, 1.0f);

  //patch->structure = adc_lp_[POT_STRUCTURE];
  //patch->brightness = adc_lp_[POT_BRIGHTNESS];
  //patch->damping = adc_lp_[POT_DAMPING];
  //patch->position = adc_lp_[POT_POSITION];

  float fm = adc_lp_[ADC_CHANNEL_CV_FREQUENCY] * 48.0f;
  float error = fm - fm_cv_;
  if (fabs(error) >= 0.8f) {
    fm_cv_ = fm;
  } else {
    fm_cv_ += 0.02f * error;
  }
  performance_state->fm = fm_cv_ * adc_lp_[ADC_CHANNEL_ATTENUVERTER_FREQUENCY] * -1.0f;
  CONSTRAIN(performance_state->fm, -48.0f, 48.0f);
  
  float transpose = 60.0f * adc_lp_[ADC_CHANNEL_POT_FREQUENCY];
  float hysteresis = transpose - transpose_ > 0.0f ? -0.3f : +0.3f;
  transpose_ = static_cast<int32_t>(transpose + hysteresis + 0.5f);
  
  // float note = calibration_data_->pitch_offset;
  // note += adc_lp_[ADC_CHANNEL_CV_V_OCT] * calibration_data_->pitch_scale;

  
  //performance_state->note = adc_lp_[ADC_CHANNEL_CV_V_OCT] * 60.0f;
  performance_state->tonic = 12.0f + transpose_;
    
  // Strumming / internal exciter triggering logic.    

  if (performance_state->internal_note) {
    // Remove quantization when nothing is plugged in the V/OCT input.
    //performance_state->note = 0.0f;
    performance_state->tonic = 12.0f + transpose;
  }
  
  // Hysteresis on chord.
  float chord = 0.505f - \
      adc_lp_[ADC_CHANNEL_CV_STRUCTURE];
  chord *= adc_lp_[ADC_CHANNEL_ATTENUVERTER_STRUCTURE];
  chord += adc_lp_[ADC_CHANNEL_POT_STRUCTURE];
  chord *= static_cast<float>(kNumChords - 1);
  hysteresis = chord - chord_ > 0.0f ? -0.1f : +0.1f;
  chord_ = static_cast<int32_t>(chord + hysteresis + 0.5f);
  CONSTRAIN(chord_, 0, kNumChords - 1);
  performance_state->chord = chord_;
}

}  // namespace torus
