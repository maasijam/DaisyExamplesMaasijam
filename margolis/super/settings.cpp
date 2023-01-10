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


namespace super {

using namespace std;
using namespace daisy;

void Settings::Init(DaisyMargolis* hw) {
  hw_ = hw;
 
  //state_.engine = 0;
  //state_.lpg_colour = 0;
  //state_.decay = 128;
  //state_.octave = 255;
  //state_.color_blind = 0;
  
  //bool success = chunk_storage_.Init(&persistent_data_, &state_);
  //RestoreSettings();
  //LoadPersistentData();
  LoadState();
  
    
  //CONSTRAIN(state_.engine, 0, 7);

  //return success;
}




 /** @brief Sets the cv offset from an externally array of data */
inline void Settings::SetStateData(State state_data)
{
    state_ = state_data;
      
}



/** @brief Sets the cv offset from an externally array of data */
inline void Settings::GetStateData(State &state_data)
{
    state_data = state_;
       
}



/** @brief Loads and sets settings data */
void Settings::LoadState()
{
    daisy::PersistentStorage<State> state_storage(hw_->seed.qspi);
    State default_state;
    state_storage.Init(default_state, FLASH_BLOCK*2);
    State &state_data = state_storage.GetSettings();
    SetStateData(state_data);
        
}



void Settings::SaveState()
{
    daisy::PersistentStorage<State> state_storage(hw_->seed.qspi);
    State default_settings;
    state_storage.Init(default_settings, FLASH_BLOCK*2);
    State &state_data = state_storage.GetSettings();
    
    GetStateData(state_data);
   
    state_storage.Save();
    
}

void Settings::RestoreState()
{
    daisy::PersistentStorage<State> settings_storage(hw_->seed.qspi);
    State default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK*2);
    settings_storage.RestoreDefaults();
    State &settings_data = settings_storage.GetSettings();
    GetStateData(settings_data);
    
}

}  // namespace plaits
