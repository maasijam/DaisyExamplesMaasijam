#include "ui.h"
#include "daisysp.h"

namespace super {
  

using namespace margolis;
using namespace daisysp;

static const int32_t kLongPressTime = 2000;



void Ui::Init(DaisyMargolis* hw, Settings* settings) {
  hw_ = hw;
  settings_ = settings;

  

  ui_task_ = 0;
  mode_ = UI_MODE_NORMAL;
  engine_ = 0;

  
  
  cv_c1_ = 0.0f;
  LoadState();

}
void Ui::LoadState() {
  const State& state = settings_->state();
  engine_ = state.engine;
  
}


void Ui::SaveState() {
  State* state = settings_->mutable_state();
  state->engine = engine_;
  
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
        
        hw_->SetRGBColor(static_cast<LeddriverLeds>(engine_),BLUE);

      }
      break;
    
          
    case UI_MODE_CALIBRATION_C1:
      if (pwm_counter < triangle) {
        for (int i = 0; i < kNumLEDs; ++i) {
          //leds_.set(i, LED_COLOR_RED);
          hw_->SetRGBColor(static_cast<LeddriverLeds>(i),GREEN);
        }
      }
      break;

    case UI_MODE_CALIBRATION_C3:
      if (pwm_counter < triangle) {
        for (int i = 0; i < kNumLEDs; ++i) {
          //leds_.set(i, LED_COLOR_RED);
          hw_->SetRGBColor(static_cast<LeddriverLeds>(i),PURPLE);
        }
      }
      break;

      case UI_MODE_SAVE_STATE:
      {
        
        if (pwm_counter < triangle) {
        for (int i = 0; i < kNumLEDs; ++i) {
          //leds_.set(i, LED_COLOR_RED);
          hw_->SetRGBColor(static_cast<LeddriverLeds>(i),ORANGE);
        }
      }

      }
      break;
    
    case UI_MODE_ERROR:
      if (pwm_counter < triangle) {
        for (int i = 0; i < kNumLEDs; ++i) {
          //leds_.set(i, LED_COLOR_RED);
          hw_->SetRGBColor(static_cast<LeddriverLeds>(i),RED);
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
        
        if(hw_->s[S2].TimeHeldMs() >= kLongPressTime) {
          StartCalibration();
        }
        
        if(hw_->s[S1].FallingEdge()) {
            engine_++;
            if(engine_ > 7) {
              engine_ = 0;
            }
            readyToSaveState = true;
            //mode_ = UI_MODE_SAVE_STATE;
        }
        
         if(hw_->s[S3].TimeHeldMs() >= kLongPressTime) {
            readyToRestoreState = true;
            mode_ = UI_MODE_SAVE_STATE;
        }

      }
      break;
      
    
    
    case UI_MODE_CALIBRATION_C1:
      
        //if (switches_.just_pressed(Switch(i))) {
        if (hw_->s[S2].RisingEdge()) {
          
          CalibrateC1();
          
        }
     
      break;
      
    case UI_MODE_CALIBRATION_C3:
        //if (switches_.just_pressed(Switch(i))) {
        if (hw_->s[S2].RisingEdge()) {
         
          CalibrateC3();
          
        }
     
      break;

      case UI_MODE_SAVE_STATE:
      {
        
        if(!readyToSaveState)
          mode_ = UI_MODE_NORMAL;
      }
      break;

    case UI_MODE_ERROR:
      
        //if (switches_.just_pressed(Switch(i))) {
        if (hw_->s[S1].RisingEdge()) {
         
          mode_ = UI_MODE_NORMAL;
        }
     
      break;
  }
}

void Ui::Poll() {
  //hw_->ProcessAnalogControls();
  //ONE_POLE(pitch_lp_calibration_, hw_->cv[CV_VOCT].Value(), 0.1f);
fonepole(pitch_lp_calibration_, hw_->cv[CV_VOCT].Value(), 0.1f);

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
  //normalization_probe_.Disable();
}

void Ui::CalibrateC1() {
  // Acquire offsets for all channels.
  //hw_->GetCvOffsetData(float *data);
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
       //SaveCalibrationData();
       mode_ = UI_MODE_NORMAL;
    } else {
      mode_ = UI_MODE_ERROR;
    }
  //normalization_probe_.Init();
}



}  // namespace super
