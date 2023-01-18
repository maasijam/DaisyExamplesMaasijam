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
// User interface.

#include "ui.h"
#include "constants.h"
#include <algorithm>

#include "stmlib/system/system_clock.h"

#include "cv_scaler.h"
#include "dsp/part.h"
#include "dsp/string_synth_part.h"

namespace rinx {

const int32_t kLongPressDuration = 3000;

using namespace std;
using namespace stmlib;
using namespace saul;

void Ui::Init(
    Settings* settings,
    CvScaler* cv_scaler,
    Part* part,
    StringSynthPart* string_synth, DaisySaul* hw) {
 
    hw_ = hw;
    settings_ = settings;
    cv_scaler_ = cv_scaler;
    part_ = part;
    string_synth_ = string_synth;
    readyToSaveState = false;
    ui_task_ = 0;
    mode_ = UI_MODE_NORMAL;

    LoadState();
  
  if (hw_->s[S_3].RawState()) {
    State* state = settings_->mutable_state();
    if (state->color_blind == 1) {
      state->color_blind = 0; 
    } else {
      state->color_blind = 1; 
    }
    settings_->SaveState();
  }
  
  
}

void Ui::LoadState() {
  const State& state = settings_->state();
  part_->set_polyphony(state.polyphony);
  part_->set_model(static_cast<ResonatorModel>(state.model));
  string_synth_->set_polyphony(state.polyphony);
  string_synth_->set_fx(static_cast<FxType>(state.model));
  mode_ = state.easter_egg
      ? UI_MODE_EASTER_EGG_INTRO
      : UI_MODE_NORMAL;
  eggFxState_ = state.eggFxState;
  noteStrumState_ = state.noteStrumState;
  exciterState_ = state.exciterState;
}


void Ui::SaveState() {
  
  State* state = settings_->mutable_state();
  state->polyphony = part_->polyphony();
  state->model = part_->model();
  settings_->SaveState();
}

void Ui::AnimateEasterEggLeds() {
  mode_ = settings_->state().easter_egg
      ? UI_MODE_EASTER_EGG_INTRO
      : UI_MODE_EASTER_EGG_OUTRO;
}

void Ui::UpdateLEDs() {
  switch (mode_) {
    case UI_MODE_NORMAL:
      {

        switch (noteStrumState_)
          {
          case 1:
              hw_->SetLed(LED_NOTE,false);
              hw_->SetLed(LED_STRUM,true);
              break;
          case 2:
              hw_->SetLed(LED_NOTE,true);
              hw_->SetLed(LED_STRUM,false);
              break;
          case 3:
              hw_->SetLed(LED_NOTE,false);
              hw_->SetLed(LED_STRUM,false);
              break;
          default:
              hw_->SetLed(LED_NOTE,true);
              hw_->SetLed(LED_STRUM,true);
              break;
          }

          hw_->SetLed(LED_EXCITER,!exciterState_);

          switch (part_->polyphony())
          {
          case 1:
              hw_->SetLed(LED_POLY2,false);
              hw_->SetLed(LED_POLY4,true);
              break;
          case 2:
              hw_->SetLed(LED_POLY2,true);
              hw_->SetLed(LED_POLY4,false);
              break;
          case 3:
              hw_->SetLed(LED_POLY2,false);
              hw_->SetLed(LED_POLY4,false);
              break;
          default:
              hw_->SetLed(LED_POLY2,true);
              hw_->SetLed(LED_POLY4,true);
              break;
          }

          hw_->SetRGBLed(1,static_cast<u_int8_t>(eggFxState_));
          hw_->SetRGBLed(4,static_cast<u_int8_t>(part_->model()));

          if(readyToSaveState) {
                  //hw_->SetRGBLed(1,RED);
                  //hw_->SetRGBLed(2,RED);
                  //hw_->SetRGBLed(3,RED);
                  //hw_->SetRGBLed(4,RED);
          } 
      }
      break;
    
    case UI_MODE_CALIBRATION_C1:
      //leds_.set(0, blink, blink);
      //leds_.set(1, false, false);
      break;

    case UI_MODE_CALIBRATION_C3:
      //leds_.set(0, false, false);
      //leds_.set(1, blink, blink);
      break;

    case UI_MODE_CALIBRATION_LOW:
      //leds_.set(0, slow_blink, 0);
      //leds_.set(1, slow_blink, 0);
      break;

    case UI_MODE_CALIBRATION_HIGH:
      //leds_.set(0, false, slow_blink);
      //leds_.set(1, false, slow_blink);
      break;
    
    case UI_MODE_EASTER_EGG_INTRO:
      {
        /*uint8_t pwm_counter = system_clock.milliseconds() & 15;
        uint8_t triangle_1 = (system_clock.milliseconds() / 7) & 31;
        uint8_t triangle_2 = (system_clock.milliseconds() / 17) & 31;
        triangle_1 = triangle_1 < 16 ? triangle_1 : 31 - triangle_1;
        triangle_2 = triangle_2 < 16 ? triangle_2 : 31 - triangle_2;*/
        /*leds_.set(
            0,
            triangle_1 > pwm_counter,
            triangle_2 > pwm_counter);
        leds_.set(
            1,
            triangle_2 > pwm_counter,
            triangle_1 > pwm_counter);*/
      }
      break;

    case UI_MODE_EASTER_EGG_OUTRO:
      {
        /*uint8_t pwm_counter = 7;
        uint8_t triangle_1 = (system_clock.milliseconds() / 9) & 31;
        uint8_t triangle_2 = (system_clock.milliseconds() / 13) & 31;
        triangle_1 = triangle_1 < 16 ? triangle_1 : 31 - triangle_1;
        triangle_2 = triangle_2 < 16 ? triangle_2 : 31 - triangle_2;*/
        //leds_.set(0, triangle_1 < pwm_counter, triangle_1 > pwm_counter);
        //leds_.set(1, triangle_2 > pwm_counter, triangle_2 < pwm_counter);
      }
      break;
    
    case UI_MODE_PANIC:
      //leds_.set(0, blink, false);
      //leds_.set(1, blink, false);
      break;
    
  }
}

void Ui::ReadSwitches() {
    hw_->ProcessDigitalControls();
  int s_pins[2] = {S_2,S_3};
  
  
  static uint32_t shiftTime{};
    
    switch (mode_) {
    case UI_MODE_NORMAL:
      {
       

    if(hw_->s[BTN_NOTE_STRUM].RisingEdge()){
        noteStrumState_ += 1;
        if(noteStrumState_ > 3) {
           noteStrumState_ = 0;     
        } 
    }
    if(hw_->s[BTN_EXCITER].RisingEdge()){
        exciterState_ += 1;
        if(exciterState_ > 1) {
           exciterState_ = 0;     
        } 
    }
    if(hw_->s[BTN_POLY].RisingEdge()){
        int32_t polyphony = part_->polyphony();
              if (polyphony == 3) {
                polyphony = 2;
              }
              polyphony <<= 1;
              if (polyphony > 4) {
                polyphony = 1;
              }
              part_->set_polyphony(polyphony);
              string_synth_->set_polyphony(polyphony);
              SaveState(); 
    }
    if(hw_->s[BTN_EGG_FX].RisingEdge()){
        eggFxState_ += 1;
        if(eggFxState_ > 5) {
           eggFxState_ = 0;     
        } 
    }
    if(hw_->s[BTN_MODEL].RisingEdge()){
        int32_t model = part_->model();
          if (model >= 3) {
            model -= 3;
          } else {
            model += 3;
          }
          part_->set_model(static_cast<ResonatorModel>(model));
          string_synth_->set_fx(static_cast<FxType>(model));
    }

    if(hw_->sw[0].Read() == 1){
        easterEggOn_ = true;
    } else {
        easterEggOn_ = false;
    }

    if (hw_->s[BTN_TAP].RisingEdge())    
    {
        shiftTime = System::GetNow();   //reset shift timer
    }

    if (hw_->s[BTN_TAP].FallingEdge())    //when button is let go shift is off
    {
        //saveState = false;
        hw_->SetRGBLed(1,OFF);
        hw_->SetRGBLed(2,OFF);
        hw_->SetRGBLed(3,OFF);
        hw_->SetRGBLed(4,OFF);
    }

    if (hw_->s[BTN_TAP].Pressed())
    {
        if ( (System::GetNow() - shiftTime) > shiftWait)
        {
            //saveState = true;
        } 
    }
      }
      break;
    
    case UI_MODE_CALIBRATION_C1:
      //leds_.set(0, blink, blink);
      //leds_.set(1, false, false);
      break;

    case UI_MODE_CALIBRATION_C3:
      //leds_.set(0, false, false);
      //leds_.set(1, blink, blink);
      break;

    case UI_MODE_CALIBRATION_LOW:
      //leds_.set(0, slow_blink, 0);
      //leds_.set(1, slow_blink, 0);
      break;

    case UI_MODE_CALIBRATION_HIGH:
      //leds_.set(0, false, slow_blink);
      //leds_.set(1, false, slow_blink);
      break;
    
    case UI_MODE_EASTER_EGG_INTRO:
      {
        /*uint8_t pwm_counter = system_clock.milliseconds() & 15;
        uint8_t triangle_1 = (system_clock.milliseconds() / 7) & 31;
        uint8_t triangle_2 = (system_clock.milliseconds() / 17) & 31;
        triangle_1 = triangle_1 < 16 ? triangle_1 : 31 - triangle_1;
        triangle_2 = triangle_2 < 16 ? triangle_2 : 31 - triangle_2;*/
        /*leds_.set(
            0,
            triangle_1 > pwm_counter,
            triangle_2 > pwm_counter);
        leds_.set(
            1,
            triangle_2 > pwm_counter,
            triangle_1 > pwm_counter);*/
      }
      break;

    case UI_MODE_EASTER_EGG_OUTRO:
      {
        /*uint8_t pwm_counter = 7;
        uint8_t triangle_1 = (system_clock.milliseconds() / 9) & 31;
        uint8_t triangle_2 = (system_clock.milliseconds() / 13) & 31;
        triangle_1 = triangle_1 < 16 ? triangle_1 : 31 - triangle_1;
        triangle_2 = triangle_2 < 16 ? triangle_2 : 31 - triangle_2;*/
        //leds_.set(0, triangle_1 < pwm_counter, triangle_1 > pwm_counter);
        //leds_.set(1, triangle_2 > pwm_counter, triangle_2 < pwm_counter);
      }
      break;
    
    case UI_MODE_PANIC:
      //leds_.set(0, blink, false);
      //leds_.set(1, blink, false);
      break;
  }
}

void Ui::Poll() {

  

  ui_task_ = (ui_task_ + 1) % 2;
  switch (ui_task_) {
    case 0:
      UpdateLEDs();
      break;
    
    case 1:
      ReadSwitches();
      break;
  }
  
}


void Ui::StartCalibration() {
  mode_ = UI_MODE_CALIBRATION_C1;
}

void Ui::CalibrateC1() {
  //cv_scaler_->CalibrateC1();
  //cv_scaler_->CalibrateOffsets();
  mode_ = UI_MODE_CALIBRATION_C3;
}

void Ui::CalibrateC3() {
  //bool success = cv_scaler_->CalibrateC3();
  bool success = true;
  if (success) {
    //settings_->Save();
    mode_ = UI_MODE_NORMAL;
  } else {
    mode_ = UI_MODE_PANIC;
  }
}


}  // namespace rinx