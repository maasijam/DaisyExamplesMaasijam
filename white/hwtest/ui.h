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

#ifndef WHITE_UI_H_
#define WHITE_UI_H_



#include "../daisy_white.h"
#include "settings.h"

namespace white {

using namespace daisy;




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
  
  void Init(Settings* settings, DaisyWhite* hw);
  
  void Poll();
  
  
  void SaveCalibrationData();
  void SaveStateData();
  void SaveState();
  

  bool readyToSaveState = false;
  bool readyToRestore = false;

  float lfovalue;
  bool ledstate[14];

  
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
  float greenLedBright[10] = {0};

  Colors colorRGB[2];

  DaisyWhite* hw_;
  
  float pitch_lp_;
  float pitch_lp_calibration_;
  
  Settings* settings_;
  

  

};

}  // namespace plaits

#endif  // PLAITS_UI_H_
