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

void Ui::Init(Patch* patch, Modulations* modulations, Voice* voice, Arp* arp, Settings* settings, DaisyHank* hw) {
  hw_ = hw;
  patch_ = patch;
  modulations_ = modulations;
  settings_ = settings;
  voice_ = voice;
  arp_ = arp;

  ui_task_ = 0;
  
  mode_ = UI_MODE_NORMAL;
  engineCounter_ = 0;
  
 

  //size_engines_ = voice_->GetNumEngines();     
  cv_ctrl_ = VOCT;
  
  //settings_->RestoreState();
  LoadState();

  
  // Engines 0,2,8,9
  PotsInit(engineCounter_);  
  
  
  pwm_counter_ = 0;
  press_time_ =  0;
  ignore_release_ = false;
  
  active_engine_ = 0;
  cv_c1_ = 0.0f;
  pitch_lp_ = 0.0f;
  pitch_lp_calibration_ = 0.0f;
  cblind_ = 0;
  octave_ = 1.0f;

  
}

void Ui::LoadState() {
  const State& state = settings_->state();
  //patch_->engine = state.engine;
  arp_->arpsettings.scaleIdx = state.scale;
  arp_->arpsettings.chordRepeats = state.repeats;
  arp_->arpsettings.chordDirection = state.direction;
  //patch_->lpg_colour = static_cast<float>(state.lpg_colour) / 256.0f;
  //patch_->decay = static_cast<float>(state.decay) / 256.0f;
  //octave_ = static_cast<float>(state.octave) / 256.0f;
  //cv_ctrl_ = static_cast<float>(state.cv_ctrl) / 256.0f;
  for(int i = 0; i < arpSlots; i++)
  {
    arp_->arpsettings.slotChordIdx[i] = state.slotChord[i];
  }
         
}


void Ui::SaveState() {
  State* state = settings_->mutable_state();
  state->scale = arp_->arpsettings.scaleIdx;
  state->direction = arp_->arpsettings.chordDirection;
  state->repeats = arp_->arpsettings.chordRepeats;
  for(int i = 0; i < arpSlots; i++)
  {
    state->slotChord[i] = arp_->arpsettings.slotChordIdx[i];
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
        
      switch (arp_->arpsettings.chord_slot_idx)
        {
        case 0:
          SetChordColor(static_cast<int>(arp_->arpsettings.slotChordIdx[0] * numChords),DaisyHank::RGB_LED_1);
          break;
        case 1:
          SetChordColor(static_cast<int>(arp_->arpsettings.slotChordIdx[1] * numChords),DaisyHank::RGB_LED_2);
          break;
        case 2:
          SetChordColor(static_cast<int>(arp_->arpsettings.slotChordIdx[2] * numChords),DaisyHank::RGB_LED_3);
          break;
        case 3:
          SetChordColor(static_cast<int>(arp_->arpsettings.slotChordIdx[3] * numChords),DaisyHank::RGB_LED_4);
          break;
        
        default:
          
          break;
        }
       

      }
      break;
    
    case UI_MODE_SLOT_CHORD:
      {
          int chord1 = static_cast<int>(arp_->arpsettings.slotChordIdx[0] * numChords);
          SetChordColor(chord1,DaisyHank::RGB_LED_1);
          int chord2 = static_cast<int>(arp_->arpsettings.slotChordIdx[1] * numChords);
          SetChordColor(chord2,DaisyHank::RGB_LED_2);
          int chord3 = static_cast<int>(arp_->arpsettings.slotChordIdx[2] * numChords);
          SetChordColor(chord3,DaisyHank::RGB_LED_3);
          int chord4 = static_cast<int>(arp_->arpsettings.slotChordIdx[3] * numChords);
          SetChordColor(chord4,DaisyHank::RGB_LED_4);
          
        
      }
      break;

      case UI_MODE_CONFIG:
      {
        int scaleidx = static_cast<int>(arp_->arpsettings.scaleIdx * LAST_SCALE);
        SetConfigColor(scaleidx, DaisyHank::RGB_LED_1);
        int directionidx = static_cast<int>(arp_->arpsettings.chordDirection * LAST_DIR);
        SetConfigColor(directionidx, DaisyHank::RGB_LED_2);
        int repeatsidx = static_cast<int>(arp_->arpsettings.chordRepeats * maxRepeats);
        SetConfigColor(repeatsidx, DaisyHank::RGB_LED_3);
        
      
        
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

      case UI_MODE_ENGINE:
      
        hw_->SetRGBColor(DaisyHank::RGB_LED_1,patch_->engine == 0 ? DaisyHank::BLUE : DaisyHank::OFF);
        hw_->SetRGBColor(DaisyHank::RGB_LED_2,patch_->engine == 2 ? DaisyHank::BLUE : DaisyHank::OFF);
        hw_->SetRGBColor(DaisyHank::RGB_LED_3,patch_->engine == 8 ? DaisyHank::BLUE : DaisyHank::OFF);
        hw_->SetRGBColor(DaisyHank::RGB_LED_4,patch_->engine == 9 ? DaisyHank::BLUE : DaisyHank::OFF);

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
          pots_[DaisyHank::KNOB_1].Lock(false);
          pots_[DaisyHank::KNOB_2].Lock(false);
          pots_[DaisyHank::KNOB_3].Lock(false);
          pots_[DaisyHank::KNOB_4].Lock(false);
        }
        
        if (pots_[DaisyHank::KNOB_1].editing_hidden_parameter1() ||
          pots_[DaisyHank::KNOB_2].editing_hidden_parameter1() ||
          pots_[DaisyHank::KNOB_3].editing_hidden_parameter1() ||
          pots_[DaisyHank::KNOB_4].editing_hidden_parameter1()) {
          mode_ = UI_MODE_SLOT_CHORD; 
        }

        if(hw_->s1.DoubleClick()) {
          pots_[DaisyHank::KNOB_3].Unlock2();
          pots_[DaisyHank::KNOB_4].Unlock2();
          pots_[DaisyHank::KNOB_2].Unlock2();
          pots_[DaisyHank::KNOB_1].Unlock2();
          pots_[DaisyHank::KNOB_1].Lock(true);
          pots_[DaisyHank::KNOB_2].Lock(true);
          pots_[DaisyHank::KNOB_3].Lock(true);
          pots_[DaisyHank::KNOB_4].Lock(true);
          mode_ = UI_MODE_CONFIG;
        }

        
                
        if (hw_->s1.FallingEdge() && !ignore_release_) {
          RealignPots();
          //patch_->engine = (patch_->engine + 1) % size_engines_;
          readyToSaveState = true;
        }

        if ((hw_->s1.TimeHeldMs() >= kLongCalPressTime) && PotsCalCheck()) {
          press_time_  = 0;
          RealignPots();
          StartCalibration();
        }

        if ((hw_->s1.TimeHeldMs() >= kLongCalPressTime) && !PotsCalCheck()) {
          mode_ = UI_MODE_ENGINE;
        }
        
      }
      break;
      
    case UI_MODE_SLOT_CHORD:
    
        
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

    case UI_MODE_CONFIG:
        
        if (hw_->s1.RisingEdge()) {
          pots_[DaisyHank::KNOB_3].Unlock();
          pots_[DaisyHank::KNOB_4].Unlock();
          pots_[DaisyHank::KNOB_2].Unlock();
          pots_[DaisyHank::KNOB_1].Unlock();
          press_time_ = 0;
          ignore_release_ = true;
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

      case UI_MODE_ENGINE:
        if (hw_->s1.FallingEdge()) {
          engineCounter_ = (engineCounter_ + 1) % 4;
          patch_->engine = enginesArr[engineCounter_];
        }
        if (hw_->s1.TimeHeldMs() >= kLongPressTime) {
          press_time_ = 0;
          ignore_release_ = true;
          mode_ = UI_MODE_NORMAL;
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
    patch_->note = 24.0f + transposition_ * 48.0f;
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


void Ui::SetLedsOctRange(int idx)
{
    hw_->SetRGBColor(DaisyHank::RGB_LED_1,(idx == 1 || idx == 5 || idx == 6 || idx == 7 || idx == 8  ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_2,(idx == 2 || idx == 5 || idx == 8 ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_3,(idx == 3 || idx == 6 || idx == 8 ? DaisyHank::YELLOW : DaisyHank::OFF));
    hw_->SetRGBColor(DaisyHank::RGB_LED_4,(idx == 4 || idx == 7 || idx == 8 ? DaisyHank::YELLOW : DaisyHank::OFF));
}

void Ui::SetChordColor(int idx, DaisyHank::Rgbs rgb_idx)
{
    
    switch (idx)
    {
    case I:
      hw_->SetRGBColor(rgb_idx,DaisyHank::BLUE);
      break;
    case IS2:
      hw_->SetRGBColor(rgb_idx,DaisyHank::TURQ);
      break;
    case IS4:
      hw_->SetRGBColor(rgb_idx,DaisyHank::CYAN);
      break;
    case I7:
      hw_->SetRGBColor(rgb_idx,DaisyHank::DARKGREEN);
      break;
    case IV:
      hw_->SetRGBColor(rgb_idx,DaisyHank::RED);
      break;
    case IV7:
      hw_->SetRGBColor(rgb_idx,DaisyHank::DARKRED);
      break;
    case V:
      hw_->SetRGBColor(rgb_idx,DaisyHank::BLUE);
      break;
    case V7:
      hw_->SetRGBColor(rgb_idx,DaisyHank::DARKBLUE);
      break;
    case VI:
      hw_->SetRGBColor(rgb_idx,DaisyHank::ORANGE);
      break;
    case VI7:
      hw_->SetRGBColor(rgb_idx,DaisyHank::DARKORANGE);
      break;
    case II:
      hw_->SetRGBColor(rgb_idx,DaisyHank::YELLOW);
      break;
    case III:
      hw_->SetRGBColor(rgb_idx,DaisyHank::PURPLE);
      break;
    
    default:
      break;
    }
}

void Ui::SetConfigColor(int idx, DaisyHank::Rgbs rgb_idx)
{
    
    switch (idx)
    {
    case 0:
      hw_->SetRGBColor(rgb_idx,DaisyHank::BLUE);
      break;
    case 1:
      hw_->SetRGBColor(rgb_idx,DaisyHank::RED);
      break;
    case 2:
      hw_->SetRGBColor(rgb_idx,DaisyHank::GREEN);
      break;
    case 3:
      hw_->SetRGBColor(rgb_idx,DaisyHank::YELLOW);
      break;
    case 4:
      hw_->SetRGBColor(rgb_idx,DaisyHank::PURPLE);
      break;
    case 5:
      hw_->SetRGBColor(rgb_idx,DaisyHank::CYAN);
      break;
    case 6:
      hw_->SetRGBColor(rgb_idx,DaisyHank::ORANGE);
      break;
    case 7:
      hw_->SetRGBColor(rgb_idx,DaisyHank::WHITE);
      break;
    
    default:
      break;
    }
}

void Ui::PotsInit(int engine){

  pots_[DaisyHank::KNOB_1].Init(
      &transposition_, &arp_->arpsettings.slotChordIdx[0], &arp_->arpsettings.scaleIdx ,1.0f, 0.0f);
  pots_[DaisyHank::KNOB_2].Init(
      &patch_->harmonics, &arp_->arpsettings.slotChordIdx[1], &arp_->arpsettings.chordDirection ,1.0f, 0.0f);
  if(engine == 0 || engine == 2)
  {
    pots_[DaisyHank::KNOB_3].Init(
        &patch_->decay, &arp_->arpsettings.slotChordIdx[2], &arp_->arpsettings.chordRepeats ,1.0f, 0.0f);
  } else if(engine == 8 || engine == 9) 
  {
    pots_[DaisyHank::KNOB_3].Init(
        &patch_->morph, &arp_->arpsettings.slotChordIdx[2], &arp_->arpsettings.chordRepeats ,1.0f, 0.0f);
  }
  pots_[DaisyHank::KNOB_4].Init(
      &patch_->timbre, &arp_->arpsettings.slotChordIdx[3], NULL, 1.0f, 0.0f);

  patch_->engine = engine;
  patch_->lpg_colour = static_cast<float>(0.5f) / 256.0f;
  
  if(engine == 2) 
  {
    patch_->morph = 0.75f;
  } else{
    patch_->morph = 0.5f;
  }

}

bool Ui::PotsCalCheck(){
    if(transposition_ < 0.01f && patch_->harmonics < 0.01f && patch_->timbre < 0.01f && (patch_->decay < 0.01 || patch_->morph < 0.01f)) {
      return true;
    } 
    return false;
}

}  // namespace plaits