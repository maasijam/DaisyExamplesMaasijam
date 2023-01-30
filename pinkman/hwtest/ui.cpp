#include "ui.h"

#include <algorithm>


  
using namespace std;



static const int32_t kLongPressTime = 2000;



void Ui::Init(Settings* settings, DaisyPinkman* hw) {
  hw_ = hw;
  settings_ = settings;

  

  ui_task_ = 0;
  mode_ = UI_MODE_NORMAL;

    
  LoadState();


}

void Ui::LoadState() {
    for (size_t i = 0; i < 14; i++)
    {
      //ledstate[i] = false;
    }
    
  
}


void Ui::SaveState() {
  
}

void Ui::UpdateLEDs() {
  
  
  
  

  switch (mode_) {
    case UI_MODE_NORMAL:
    {
      

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
  
}

void Ui::ReadSwitches() {

  hw_->ProcessDigitalControls();

  
  
  switch (mode_) {
    case UI_MODE_NORMAL:
                  
         

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





