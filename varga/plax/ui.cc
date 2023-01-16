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
// UI and CV processing ("controller" and "view")

#include "ui.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/system/system_clock.h"

namespace plaits {
  
using namespace std;
using namespace stmlib;
using namespace varga;

static const int32_t kLongPressTime = 2000;

#define ENABLE_LFO_MODE

void Ui::Init(Patch* patch, Modulations* modulations, Settings* settings, DaisyVarga* hw) {
  hw_ = hw;
  patch_ = patch;
  modulations_ = modulations;
  settings_ = settings;

  

  ui_task_ = 0;
  mode_ = UI_MODE_NORMAL;

       

  
  
  LoadState();

  
  /*if (hw_->s[S3].RawState()) {
    State* state = settings_->mutable_state();
    if (state->color_blind == 1) {
      state->color_blind = 0; 
    } else {
      state->color_blind = 1; 
    }
    settings_->SaveState();
  }*/
  
  // Bind pots to parameters.
  pots_[KNOB_1].Init(
      &transposition_, NULL, 2.0f, -1.0f);
  pots_[KNOB_2].Init(
      &patch->harmonics, &octave_, 1.0f, 0.0f);
  pots_[KNOB_3].Init(
      &patch->timbre, &patch->lpg_colour, 1.0f, 0.0f);
  pots_[KNOB_4].Init(
      &patch->morph, &patch->decay, 1.0f, 0.0f);
  
  
  // Keep track of the agreement between the random sequence sent to the 
  // switch and the value read by the ADC.
  
  
  pwm_counter_ = 0;
  fill(&press_time_[0], &press_time_[2], 0);
  fill(&ignore_release_[0], &ignore_release_[2], false);
  
  active_engine_ = 0;
  cv_c1_ = 0.0f;
  pitch_lp_ = 0.0f;
  pitch_lp_calibration_ = 0.0f;
  cblind_ = 0;
}

void Ui::LoadState() {
  const State& state = settings_->state();
  patch_->engine = state.engine;
  patch_->lpg_colour = static_cast<float>(state.lpg_colour) / 256.0f;
  patch_->decay = static_cast<float>(state.decay) / 256.0f;
  octave_ = static_cast<float>(state.octave) / 256.0f;
  
  
}


void Ui::SaveState() {
  State* state = settings_->mutable_state();
  state->engine = patch_->engine;
  state->lpg_colour = static_cast<uint8_t>(patch_->lpg_colour * 256.0f);
  state->decay = static_cast<uint8_t>(patch_->decay * 256.0f);
  state->octave = static_cast<uint8_t>(octave_ * 256.0f);
  
  settings_->SaveState();
}

void Ui::UpdateLEDs() {
  hw_->ClearLeds();
  ++pwm_counter_;
  
  int pwm_counter = pwm_counter_ & 15;
  int triangle = (pwm_counter_ >> 4) & 31;
  triangle = triangle < 16 ? triangle : 31 - triangle;

  switch (mode_) {
    case UI_MODE_NORMAL:
      {

        /*Colors red = settings_->state().color_blind == 1
            ? ((pwm_counter & 7) ? OFF : YELLOW)
            : RED;
        Colors green = settings_->state().color_blind == 1
            ? YELLOW
            : GREEN;
        hw_->SetRGBColor(static_cast<LeddriverLeds>(active_engine_ & 7),active_engine_ & 8 ? red : green);
        if (pwm_counter < triangle) {
          hw_->SetRGBColor(static_cast<LeddriverLeds>(patch_->engine & 7),patch_->engine & 8 ? red : green);
        }

        hw_->SetGreenLeds(LED_GREEN_1,modulations_->timbre_patched ? 1.f : 0.f);
        hw_->SetGreenLeds(LED_GREEN_2,modulations_->frequency_patched ? 1.f : 0.f);
        hw_->SetGreenLeds(LED_GREEN_3,modulations_->morph_patched ? 1.f : 0.f);*/
        hw_->SetRGBColor((patch_->engine & 8 ? LED_2 : LED_1),Colors (patch_->engine & 7));
        //hw_->SetRGBColor((active_engine_ & 8 ? LED_2 : LED_1),Colors (active_engine_ & 7));

      }
      break;
    
    case UI_MODE_DISPLAY_ALTERNATE_PARAMETERS:
      {
       
      }
      break;
    
    case UI_MODE_DISPLAY_OCTAVE:
      {
#ifdef ENABLE_LFO_MODE

#else
        int octave = static_cast<float>(octave_ * 9.0f);
        for (int i = 0; i < 8; ++i) {
         // hw_->SetRGBColor(static_cast<LeddriverLeds>(7 - i),octave == i || (octave == 8) ? YELLOW : OFF);
        }
#endif  // ENABLE_LFO_MODE
      }
      break;
      
    case UI_MODE_CALIBRATION_C1:
      if (pwm_counter < triangle) {
        hw_->SetRGBColor(LED_2,GREEN);
      }
      break;

    case UI_MODE_CALIBRATION_C3:
      if (pwm_counter < triangle) {
        hw_->SetRGBColor(LED_1,YELLOW);
      }
      break;
    
    case UI_MODE_ERROR:
      if (pwm_counter < triangle) {
        //for (int i = 0; i < kNumLEDs; ++i) {
          hw_->SetRGBColor(LED_2,RED);
          hw_->SetRGBColor(LED_1,RED);
        //}
      }
      break;

      case UI_MODE_TEST:
      {
        
      }
        break;

      case UI_MODE_RESTORE_STATE:
      {
        if (pwm_counter < triangle) {
          //for (int i = 0; i < kNumLEDs; ++i) {
            hw_->SetRGBColor(LED_2,PURPLE);
            hw_->SetRGBColor(LED_1,PURPLE);
          //}
        }
      }     
      break;

      
  }
  hw_->UpdateLeds();
}

void Ui::ReadSwitches() {

  hw_->ProcessDigitalControls();

  int s_pins[2] = {S_1,S_2};
  
  switch (mode_) {
    case UI_MODE_NORMAL:
      {
        for (int i = 0; i < 2; ++i) {
          if (hw_->s[s_pins[i]].RisingEdge()) {
            press_time_[i] = 0;
            ignore_release_[i] = false;
          }
          if (hw_->s[s_pins[i]].Pressed()) {
            ++press_time_[i];
          } else {
            press_time_[i] = 0;
          }
        }
        
        if (hw_->s[S_1].RisingEdge()) {
          pots_[KNOB_3].Lock();
          pots_[KNOB_4].Lock();
        }
        if (hw_->s[S_2].RisingEdge()) {
          pots_[KNOB_2].Lock();
        }
        
        if (pots_[KNOB_4].editing_hidden_parameter() ||
            pots_[KNOB_3].editing_hidden_parameter()) {
          mode_ = UI_MODE_DISPLAY_ALTERNATE_PARAMETERS;
        }
        
        if (pots_[KNOB_2].editing_hidden_parameter()) {
          mode_ = UI_MODE_DISPLAY_OCTAVE;
        }
        
        // Long, double press: enter calibration mode.
        if (press_time_[0] >= kLongPressTime &&
            press_time_[1] >= kLongPressTime) {
          press_time_[0] = press_time_[1] = 0;
          RealignPots();
          StartCalibration();
        }
        
        // Long press or actually editing any hidden parameter: display value
        // of hidden parameters.
        if (press_time_[0] >= kLongPressTime && !press_time_[1]) {
          press_time_[0] = press_time_[1] = 0;
          mode_ = UI_MODE_DISPLAY_ALTERNATE_PARAMETERS;
        }
        if (press_time_[1] >= kLongPressTime && !press_time_[0]) {
          press_time_[0] = press_time_[1] = 0;
          mode_ = UI_MODE_DISPLAY_OCTAVE;
        }
        
        if (hw_->s[S_1].FallingEdge() && !ignore_release_[0]) {
          RealignPots();
          if (patch_->engine >= 8) {
            patch_->engine = patch_->engine & 7;
          } else {
            patch_->engine = (patch_->engine + 1) % 8;
          }
          readyToSaveState = true;
        }

        if (hw_->s[S_2].FallingEdge() && !ignore_release_[1]) {
          RealignPots();
          if (patch_->engine < 8) {
            patch_->engine = (patch_->engine & 7) + 8;
          } else {
            patch_->engine = 8 + ((patch_->engine + 1) % 8);
          }
         readyToSaveState = true;
        }
        
      }
      break;
      
    case UI_MODE_DISPLAY_ALTERNATE_PARAMETERS:
    case UI_MODE_DISPLAY_OCTAVE:
      for (int i = 0; i < 2; ++i) {
        if (hw_->s[s_pins[i]].FallingEdge()) {
          pots_[KNOB_3].Unlock();
          pots_[KNOB_4].Unlock();
          pots_[KNOB_2].Unlock();
          press_time_[i] = 0;
          mode_ = UI_MODE_NORMAL;
        }
      }
      break;
    
    case UI_MODE_CALIBRATION_C1:
      for (int i = 0; i < 2; ++i) {
        if (hw_->s[s_pins[i]].RisingEdge()) {
          press_time_[i] = 0;
          ignore_release_[i] = true;
          CalibrateC1();
          break;
        }
      }
      break;
      
    case UI_MODE_CALIBRATION_C3:
      for (int i = 0; i < 2; ++i) {
        if (hw_->s[s_pins[i]].RisingEdge()) {
          press_time_[i] = 0;
          ignore_release_[i] = true;
          CalibrateC3();
          break;
        }
      }
      break;

    case UI_MODE_TEST:
    case UI_MODE_ERROR:
    case UI_MODE_RESTORE_STATE:
      for (int i = 0; i < 2; ++i) {
        if (hw_->s[s_pins[i]].RisingEdge()) {
          press_time_[i] = 0;
          ignore_release_[i] = true;
          mode_ = UI_MODE_NORMAL;
        }
      }
      break;
      

  }
}

void Ui::ProcessPotsHiddenParameters() {
  for (int i = 0; i < KNOB_LAST; ++i) {
    pots_[i].ProcessUIRate();
  }
}



void Ui::DetectNormalization() {

  

}

void Ui::Poll() {

  
  

  for (int i = 0; i < KNOB_LAST; ++i) {
    pots_[i].ProcessControlRate(hw_->knob[i].Value());
  }

  
  
  
  ONE_POLE(pitch_lp_, hw_->GetWarpVoct(), 0.7f);
  ONE_POLE(pitch_lp_calibration_, hw_->cv[CV_VOCT].Value(), 0.1f);
  modulations_->note = pitch_lp_;
  
  ui_task_ = (ui_task_ + 1) % 4;
  switch (ui_task_) {
    case 0:
      UpdateLEDs();
      break;
    
    case 1:
      ReadSwitches();
      break;
    
    case 2:
      ProcessPotsHiddenParameters();
      break;
      
    case 3:
      DetectNormalization();
      break;
  }
  

#ifdef ENABLE_LFO_MODE
  int octave = static_cast<int>(octave_ * 10.0f);
  if (octave == 0) {
    patch_->note = -48.37f + transposition_ * 60.0f;
  } else if (octave == 9) {
    patch_->note = 60.0f + transposition_ * 48.0f;
  } else {
    const float fine = transposition_ * 7.0f;
    patch_->note = fine + static_cast<float>(octave) * 12.0f;
  }
#else
  int octave = static_cast<int>(octave_ * 9.0f);
  if (octave < 8) {
    const float fine = transposition_ * 7.0f;
    patch_->note = fine + static_cast<float>(octave) * 12.0f + 12.0f;
  } else {
    patch_->note = 60.0f + transposition_ * 48.0f;
  }
#endif  // ENABLE_LFO_MODE
}

void Ui::StartCalibration() {
  mode_ = UI_MODE_CALIBRATION_C1;
}

void Ui::CalibrateC1() {
  // Acquire offsets for all channels.
  float co[CV_LAST];
  for (int i = 0; i < CV_LAST; ++i) {
    if (i != CV_VOCT) {
      co[i] = hw_->cv[i].Value();
    } else {
      co[i] = 0.f;
    }
  }
  auto &current_offset_data = co;
  hw_->SetCvOffsetData(current_offset_data);

  hw_->CalibrateV1(pitch_lp_calibration_);

  mode_ = UI_MODE_CALIBRATION_C3;
}

void Ui::CalibrateC3() {
  hw_->CalibrateV3(pitch_lp_calibration_);
    if(hw_->ReadyToSaveCal()) {
       mode_ = UI_MODE_NORMAL;
    } else {
      mode_ = UI_MODE_ERROR;
    }
}



/** @brief Loads and sets calibration data */
void Ui::SaveCalibrationData()
{
    daisy::PersistentStorage<CalibrationData> cal_storage(hw_->seed.qspi);
    CalibrationData                           default_cal;
    cal_storage.Init(default_cal, FLASH_BLOCK);
    auto &cal_data = cal_storage.GetSettings();
    hw_->GetWarpCalData(cal_data.warp_scale, cal_data.warp_offset);
    hw_->GetCvOffsetData(cal_data.cv_offset);
    cal_storage.Save();
    hw_->ClearSaveCalFlag();
}

}  // namespace plaits
