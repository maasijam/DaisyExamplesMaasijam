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
  UI_MODE_SAVE_STATE,
  UI_MODE_ERROR
};

/** @brief State data container
 * 
*/
struct StateData
{
    StateData() : engine(0) {}
    int engine;
        

    /** @brief checks sameness */
    bool operator==(const StateData &rhs)
    {
        if(engine != rhs.engine)
        {
            return false;
        }
        
        return true;
    }

    /** @brief Not equal operator */
    bool operator!=(const StateData &rhs) { return !operator==(rhs); }
};

class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(DaisyMargolis* hw);
  void Poll();
  UiMode GetUiMode(){return mode_;};
  int GetEngine(){return engine_;};
  int GetTask(){return ui_task_;};
  void SaveCalibrationData();
  void SaveStateData();

  bool readyToSaveState = false;
  
 private:
  void UpdateLEDs();
  void ReadSwitches();
  void LoadState();
  void SaveState();
    
  void StartCalibration();
  void CalibrateC1();
  void CalibrateC3();
  
  void LoadStateData();
  

  void SetStateData(int &statedata);
  void GetStateData(int &statedata);


    
  UiMode mode_;
  
  int pwm_counter_;
  
  int ui_task_;
  int engine_;

  

  
  DaisyMargolis* hw_;
  

  float pitch_lp_calibration_;
  
  float cv_c1_;  // For calibration
  


};

}  // namespace plaits

#endif  // SUPERSAW_UI_H_
