#include "ui.h"
#include "daisysp.h"

namespace supersaw {
  

using namespace margolis;
using namespace daisysp;

static const int32_t kLongPressTime = 2000;



void Ui::Init(DaisyMargolis* hw) {
  hw_ = hw;
  

  ui_task_ = 0;
  mode_ = UI_MODE_NORMAL;
  engine_ = 0;

  
  
  cv_c1_ = 0.0f;
  LoadStateData();

}


void Ui::SaveState() {
  
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
            
        }
        if(engine_ > 7) {
            engine_ = 0;
          }
         if(hw_->s[S3].TimeHeldMs() >= kLongPressTime) {
            readyToSaveState = true;
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

void Ui::SetStateData(int &eng)
{
    engine_ = eng;
}

void Ui::GetStateData(int &eng)
{
    eng = engine_;
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

/** @brief Loads and sets state data */
void Ui::LoadStateData()
{
    daisy::PersistentStorage<StateData> state_storage(hw_->seed.qspi);
    StateData                           default_state;
    state_storage.Init(default_state, FLASH_BLOCK*2);
    auto &state_data = state_storage.GetSettings();
    SetStateData(state_data.engine);
}

/** @brief Saves state data */
void Ui::SaveStateData()
{
    daisy::PersistentStorage<StateData> state_storage(hw_->seed.qspi);
    StateData                           default_state;
    state_storage.Init(default_state, FLASH_BLOCK*2);
    auto &state_data = state_storage.GetSettings();
    GetStateData(state_data.engine);
    state_storage.Save();
    
}

}  // namespace supersaw
