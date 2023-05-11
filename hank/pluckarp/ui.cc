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

//#define ENABLE_LFO_MODE

void Ui::Init(ArpSettings* arpsettings, Settings* settings, DaisyHank* hw) {
  hw_ = hw;
  settings_ = settings;
  arpsettings_ = arpsettings;

  ui_task_ = 0;
  mode_ = UI_MODE_NORMAL;
 
 
  cv_ctrl_ = VOCT;
  
  //settings_->RestoreState();
  LoadState();

  
  // Bind pots to parameters.
  /*
  pots_[DaisyHank::KNOB_1].Init(
      &transposition_, &octave_, 2.0f, -1.0f);
  pots_[DaisyHank::KNOB_2].Init(
      &patch->harmonics, &patch->lpg_colour, 1.0f, 0.0f);
  pots_[DaisyHank::KNOB_3].Init(
      &patch->timbre, &patch->decay, 1.0f, 0.0f);
  pots_[DaisyHank::KNOB_4].Init(
      &patch->morph, &cv_ctrl_, 1.0f, 0.0f);
  */
  
  pwm_counter_ = 0;
  press_time_ =  0;
  ignore_release_ = false;
  

  cv_c1_ = 0.0f;
  pitch_lp_ = 0.0f;
  pitch_lp_calibration_ = 0.0f;

  
}

void Ui::LoadState() {
  const State& state = settings_->state();
  //patch_->engine = state.engine;
  arpsettings_->scaleIdx = state.scale;
  arpsettings_->chordRepeats = state.repeats;
  arpsettings_->chordDirection = state.direction;
  //patch_->lpg_colour = static_cast<float>(state.lpg_colour) / 256.0f;
  //patch_->decay = static_cast<float>(state.decay) / 256.0f;
  //octave_ = static_cast<float>(state.octave) / 256.0f;
  //cv_ctrl_ = static_cast<float>(state.cv_ctrl) / 256.0f;
  for(int i = 0; i < 4; i++)
  {
    arpsettings_->slotChordIdx[i] = state.slotChord[i];
  }
         
}

  
  


void Ui::SaveState() {
  State* state = settings_->mutable_state();
  state->scale = arpsettings_->scaleIdx;
  state->direction = arpsettings_->chordDirection;
  state->repeats = arpsettings_->chordRepeats;
  for(int i = 0; i < 4; i++)
  {
    state->slotChord[i] = arpsettings_->slotChordIdx[i];
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

        
        //hw_->SetRGBColor(static_cast<size_t>(active_engine_ & 3),engine_color_);


      }
      break;
    
    case UI_MODE_DISPLAY_VCFA_VCA:
      {
          
         
        
      }
      break;

      case UI_MODE_DISPLAY_TIME_DECAY:
      {
        
        
      }
      break;

      case UI_MODE_DISPLAY_CV_CTRL:
      {
        
      }
      break;
    
    case UI_MODE_DISPLAY_OCTAVE:
      {

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
          //patch_->engine = (patch_->engine + 1) % size_engines_;
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


 
  
  ONE_POLE(pitch_lp_, hw_->GetWarpVoct(), 0.7f);
  ONE_POLE(pitch_lp_calibration_, hw_->cv[DaisyHank::CV_1].Value(), 0.1f);
  //modulations_->note = pitch_lp_;
  
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


void Ui::SetLedsOctRange(int idx)
{
    hw_->SetRGBColor(DaisyHank::RGB_LED_1,(idx == 1 || idx == 5 || idx == 6 || idx == 7 || idx == 8  ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_2,(idx == 2 || idx == 5 || idx == 8 ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_3,(idx == 3 || idx == 6 || idx == 8 ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_4,(idx == 4 || idx == 7 || idx == 8 ? DaisyHank::YELLOW : DaisyHank::OFF));
}

void Ui::SetLedsState(int idx)
{
    hw_->SetRGBColor(DaisyHank::RGB_LED_1,(idx == FM || idx == MODEL ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_2,(idx == HARM || idx == MODEL ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_3,(idx == TIMBRE ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_4,(idx == MORPH ? DaisyHank::YELLOW : DaisyHank::OFF));
}

}  // namespace plaits