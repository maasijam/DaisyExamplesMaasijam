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

#ifndef SUPERSAW_UI_H_
#define SUPERSAW_UI_H_



#include "../daisy_margolis.h"


namespace supersaw {

using namespace daisy;
using namespace margolis;

const int kNumLEDs = 8;

enum UiMode {
  UI_MODE_NORMAL,
  UI_MODE_CALIBRATION_C1,
  UI_MODE_CALIBRATION_C3,
  UI_MODE_ERROR
};



class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(DaisyMargolis* hw);
  void Poll();
  

  
 private:
  void UpdateLEDs();
  void ReadSwitches();
  void LoadState();
  void SaveState();
    
  void StartCalibration();
  void CalibrateC1();
  void CalibrateC3();
  void SaveCalibrationData();

    
  UiMode mode_;
  
  int pwm_counter_;
  
  int ui_task_;

  
  DaisyMargolis* hw_;
  

  float pitch_lp_calibration_;
  
  float cv_c1_;  // For calibration
  


};

}  // namespace plaits

#endif  // SUPERSAW_UI_H_
