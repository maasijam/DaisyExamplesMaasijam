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

#include "../daisy_margolis.h"
#include "daisysp.h"
#include "dsp/dsp.h"
#include "dsp/voice.h"
#include "settings.h"
#include "ui.h"


using namespace daisy;
using namespace daisysp;
using namespace margolis;

DaisyMargolis hw;



using namespace plaits;
using namespace stmlib;

// #define PROFILE_INTERRUPT 1

const bool test_adc_noise = false;




Modulations modulations;
Patch patch;
Settings settings;
Ui ui;
Voice voice;



char shared_buffer[16384] = {};

#define BLOCK_SIZE 16
plaits::Voice::Frame outputPlaits[BLOCK_SIZE];

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {

  ui.Poll();
  
  
  modulations.frequency = hw.GetCvValue(CV_3);
	modulations.harmonics = hw.GetCvValue(CV_5);
	modulations.timbre = hw.GetCvValue(CV_2);
	modulations.morph = hw.GetCvValue(CV_4);
	modulations.engine = hw.GetCvValue(CV_1);


  voice.Render(patch, modulations, outputPlaits, BLOCK_SIZE);

	for (size_t i = 0; i < size; i++) {
		OUT_L[i] = outputPlaits[i].out / 32768.f;
		OUT_R[i] = outputPlaits[i].aux / 32768.f;
	}
  ui.set_active_engine(voice.active_engine());
  
  
  
}



void Init() {

  hw.Init();
	hw.SetAudioBlockSize(BLOCK_SIZE); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

  hw.ClearLeds();
	hw.UpdateLeds();

  stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
	voice.Init(&allocator);
    
  
  volatile size_t counter = 1000000;
  while (counter--);
  
  
  settings.Init(&hw);
  
  ui.Init(&patch, &modulations, &settings, &hw);
  hw.StartAdc();
	hw.StartAudio(AudioCallback);
  
}

int main(void) {
  //hw.seed.StartLog(false);
  Init();
  //CvIns mycvin;
  //float  cvval;
  uint32_t last_save_time = System::GetNow(); 

  while (1) {
    //cvval = modulations.trigger_patched ? 1.0f : 0.f;
    /*mycvin = CV_VOCT;
        if(hw.cv[mycvin].Value() >= 0.f) {
            //cvval = hw.cv[mycvin].Value();
            cvval = hw.cv[mycvin].Value();
        } else {
            //cvval = hw.cv[mycvin].Value() * -1;
            cvval = hw.cv[mycvin].Value() * -1;
        }
        hw.seed.dac.WriteValue(
            DacHandle::Channel::ONE,
            uint16_t(cvval * 1024.f));*/
        if (hw.ReadyToSaveCal()) {
            ui.SaveCalibrationData();
        }
        if (System::GetNow() - last_save_time > 100 && ui.readyToSaveState)
        {
          ui.SaveState();
          last_save_time = System::GetNow();
          ui.readyToSaveState = false;
        }
        if (ui.readyToRestore) {
            /** Collect the data from where ever in the application it is */
            settings.RestoreState();
            ui.readyToRestore = false;
        }
  }
  
}
