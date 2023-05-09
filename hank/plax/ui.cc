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


static const int32_t kLongPressTime = 2000;
static const int32_t kLongCalPressTime = 10000;

#define ENABLE_LFO_MODE

void Ui::Init(Patch* patch, Modulations* modulations, Voice* voice, Settings* settings, DaisyHank* hw) {
  hw_ = hw;
  patch_ = patch;
  modulations_ = modulations;
  settings_ = settings;
  voice_ = voice;
  

  ui_task_ = 0;
  mode_ = UI_MODE_NORMAL;
 

  size_engines_ = voice_->GetNumEngines();     
  cv_ctrl_ = VOCT;
  
  //settings_->RestoreState();
  LoadState();

  
  // Bind pots to parameters.
  pots_[DaisyHank::KNOB_1].Init(
      &transposition_, &octave_, 2.0f, -1.0f);
  pots_[DaisyHank::KNOB_2].Init(
      &patch->harmonics, &patch->lpg_colour, 1.0f, 0.0f);
  pots_[DaisyHank::KNOB_3].Init(
      &patch->timbre, &patch->decay, 1.0f, 0.0f);
  pots_[DaisyHank::KNOB_4].Init(
      &patch->morph, &cv_ctrl_, 1.0f, 0.0f);
  
  
  pwm_counter_ = 0;
  press_time_ =  0;
  ignore_release_ = false;
  
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
  cv_ctrl_ = static_cast<float>(state.cv_ctrl) / 256.0f;
  for(int i = 0; i < PATCHED_LAST; i++)
        {
            switch (i)
            {
            case TIMBRE_PATCHED:
              modulations_->timbre_patched = state.is_patched[TIMBRE_PATCHED];
              break;
            case FM_PATCHED:
              modulations_->frequency_patched = state.is_patched[FM_PATCHED];
              break;
            case MORPH_PATCHED:
              modulations_->morph_patched = state.is_patched[MORPH_PATCHED];
              break;
            case TRIG_PATCHED:
              modulations_->trigger_patched = state.is_patched[TRIG_PATCHED];
              break;
            case LEVEL_PATCHED:
              modulations_->level_patched = state.is_patched[LEVEL_PATCHED];
              break;
            }
        }
  
}


void Ui::SaveState() {
  State* state = settings_->mutable_state();
  state->engine = patch_->engine;
  state->lpg_colour = static_cast<uint8_t>(patch_->lpg_colour * 256.0f);
  state->decay = static_cast<uint8_t>(patch_->decay * 256.0f);
  state->octave = static_cast<uint8_t>(octave_ * 256.0f);
  state->cv_ctrl = static_cast<uint8_t>(cv_ctrl_ * 256.0f);
  for(int i = 0; i < PATCHED_LAST; i++)
        {
            switch (i)
            {
            case TIMBRE_PATCHED:
               state->is_patched[TIMBRE_PATCHED] = modulations_->timbre_patched;
              break;
            case FM_PATCHED:
               state->is_patched[FM_PATCHED] = modulations_->frequency_patched;
              break;
            case MORPH_PATCHED:
              state->is_patched[MORPH_PATCHED] = modulations_->morph_patched;
              break;
            case TRIG_PATCHED:
              state->is_patched[TRIG_PATCHED] = modulations_->trigger_patched;
              break;
            case LEVEL_PATCHED:
              state->is_patched[LEVEL_PATCHED] = modulations_->level_patched;
              break;
            }
        }
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

        DaisyHank::Colors engine_color_;
        if(active_engine_ < 4) 
        {
            engine_color_ = DaisyHank::RED;
        }
        else if(active_engine_ > 3 && active_engine_ < 8) 
        {
            engine_color_ = DaisyHank::GREEN;
        }
        else if(active_engine_ > 7 && active_engine_ < 12) 
        {
            engine_color_ = DaisyHank::BLUE;
        }
        else {
            engine_color_ = DaisyHank::PURPLE;
        }
        hw_->SetRGBColor(static_cast<size_t>(active_engine_ & 3),engine_color_);


      }
      break;
    
    case UI_MODE_DISPLAY_VCFA_VCA:
      {
          
          float value = patch_->lpg_colour;
          value -= 0.001f;
          for (int i = 0; i < 4; ++i) {
            hw_->SetRGBColor(
                static_cast<size_t>(i),
                value * 64.0f > pwm_counter ? DaisyHank::YELLOW : DaisyHank::OFF);
            value -= 0.25f;
          }
        
      }
      break;

      case UI_MODE_DISPLAY_TIME_DECAY:
      {
        
        float value = patch_->decay;
          value -= 0.001f;
          for (int i = 0; i < 4; ++i) {
            hw_->SetRGBColor(
                static_cast<size_t>(i),
                value * 64.0f > pwm_counter ? DaisyHank::YELLOW : DaisyHank::OFF);
            value -= 0.25f;
          }
      }
      break;

      case UI_MODE_DISPLAY_CV_CTRL:
      {
        float value = cv_ctrl_;
        value -= 0.001f;
        if(value >= 0.165f && value < 0.33f ) {
            SetLedsState(FM);
        } else if(value >= 0.33f && value < 0.495f) {
            SetLedsState(HARM);
        } else if(value >= 0.495f && value < 0.66f) {
            SetLedsState(TIMBRE);
        } else if(value >= 0.66f && value < 0.825f) {
            SetLedsState(MORPH);
        } else if(value >= 0.825f && value <= 1.f) {
            SetLedsState(MODEL);
        } else {
            SetLedsState(VOCT);
        }
      }
      break;
    
    case UI_MODE_DISPLAY_OCTAVE:
      {
#ifdef ENABLE_LFO_MODE
        int octave = static_cast<float>(octave_ * 10.0f);
        for (int i = 0; i < 4; ++i) {
          DaisyHank::Colors color = DaisyHank::OFF;
          if (octave == 0) {
            color = i == (triangle >> 1) ? DaisyHank::OFF : DaisyHank::YELLOW;
          } else if (octave == 9) {
            color = DaisyHank::YELLOW;
          } else {
            color = (octave - 1) == i ? DaisyHank::YELLOW : DaisyHank::OFF;
          }
          hw_->SetRGBColor(3 - i,color);
        }
#else
        int octave = static_cast<float>(octave_ * 9.0f);
        for (int i = 0; i < 8; ++i) {
          hw_->SetRGBColor(static_cast<LeddriverLeds>(7 - i),octave == i || (octave == 8) ? YELLOW : OFF);
        }
#endif  // ENABLE_LFO_MODE
      }
      break;
      
    case UI_MODE_CALIBRATION_C1:
      if (pwm_counter < triangle) {
        hw_->SetRGBColor(DaisyHank::RGB_LED_1,DaisyHank::ORANGE);
        hw_->SetRGBColor(DaisyHank::RGB_LED_2,DaisyHank::ORANGE);
        hw_->SetRGBColor(DaisyHank::RGB_LED_3,DaisyHank::ORANGE);
        hw_->SetRGBColor(DaisyHank::RGB_LED_4,DaisyHank::ORANGE);
      }
      break;

    case UI_MODE_CALIBRATION_C3:
      if (pwm_counter < triangle) {
        hw_->SetRGBColor(DaisyHank::RGB_LED_1,DaisyHank::TURQ);
        hw_->SetRGBColor(DaisyHank::RGB_LED_2,DaisyHank::TURQ);
        hw_->SetRGBColor(DaisyHank::RGB_LED_3,DaisyHank::TURQ);
        hw_->SetRGBColor(DaisyHank::RGB_LED_4,DaisyHank::TURQ);
      }
      break;
    
    case UI_MODE_ERROR:
      if (pwm_counter < triangle) {
        for (int i = 0; i < kNumLEDs; ++i) {
          hw_->SetRGBColor(i,DaisyHank::RED);
        }
      }
      break;
  }
  hw_->UpdateLeds();
}

void Ui::ReadSwitches() {

  hw_->ProcessDigitalControls();

  switch (mode_) {
    case UI_MODE_NORMAL:
      {
          if (hw_->s1.RisingEdge()) {
            press_time_ = 0;
            ignore_release_ = false;
          }
          if (hw_->s1.Pressed()) {
            ++press_time_;
          } else {
            press_time_ = 0;
          }
        
        if (hw_->s1.RisingEdge()) {
          pots_[DaisyHank::KNOB_1].Lock();
          pots_[DaisyHank::KNOB_2].Lock();
          pots_[DaisyHank::KNOB_3].Lock();
          pots_[DaisyHank::KNOB_4].Lock();
        }
        
        if (pots_[DaisyHank::KNOB_1].editing_hidden_parameter()) {
          mode_ = UI_MODE_DISPLAY_OCTAVE; 
        }

        if (pots_[DaisyHank::KNOB_2].editing_hidden_parameter()) {
          mode_ = UI_MODE_DISPLAY_VCFA_VCA;
        }
        
        
        if (pots_[DaisyHank::KNOB_3].editing_hidden_parameter()) {
          mode_ = UI_MODE_DISPLAY_TIME_DECAY;
        }
        
        if (pots_[DaisyHank::KNOB_4].editing_hidden_parameter()) {
          mode_ = UI_MODE_DISPLAY_CV_CTRL;
        }
                
        if (hw_->s1.FallingEdge() && !ignore_release_) {
          RealignPots();
          patch_->engine = (patch_->engine + 1) % size_engines_;
          readyToSaveState = true;
        }

        if (hw_->s1.TimeHeldMs() >= kLongCalPressTime) {
          press_time_  = 0;
          RealignPots();
          StartCalibration();
        }
        
      }
      break;
      
    case UI_MODE_DISPLAY_OCTAVE:
    case UI_MODE_DISPLAY_VCFA_VCA:
    case UI_MODE_DISPLAY_TIME_DECAY:
    case UI_MODE_DISPLAY_CV_CTRL:
      
        if (hw_->s1.FallingEdge()) {
          pots_[DaisyHank::KNOB_3].Unlock();
          pots_[DaisyHank::KNOB_4].Unlock();
          pots_[DaisyHank::KNOB_2].Unlock();
          pots_[DaisyHank::KNOB_1].Unlock();
          press_time_ = 0;
          readyToSaveState = true;
          mode_ = UI_MODE_NORMAL;
        }
      
      break;
    
    case UI_MODE_CALIBRATION_C1:
     
        if (hw_->s1.RisingEdge()) {
          press_time_ = 0;
          ignore_release_ = true;
          CalibrateC1();
          break;
        }
      
      break;
      
    case UI_MODE_CALIBRATION_C3:
      
        if (hw_->s1.RisingEdge()) {
          press_time_ = 0;
          ignore_release_ = true;
          CalibrateC3();
          break;
        }
      
      break;

    case UI_MODE_ERROR:
        if (hw_->s1.RisingEdge()) {
          press_time_ = 0;
          ignore_release_ = true;
          mode_ = UI_MODE_NORMAL;
        }
      break;
  }
}

void Ui::ProcessPotsHiddenParameters() {
  for (int i = 0; i < DaisyHank::KNOB_LAST; ++i) {
    pots_[i].ProcessUIRate();
  }
}


void Ui::DetectNormalization() {

}

void Ui::Poll() {

  for (int i = 0; i < DaisyHank::KNOB_LAST; ++i) {
    pots_[i].ProcessControlRate(hw_->knob[i].Value());
  }

  if(cv_ctrl_ >= 0.165f && cv_ctrl_ < 0.33f ) {
      cv_ctrl_state_ = FM;
  } else if(cv_ctrl_ >= 0.33f && cv_ctrl_ < 0.495f) {
      cv_ctrl_state_ = HARM;
  } else if(cv_ctrl_ >= 0.495f && cv_ctrl_ < 0.66f) {
      cv_ctrl_state_ = TIMBRE;
  } else if(cv_ctrl_ >= 0.66f && cv_ctrl_ < 0.825f) {
      cv_ctrl_state_ = MORPH;
  } else if(cv_ctrl_ >= 0.825f && cv_ctrl_ <= 1.f) {
      cv_ctrl_state_ = MODEL;
  } else {
      cv_ctrl_state_ = VOCT;
  }


  modulations_->frequency = cv_ctrl_state_ == FM ? hw_->cv[DaisyHank::CV_1].Value() * plaits_cv_scale[FM] : 0;
	modulations_->harmonics = cv_ctrl_state_ == HARM ? hw_->cv[DaisyHank::CV_1].Value() * plaits_cv_scale[HARM] : 0;
	modulations_->timbre = cv_ctrl_state_ == TIMBRE ? hw_->cv[DaisyHank::CV_1].Value() * plaits_cv_scale[TIMBRE] : 0;
	modulations_->morph = cv_ctrl_state_ == MORPH ? hw_->cv[DaisyHank::CV_1].Value() * plaits_cv_scale[MORPH] : 0;
	modulations_->engine = cv_ctrl_state_ == MODEL ? hw_->cv[DaisyHank::CV_1].Value() * plaits_cv_scale[MODEL] : 0;

  
  ONE_POLE(pitch_lp_, hw_->GetWarpVoct(), 0.7f);
  ONE_POLE(pitch_lp_calibration_, hw_->cv[DaisyHank::CV_1].Value(), 0.1f);
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
  float co[DaisyHank::CV_LAST];
  for (int i = 0; i < DaisyHank::CV_LAST; ++i) {
    if (i != DaisyHank::CV_1) {
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
    cal_storage.Save();
    hw_->ClearSaveCalFlag();
}


void Ui::SetLedsState(int idx)
{
    hw_->SetRGBColor(DaisyHank::RGB_LED_1,(idx == FM || idx == MODEL ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_2,(idx == HARM || idx == MODEL ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_3,(idx == TIMBRE ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_4,(idx == MORPH ? DaisyHank::YELLOW : DaisyHank::OFF));
}

}  // namespace plaits