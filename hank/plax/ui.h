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

#ifndef PLAITS_UI_H_
#define PLAITS_UI_H_

#include "stmlib/stmlib.h"


#include "../daisy_hank.h"
#include "dsp/voice.h"
#include "pot_controller.h"
#include "settings.h"

namespace plaits {

using namespace daisy;

const int kNumNormalizedChannels = 3;
const int kProbeSequenceDuration = 32;
const int kNumLEDs = 4;

enum UiMode {
  UI_MODE_NORMAL,
  UI_MODE_DISPLAY_OCTAVE,
  UI_MODE_DISPLAY_VCFA_VCA,
  UI_MODE_DISPLAY_TIME_DECAY,
  UI_MODE_DISPLAY_CV_CTRL,
  UI_MODE_CALIBRATION_C1,
  UI_MODE_CALIBRATION_C3,
  UI_MODE_TEST,
  UI_MODE_ERROR,
  UI_MODE_RESTORE_STATE,
  UI_MODE_HIDDEN_PATCHED,
};

enum CvCtrlState {
  VOCT,
  FM,
  HARM,
  TIMBRE,
  MORPH,
  MODEL,
  CV_CTRL_LAST,
};




class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(Patch* patch, Modulations* modulations, Voice* voice, Settings* settings, DaisyHank* hw);
  
  void Poll();
  
  void set_active_engine(int active_engine) {
    active_engine_ = active_engine;
  }
  void SaveCalibrationData();
  void SaveStateData();
  void SaveState();

  void StartCalibration();
  void CalibrateC1();
  void CalibrateC3();
  

  bool readyToSaveState = false;
  bool readyToRestore = false;

  float lfovalue;
  float plaits_cv_scale[CV_CTRL_LAST] = {0,60.0,1,1.6,1.6,1.03};

  
 private:
  void UpdateLEDs();
  void ReadSwitches();
  void ProcessPotsHiddenParameters();
  void LoadState();
  
  
  void DetectNormalization();
  
  

  void RealignPots() {
    pots_[DaisyHank::KNOB_3].Realign();
    pots_[DaisyHank::KNOB_4].Realign();
    pots_[DaisyHank::KNOB_2].Realign();
    pots_[DaisyHank::KNOB_1].Realign();
  }

  void LoadStateData();

  void SetLedsState(int idx);
    
  UiMode mode_;
  CvCtrlState cv_ctrl_state_;
  
  
  int ui_task_;

  float transposition_;
  float octave_;
  float cv_ctrl_;
  uint8_t cblind_;
  DaisyHank* hw_;
  Patch* patch_;
  Modulations* modulations_;
  Voice* voice_;
  PotController pots_[DaisyHank::KNOB_LAST];
  float pitch_lp_;
  float pitch_lp_calibration_;
  
  Settings* settings_;
  

  bool isPatched[PATCHED_LAST] = {false};

  int pwm_counter_;
  int press_time_;
  bool ignore_release_;
  
  int active_engine_;
  int size_engines_;
  
  float cv_c1_;  // For calibration
  
};

}  // namespace plaits

#endif  // PLAITS_UI_H_
