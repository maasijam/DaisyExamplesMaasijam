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


#include "../daisy_margolis.h"
#include "dsp/voice.h"
#include "pot_controller.h"
//#include "settings.h"

namespace plaits {

using namespace daisy;
using namespace margolis;

const int kNumNormalizedChannels = 3;
const int kProbeSequenceDuration = 32;
const int kNumLEDs = 8;

enum UiMode {
  UI_MODE_NORMAL,
  UI_MODE_DISPLAY_ALTERNATE_PARAMETERS,
  UI_MODE_DISPLAY_OCTAVE,
  UI_MODE_CALIBRATION_C1,
  UI_MODE_CALIBRATION_C3,
  UI_MODE_TEST,
  UI_MODE_ERROR,
  UI_MODE_RESTORE_STATE,
};



enum PlaitsPatched {
  TIMBRE_PATCHED,
  FM_PATCHED,
  MORPH_PATCHED,
  PATCHED_LAST
};

struct PlaitsState {

  PlaitsState() : 
  engine(0), 
  lpg_colour(0), 
  decay(128),
  octave(255),
  color_blind(0) {}


  uint8_t engine;
  uint8_t lpg_colour;
  uint8_t decay;
  uint8_t octave;
  uint8_t color_blind;
  
  /**@brief checks sameness */
    bool operator==(const PlaitsState &rhs)
    {
        
        return true;
        if(engine != rhs.engine)
        {
            return false;
        }
        else if(lpg_colour != rhs.lpg_colour)
        {
            return false;
        }
        else if(decay != rhs.decay)
        {
            return false;
        }
        else if(octave != rhs.octave)
        {
            return false;
        }
        else if(color_blind != rhs.color_blind)
        {
            return false;
        }
        
        return true;
    }

    /** @brief Not equal operator */
    bool operator!=(const PlaitsState &rhs) { return !operator==(rhs); }
};



class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(Patch* patch, Modulations* modulations, DaisyMargolis* hw);
  
  void Poll();
  
  void set_active_engine(int active_engine) {
    active_engine_ = active_engine;
  }
  void SaveCalibrationData();
  void SaveStateData();
  void RestoreState();

  bool readyToSaveState = false;
  bool readyToRestore = false;


  
 private:
  void UpdateLEDs();
  void ReadSwitches();
  void ProcessPotsHiddenParameters();
  void LoadState();
  void SaveState();
  
  void DetectNormalization();
  
  void StartCalibration();
  void CalibrateC1();
  void CalibrateC3();

  void RealignPots() {
    pots_[KNOB_3].Realign();
    pots_[KNOB_4].Realign();
    pots_[KNOB_2].Realign();
  }

  void LoadStateData();
  

  void SetStateData(PlaitsState &statedata);
  void GetStateData(PlaitsState &statedata);
  
  UiMode mode_;
  
  
  
  int ui_task_;

  float transposition_;
  float octave_;
  uint8_t cblind_;
  DaisyMargolis* hw_;
  Patch* patch_;
  Modulations* modulations_;
  //NormalizationProbe normalization_probe_;
  PotController pots_[KNOB_LAST];
  float pitch_lp_;
  float pitch_lp_calibration_;
  
  //Settings* settings_;
  

  bool isPatched[3] = {false};

  int pwm_counter_;
  int press_time_[2];
  bool ignore_release_[2];
  
  int active_engine_;
  
  float cv_c1_;  // For calibration
  float plaits_cv_scale[CV_LAST];

   
  //DISALLOW_COPY_AND_ASSIGN(Ui);
};

}  // namespace plaits

#endif  // PLAITS_UI_H_
