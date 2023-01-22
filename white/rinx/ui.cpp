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

#include <algorithm>

//#include "stmlib/system/system_clock.h"

#include "cv_scaler.h"
#include "dsp/part.h"
#include "dsp/string_synth_part.h"

namespace rings {

const int32_t kLongPressDuration = 3000;

using namespace std;
using namespace stmlib;
using namespace white;

void Ui::Init(
    Settings* settings,
    CvScaler* cv_scaler,
    Part* part,
    StringSynthPart* string_synth,
    DaisyWhite* hw) {
  //leds_.Init();
  //switches_.Init();
  
  settings_ = settings;
  cv_scaler_ = cv_scaler;
  part_ = part;
  string_synth_ = string_synth;
  hw_ = hw;

  ui_task_ = 0;
  onoff = false;
  
  if (hw_->SwitchRisingEdge(S5)) {
    State* state = settings_->mutable_state();
    if (state->color_blind == 1) {
      state->color_blind = 0; 
    } else {
      state->color_blind = 1; 
    }
    settings_->SaveState();
  }
  
  
  part_->set_polyphony(settings_->state().polyphony);
  part_->set_model(static_cast<ResonatorModel>(settings_->state().model));
  string_synth_->set_polyphony(settings_->state().polyphony);
  string_synth_->set_fx(static_cast<FxType>(settings_->state().model));
  mode_ = settings_->state().easter_egg
      ? UI_MODE_EASTER_EGG_INTRO
      : UI_MODE_NORMAL;
  //mode_ = UI_MODE_NORMAL;
}

void Ui::SaveState() {
  settings_->mutable_state()->polyphony = part_->polyphony();
  settings_->mutable_state()->model = part_->model();
  settings_->SaveState();
}

void Ui::AnimateEasterEggLeds() {
  mode_ = settings_->state().easter_egg
      ? UI_MODE_EASTER_EGG_INTRO
      : UI_MODE_EASTER_EGG_OUTRO;
}

void Ui::UpdateLeds() {
  // 1kHz.
  system_clock.GetTick();
  //switches_.Debounce();
  hw_->ClearLeds();
 
   
  bool blink = (system_clock.GetNow() & 127) > 64;
  bool slow_blink = (system_clock.GetNow() & 255) > 128;
/*
  switch (settings_->state().model)
  {
  case 0:
    hw_->SetRGBColor(RGB_LED_3,RED);
    break;
  case 1:
    hw_->SetRGBColor(RGB_LED_3,GREEN);
    break;
  case 2:
    hw_->SetRGBColor(RGB_LED_3,BLUE);
    break;
  case 3:
    hw_->SetRGBColor(RGB_LED_3,PURPLE);
    break;
  case 4:
    hw_->SetRGBColor(RGB_LED_3,WHITE);
    break;
  case 5:
    hw_->SetRGBColor(RGB_LED_3,CYAN);
    break;
    default:
    hw_->SetRGBColor(RGB_LED_3,YELLOW);
    break;
  }
  */
  //hw_->UpdateLeds();

  

  

  switch (mode_) {
    case UI_MODE_NORMAL:
      {
        uint8_t pwm_counter = system_clock.GetNow() & 15;
        uint8_t triangle = (system_clock.GetNow() >> 5) & 31;
        triangle = triangle < 16 ? triangle : 31 - triangle;

        hw_->SetRgbLeds(RGB_LED_2,0.f,0.f,onoff ? 1.f : 0.f);

        if (settings_->state().color_blind == 1) {
          uint8_t mode_red_brightness[] = {
            0, 15, 1,
            0, triangle, uint8_t(triangle >> 3)
          };
          uint8_t mode_green_brightness[] = {
            4, 15, 0, 
            uint8_t(triangle >> 1), triangle, 0,
          };
          
          uint8_t poly_counter = (system_clock.GetNow() >> 7) % 12;
          uint8_t poly_brightness = (poly_counter >> 1) < part_->polyphony() &&
                (poly_counter & 1);
          uint8_t poly_red_brightness = part_->polyphony() >= 2
              ? 8 + 8 * poly_brightness
              : 0;
          uint8_t poly_green_brightness = part_->polyphony() <= 3
              ? 8 + 8 * poly_brightness
              : 0;
          if (part_->polyphony() == 1 || part_->polyphony() == 4) {
            poly_red_brightness >>= 3;
            poly_green_brightness >>= 2;
          }
          /*leds_.set(
              0,
              pwm_counter < poly_red_brightness,
              pwm_counter < poly_green_brightness);*/
          hw_->SetRgbLeds(RGB_LED_1,(pwm_counter < poly_red_brightness ? 1.f : 0.f),(pwm_counter < poly_green_brightness ? 1.f : 0.f),0.f);
          /*leds_.set(
              1,
              pwm_counter < mode_red_brightness[part_->model()],
              pwm_counter < mode_green_brightness[part_->model()]);*/
          hw_->SetRgbLeds(RGB_LED_4,(pwm_counter < mode_red_brightness[part_->model()] ? 1.f : 0.f),(pwm_counter < mode_green_brightness[part_->model()] ? 1.f : 0.f),0.f);
        } else {
          //leds_.set(0, part_->polyphony() >= 2, part_->polyphony() <= 2);
          //leds_.set(1, part_->model() >= 1, part_->model() <= 1);

          hw_->SetRgbLeds(RGB_LED_1,(part_->polyphony() >= 2 ? 1.f : 0.f),(part_->polyphony() <= 2 ? 1.f : 0.f),0.f);
          hw_->SetRgbLeds(RGB_LED_4,(part_->model() >= 1 ? 1.f : 0.f),(part_->model() <= 1 ? 1.f : 0.f),0.f);

          // Fancy modes!
          if (part_->polyphony() == 3) {
            //leds_.set(0, true, pwm_counter < triangle);
            hw_->SetRgbLeds(RGB_LED_1,1.f,(pwm_counter < triangle ? 1.f : 0.f),0.f);
          }
          if (part_->model() >= 3) {
            bool led_1 = part_->model() >= 4 && pwm_counter < triangle;
            bool led_2 = part_->model() <= 4 && pwm_counter < triangle;
            //leds_.set(1, led_1, led_2);
            hw_->SetRgbLeds(RGB_LED_4,led_1,led_2,0.f);
          }
        }
        ++strumming_flag_interval_;
        if (strumming_flag_counter_) {
          --strumming_flag_counter_;
          //leds_.set(0, false, false);
          hw_->SetRgbLeds(RGB_LED_1,0.f,0.f,0.f);
        }
      }
      break;
    
    case UI_MODE_CALIBRATION_C1:
      //leds_.set(0, blink, blink);
      hw_->SetRgbLeds(RGB_LED_1,blink ? 1.f : 0.f, blink ? 1.f : 0.f,0.f);
      //leds_.set(1, false, false);
      hw_->SetRgbLeds(RGB_LED_4,0.f,0.f,0.f);
      break;

    case UI_MODE_CALIBRATION_C3:
      //leds_.set(0, false, false);
      hw_->SetRgbLeds(RGB_LED_1,0.f,0.f,0.f);
      //leds_.set(1, blink, blink);
      hw_->SetRgbLeds(RGB_LED_4,blink ? 1.f : 0.f, blink ? 1.f : 0.f,0.f);
      break;

    case UI_MODE_CALIBRATION_LOW:
      //leds_.set(0, slow_blink, 0);
      hw_->SetRgbLeds(RGB_LED_1,slow_blink ? 1.f : 0.f, 0.f,0.f);
      //leds_.set(1, slow_blink, 0);
      hw_->SetRgbLeds(RGB_LED_4,slow_blink ? 1.f : 0.f, 0.f,0.f);
      break;

    case UI_MODE_CALIBRATION_HIGH:
      //leds_.set(0, false, slow_blink);
      hw_->SetRgbLeds(RGB_LED_1,0.f,slow_blink ? 1.f : 0.f,0.f);
      //leds_.set(1, false, slow_blink);
      hw_->SetRgbLeds(RGB_LED_4,0.f,slow_blink ? 1.f : 0.f,0.f);
      break;
    
    case UI_MODE_EASTER_EGG_INTRO:
      {
        uint8_t pwm_counter = system_clock.GetNow() & 15;
        uint8_t triangle_1 = (system_clock.GetNow() / 7) & 31;
        uint8_t triangle_2 = (system_clock.GetNow() / 17) & 31;
        triangle_1 = triangle_1 < 16 ? triangle_1 : 31 - triangle_1;
        triangle_2 = triangle_2 < 16 ? triangle_2 : 31 - triangle_2;
        /*leds_.set(
            0,
            triangle_1 > pwm_counter,
            triangle_2 > pwm_counter);*/
        hw_->SetRgbLeds(RGB_LED_1,(triangle_1 > pwm_counter ? 1.f : 0.f),(triangle_2 > pwm_counter ? 1.f : 0.f),0.f);
        /*leds_.set(
            1,
            triangle_2 > pwm_counter,
            triangle_1 > pwm_counter);*/
        hw_->SetRgbLeds(RGB_LED_4,(triangle_2 > pwm_counter ? 1.f : 0.f),(triangle_1 > pwm_counter ? 1.f : 0.f),0.f);
      }
      break;

    case UI_MODE_EASTER_EGG_OUTRO:
      {
        uint8_t pwm_counter = 7;
        uint8_t triangle_1 = (system_clock.GetNow() / 9) & 31;
        uint8_t triangle_2 = (system_clock.GetNow() / 13) & 31;
        triangle_1 = triangle_1 < 16 ? triangle_1 : 31 - triangle_1;
        triangle_2 = triangle_2 < 16 ? triangle_2 : 31 - triangle_2;
        //leds_.set(0, triangle_1 < pwm_counter, triangle_1 > pwm_counter);
        hw_->SetRgbLeds(RGB_LED_1,(triangle_1 < pwm_counter ? 1.f : 0.f),(triangle_1 > pwm_counter ? 1.f : 0.f),0.f);
        //leds_.set(1, triangle_2 > pwm_counter, triangle_2 < pwm_counter);
        hw_->SetRgbLeds(RGB_LED_4,(triangle_2 > pwm_counter ? 1.f : 0.f),(triangle_2 < pwm_counter ? 1.f : 0.f),0.f);
      }
      break;
    
    case UI_MODE_PANIC:
      //leds_.set(0, blink, false);
      hw_->SetRgbLeds(RGB_LED_1,blink ? 1.f : 0.f, 0.f,0.f);
      //leds_.set(1, blink, false);
      hw_->SetRgbLeds(RGB_LED_4,blink ? 1.f : 0.f, 0.f,0.f);
      break;
  }
  //leds_.Write();
  hw_->UpdateLeds();
}


void Ui::StartCalibration() {
  mode_ = UI_MODE_CALIBRATION_C1;
}

void Ui::CalibrateC1() {
  cv_scaler_->CalibrateC1();
  cv_scaler_->CalibrateOffsets();
  mode_ = UI_MODE_CALIBRATION_C3;
}

void Ui::CalibrateC3() {
  bool success = cv_scaler_->CalibrateC3();
  if (success) {
    settings_->SaveCalibration();
    mode_ = UI_MODE_NORMAL;
  } else {
    mode_ = UI_MODE_PANIC;
  }
}
/*
void Ui::StartNormalizationCalibration() {
  cv_scaler_->StartNormalizationCalibration();
  mode_ = UI_MODE_CALIBRATION_LOW;
}

void Ui::CalibrateLow() {
  cv_scaler_->CalibrateLow();
  mode_ = UI_MODE_CALIBRATION_HIGH;
}

void Ui::CalibrateHigh() {
  bool success = cv_scaler_->CalibrateHigh();
  if (success) {
    settings_->Save();
    mode_ = UI_MODE_NORMAL;
  } else {
    mode_ = UI_MODE_PANIC;
  }
}
*/
/*void Ui::DoEvents() {
  
  while (!queue_.IsQueueEmpty()) {
    //queue_.GetAndRemoveNextEvent()
    UiEventQueue::Event e = queue_.GetAndRemoveNextEvent();
    if (e.type == UiEventQueue::Event::EventType::buttonPressed) {
      
      if (e.asButtonPressed.id == S2) {
        OnSwitchPressed(e);
      } else {
        OnSwitchReleased(e);
      }
    }
  }
}*/

void Ui::ReadSwitches() {

  hw_->ProcessDigitalControls();
  int s_pins[2] = {S4,S5};



      switch (mode_) {
          case UI_MODE_CALIBRATION_C1:
            CalibrateC1();
            break;
          case UI_MODE_CALIBRATION_C3:
            CalibrateC3();
            break;
          case UI_MODE_CALIBRATION_LOW:
            //CalibrateLow();
            break;
          case UI_MODE_CALIBRATION_HIGH:
            //CalibrateHigh();
            break;
          case UI_MODE_NORMAL:
            {
              if (hw_->SwitchRisingEdge(S4)) {
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
                //SaveState();
                readyToSaveState = true;
              }
              
              if (hw_->SwitchRisingEdge(S5)) {
                  int32_t model = part_->model();
                  if (model >= 3) {
                    model -= 3;
                  } else {
                    model = (model + 1) % 3;
                  }
                  part_->set_model(static_cast<ResonatorModel>(model));
                  string_synth_->set_fx(static_cast<FxType>(model));
                  //SaveState();
                  readyToSaveState = true;
              }

              if (hw_->SwitchRisingEdge(S1)) {
                  onoff = !onoff;
                  readyToRestore = true;
              }
              
            }
            break;
          }
  }

void Ui::Poll() {

  //UpdateLeds();
  //ReadSwitches();

  ui_task_ = (ui_task_ + 1) % 4;
  switch (ui_task_) {
    case 0:
      UpdateLeds();
      break;
    
    case 1:
      ReadSwitches();
      break;
    case 2:
      
      break;
    case 3:
      
      break;
  }
  
}
  
  //if (queue_.idle_time() > 800 && mode_ == UI_MODE_PANIC) {
  //  mode_ = UI_MODE_NORMAL;
  //}
  /*
  if (mode_ == UI_MODE_EASTER_EGG_INTRO || mode_ == UI_MODE_EASTER_EGG_OUTRO) {
    if (queue_.idle_time() > 3000) {
      mode_ = UI_MODE_NORMAL;
      queue_.Touch();
    }
  } else if (queue_.idle_time() > 1000) {
    queue_.Touch();
  }*/



/*
uint8_t Ui::HandleFactoryTestingRequest(uint8_t command) {
  uint8_t argument = command & 0x1f;
  command = command >> 5;
  uint8_t reply = 0;
  switch (command) {
    case FACTORY_TESTING_READ_POT:
    case FACTORY_TESTING_READ_CV:
      reply = cv_scaler_->adc_value(argument);
      break;
    
    case FACTORY_TESTING_READ_NORMALIZATION:
      reply = cv_scaler_->normalization(argument);
      break;      
    
    case FACTORY_TESTING_READ_GATE:
      reply = argument == 2
          ? cv_scaler_->gate_value()
          : switches_.pressed(argument);
      break;
      
    case FACTORY_TESTING_SET_BYPASS:
      part_->set_bypass(argument);
      break;
      
    case FACTORY_TESTING_CALIBRATE:
      {
        switch (argument) {
          case 0:
            StartCalibration();
            break;
          
          case 1:
            CalibrateC1();
            break;
          
          case 2:
            CalibrateC3();
            break;
          
          case 3:
            StartNormalizationCalibration();
            break;

          case 4:
            CalibrateLow();
            break;
          
          case 5:
            CalibrateHigh();
            queue_.Touch();
            break;
        }
      }
      break;
  }
  return reply;
}
*/
}  // namespace rings
