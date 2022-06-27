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

#ifndef TORUS_CV_SCALER_H_
#define TORUS_CV_SCALER_H_

#include "stmlib/stmlib.h"

namespace torus
{
enum Law
{
    LAW_LINEAR,
    LAW_QUADRATIC_BIPOLAR,
    LAW_QUARTIC_BIPOLAR
};

struct ChannelSettings
{
    Law   law;
    bool  remove_offset;
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

class CvScaler
{
  public:
    CvScaler() {}
    ~CvScaler() {}

    void Init();
    void Read(Patch* patch, PerformanceState* performance_state);

    inline bool easter_egg() const
    {
        return false;
        // return adc_lp_[ADC_CHANNEL_POT_FREQUENCY] < 0.1f &&
        //     adc_lp_[ADC_CHANNEL_POT_STRUCTURE] > 0.9f &&
        //     adc_lp_[ADC_CHANNEL_POT_BRIGHTNESS] < 0.1f &&
        //     adc_lp_[ADC_CHANNEL_POT_POSITION] > 0.9f &&
        //     adc_lp_[ADC_CHANNEL_POT_DAMPING] > 0.4f &&
        //     adc_lp_[ADC_CHANNEL_POT_DAMPING] < 0.6f &&
        //     adc_lp_[ADC_CHANNEL_ATTENUVERTER_BRIGHTNESS] < -1.00f &&
        //     adc_lp_[ADC_CHANNEL_ATTENUVERTER_FREQUENCY] > 1.00f &&
        //     adc_lp_[ADC_CHANNEL_ATTENUVERTER_DAMPING] < -1.00f &&
        //     adc_lp_[ADC_CHANNEL_ATTENUVERTER_STRUCTURE] > 1.00f &&
        //     adc_lp_[ADC_CHANNEL_ATTENUVERTER_POSITION] < -1.00f;
    }

   

  private:
    static ChannelSettings channel_settings_[CHAN_LAST];
    float                  adc_lp_[CHAN_LAST];

    int32_t inhibit_strum_;

    float transpose_;
    float fm_cv_;
    float cv_c1_;
    float cv_low_;

    int32_t chord_;
};

} // namespace torus

#endif // TORUS_CV_SCALER_H_
