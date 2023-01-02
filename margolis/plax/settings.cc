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

#include "settings.h"

#include <algorithm>


namespace plaits {

using namespace std;
using namespace daisy;

void Settings::Init(DaisyMargolis* hw) {
  hw_ = hw;
  ChannelCalibrationData* c = persistent_data_.channel_calibration_data;
  c[hw_->CV_1].offset = 0.025f;
  c[hw_->CV_1].scale = -1.03f;
  c[hw_->CV_1].normalization_detection_threshold = 0;
  
  c[hw_->CV_VOCT].offset = 25.71;
  c[hw_->CV_VOCT].scale = -60.0f;
  c[hw_->CV_VOCT].normalization_detection_threshold = 0;

  c[hw_->CV_3].offset = 0.0f;
  c[hw_->CV_3].scale = -60.0f;
  c[hw_->CV_3].normalization_detection_threshold = -2945;

  c[hw_->CV_5].offset = 0.0f;
  c[hw_->CV_5].scale = -1.0f;
  c[hw_->CV_5].normalization_detection_threshold = 0;

  c[hw_->CV_2].offset = 0.0f;
  c[hw_->CV_2].scale = -1.6f;
  c[hw_->CV_2].normalization_detection_threshold = -2945;

  c[hw_->CV_4].offset = 0.0f;
  c[hw_->CV_4].scale = -1.6f;
  c[hw_->CV_4].normalization_detection_threshold = -2945;

  //c[CV_ADC_CHANNEL_TRIGGER].offset = 0.4f;
  //c[CV_ADC_CHANNEL_TRIGGER].scale = -0.6f;
  //c[CV_ADC_CHANNEL_TRIGGER].normalization_detection_threshold = 13663;

  c[hw_->CV_6].offset = 0.49f;
  c[hw_->CV_6].scale = -0.6f;
  c[hw_->CV_6].normalization_detection_threshold = 21403;
  
  state_.engine = 0;
  state_.lpg_colour = 0;
  state_.decay = 128;
  state_.octave = 255;
  state_.color_blind = 0;
  
  //bool success = chunk_storage_.Init(&persistent_data_, &state_);
  LoadPersistentData(&persistent_data_);
  LoadState(&state_);
  
  CONSTRAIN(state_.engine, 0, 15);

  //return success;
}

/** @brief Loads and sets settings data */
void Settings::LoadPersistentData(PersistentData* persistent_data)
{
    daisy::PersistentStorage<PersistentData> persistent_storage(hw_->seed.qspi);
    PersistentData default_settings;
    persistent_storage.Init(default_settings, FLASH_BLOCK);
    // &persistent_data = persistent_storage.GetSettings();
    
    //SetSettingsData(settings_data.ledcount,settings_data.engine);
    //SetAttData(settings_data.ledatt);
    
}

/** @brief Loads and sets settings data */
void Settings::LoadState(State* state_data)
{
    daisy::PersistentStorage<State> state_storage(hw_->seed.qspi);
    State default_state;
    state_storage.Init(default_state, FLASH_BLOCK*2);
    //&state_data = state_storage.GetSettings();
    
    //SetStateSettingsData(settings_data.decay);
        
}

void Settings::SavePersistentData()
{
    daisy::PersistentStorage<PersistentData> persistent_storage(hw_->seed.qspi);
    PersistentData default_settings;
    persistent_storage.Init(default_settings, FLASH_BLOCK);
    //&persistent_data = persistent_storage.GetSettings();

    persistent_storage.Save();
    
}

void Settings::SaveState()
{
    daisy::PersistentStorage<State> state_storage(hw_->seed.qspi);
    State default_settings;
    state_storage.Init(default_settings, FLASH_BLOCK*2);
    //&state_data = state_storage.GetSettings();
   
    state_storage.Save();
    
}



}  // namespace plaits
