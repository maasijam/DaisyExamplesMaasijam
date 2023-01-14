#include "ui.h"

#include <algorithm>

namespace white {
  
using namespace std;



static const int32_t kLongPressTime = 2000;



void Ui::Init(Settings* settings, DaisyWhite* hw) {
  hw_ = hw;
  settings_ = settings;

  

  ui_task_ = 0;
  mode_ = UI_MODE_NORMAL;

    
  LoadState();


}

void Ui::LoadState() {
    for (size_t i = 0; i < 14; i++)
    {
      ledstate[i] = false;
    }
    
  
}


void Ui::SaveState() {
  
}

void Ui::UpdateLEDs() {
  hw_->ClearLeds();
  
  
  

  switch (mode_) {
    case UI_MODE_NORMAL:
    {
      if(ledstate[0]){
          hw_->SetRGBColor(RGB_LED_1,colorRGB[0]);
        }
      if(ledstate[1]){
          hw_->SetGreenLeds(GREEN_LED_3,greenLedBright[0]);
        }
      if(ledstate[2]){
          hw_->SetGreenDirectLeds(GREEN_D_LED_1,1.f);
        }
      if(ledstate[3]){
          hw_->SetRGBColor(RGB_LED_2,colorRGB[0]);
        }
      if(ledstate[4]){
          hw_->SetRGBColor(RGB_LED_3,colorRGB[1]);
        }
      if(ledstate[5]){
          hw_->SetGreenDirectLeds(GREEN_D_LED_4,1.f);
        }
      if(ledstate[6]){
          hw_->SetGreenDirectLeds(GREEN_D_LED_6,1.f);
        }
      if(ledstate[7]){
          hw_->SetRGBColor(RGB_LED_4,colorRGB[1]);
        }

        if(ledstate[8]){
          hw_->SetGreenDirectLeds(GREEN_D_LED_2,1.f);
        }
        if(ledstate[9]){
          hw_->SetGreenDirectLeds(GREEN_D_LED_5,1.f);
        }

        if(ledstate[10]){
          hw_->SetGreenLeds(GREEN_LED_1,1.f);
        }

        if(ledstate[11]){
          hw_->SetGreenLeds(GREEN_LED_2,1.f);
        }

        if(ledstate[13]){
          hw_->SetGreenLeds(GREEN_LED_4,1.f);
        }

        if(ledstate[12]){
          hw_->SetGreenDirectLeds(GREEN_D_LED_3,1.f);
        }

     }
      break;
  
      
    case UI_MODE_CALIBRATION_C1:
      
      break;

    case UI_MODE_CALIBRATION_C3:
      
      break;
    
    case UI_MODE_ERROR:
      
      break;

      

      case UI_MODE_RESTORE_STATE:
      
        
      
      break;

      
  }
  hw_->UpdateLeds();
}

void Ui::ReadSwitches() {

  hw_->ProcessDigitalControls();

  
  
  switch (mode_) {
    case UI_MODE_NORMAL:
        if(hw_->SwitchRisingEdge(S1)){
          ledstate[0] = !ledstate[0];
        }
        if(hw_->SwitchRisingEdge(S2)){
          ledstate[1] = !ledstate[1];
        }
        if(hw_->SwitchRisingEdge(S3)){
          ledstate[2] = !ledstate[2];
        }
        if(hw_->SwitchRisingEdge(S4)){
          ledstate[3] = !ledstate[3];
        }
        if(hw_->SwitchRisingEdge(S5)){
          ledstate[4] = !ledstate[4];
        }
        if(hw_->SwitchRisingEdge(S6)){
          ledstate[5] = !ledstate[5];
        }
        if(hw_->SwitchRisingEdge(S7)){
          ledstate[6] = !ledstate[6];
        }
        if(hw_->SwitchRisingEdge(S8)){
          ledstate[7] = !ledstate[7];
        }

        //if(hw_->SwitchState(S0B)){
          ledstate[8] = !hw_->SwitchState(S0B) ? true : false;
          ledstate[9] = !hw_->SwitchState(S1B) ? true : false;

          ledstate[10] = !hw_->SwitchState(S0A) ? true : false;
          ledstate[11] = !hw_->SwitchState(S1A) ? true : false;

          ledstate[12] = hw_->SwitchState(S1A)  && hw_->SwitchState(S1B)  ? true : false;
          ledstate[13] = hw_->SwitchState(S0A) && hw_->SwitchState(S0B) ? true : false;

          colorRGB[0] = GREEN;
          colorRGB[1] = GREEN;
          
          if(!hw_->SwitchState(S0B))
            colorRGB[0] = RED;  
          if(!hw_->SwitchState(S1B))
            colorRGB[1] = RED; 

          if(!hw_->SwitchState(S0A))
            colorRGB[0] = BLUE; 
          if(!hw_->SwitchState(S1A))
            colorRGB[1] = BLUE;

          
         
        //}
      break;
      
   
    
    case UI_MODE_CALIBRATION_C1:
      
      break;
      
    case UI_MODE_CALIBRATION_C3:
      
      
      break;

    
    case UI_MODE_ERROR:
    case UI_MODE_RESTORE_STATE:
      
      break;
      

  }
}



void Ui::Poll() {

  hw_->ProcessAnalogControls();
  
  greenLedBright[0] = hw_->GetKnobValue(KNOB_0);
  
  
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

}

void Ui::CalibrateC3() {
  
}





}  // namespace white
