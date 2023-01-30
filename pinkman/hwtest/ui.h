#ifndef PINKMAN_UI_H_
#define PINKMAN_UI_H_



#include "../daisy_pinkman.h"
#include "settings.h"



using namespace daisy;
using namespace pinkman;




enum UiMode {
  UI_MODE_NORMAL,
  UI_MODE_CALIBRATION_C1,
  UI_MODE_CALIBRATION_C3,
  UI_MODE_ERROR,
  UI_MODE_RESTORE_STATE,
};






class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(Settings* settings, DaisyPinkman* hw);
  
  void Poll();
  
  
  void SaveCalibrationData();
  void SaveStateData();
  void SaveState();
  

  bool readyToSaveState = false;
  bool readyToRestore = false;

  float lfovalue;


  
 private:
  void UpdateLEDs();
  void ReadSwitches();
  void LoadState();
   
  void StartCalibration();
  void CalibrateC1();
  void CalibrateC3();

  
  void LoadStateData();
    
  UiMode mode_;
  
  
  
  int ui_task_;




  DaisyPinkman* hw_;
  
  float pitch_lp_;
  float pitch_lp_calibration_;
  
  Settings* settings_;
  

  

};



#endif  // PLAITS_UI_H_
