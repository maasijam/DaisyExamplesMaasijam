#include "ui.h"
#include "daisysp.h"

namespace supersaw {
  

using namespace margolis;
using namespace daisysp;

static const int32_t kLongPressTime = 1000;



void Ui::Init(DaisyMargolis* hw) {
  hw_ = hw;
  

  ui_task_ = 0;
  mode_ = UI_MODE_NORMAL;
  

  
  
  cv_c1_ = 0.0f;

}

void Ui::LoadState() {
  
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
        
        hw_->SetRGBColor(LED_RGB_5,hw_->ledcolor);

      }
      break;
    
          
    case UI_MODE_CALIBRATION_C1:
      if (pwm_counter < triangle) {
        //leds_.set(0, LED_COLOR_GREEN);
        hw_->SetRGBColor(LED_RGB_2,GREEN);
      }
      break;

    case UI_MODE_CALIBRATION_C3:
      if (pwm_counter < triangle) {
        //leds_.set(0, LED_COLOR_YELLOW);
        hw_->SetRGBColor(LED_RGB_3,PURPLE);
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
        
        if(hw_->s[S6].TimeHeldMs() >= kLongPressTime && hw_->s[S4].TimeHeldMs() >= kLongPressTime) {
          StartCalibration();
        }
        
        if(hw_->s[S2].RisingEdge()) {
          hw_->ledcount++;
          
          if(hw_->ledcount == static_cast<int>(OFF)) {
            hw_->ledcount = 0;
          }
          hw_->SetLedcolorData(static_cast<Colors>(hw_->ledcount));
        }

        //if(hw_->s[S5].TimeHeldMs() >= kLongPressTime) {
          
        //}
        

      }
      break;
      
    
    
    case UI_MODE_CALIBRATION_C1:
      
        //if (switches_.just_pressed(Switch(i))) {
        if (hw_->s[S1].RisingEdge()) {
          
          CalibrateC1();
          
        }
     
      break;
      
    case UI_MODE_CALIBRATION_C3:
        //if (switches_.just_pressed(Switch(i))) {
        if (hw_->s[S1].RisingEdge()) {
         
          CalibrateC3();
          
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
       SaveCalibrationData();
       mode_ = UI_MODE_NORMAL;
    } else {
      mode_ = UI_MODE_ERROR;
    }
  //normalization_probe_.Init();
}


/** @brief Loads and sets calibration data */
void Ui::SaveCalibrationData()
{
    daisy::PersistentStorage<CalibrationData> cal_storage(hw_->seed.qspi);
    CalibrationData                           default_cal;
    cal_storage.Init(default_cal, 4096);
    auto &cal_data = cal_storage.GetSettings();
    hw_->GetWarpCalData(cal_data.warp_scale, cal_data.warp_offset);
    hw_->GetCvOffsetData(cal_data.cv_offset);
    hw_->GetLedcolorData(cal_data.ledcolor);
    cal_storage.Save();
}


}  // namespace supersaw
