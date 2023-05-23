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

#include "../daisy_hank.h"
#include "daisysp.h"
#include "dsp/dsp.h"
#include "dsp/voice.h"
#include "arp.h"
#include "settings.h"
#include "ui.h"


using namespace daisy;
using namespace daisysp;
using namespace arp;

DaisyHank hw;



using namespace plaits;
using namespace stmlib;

// #define PROFILE_INTERRUPT 1

const bool test_adc_noise = false;




Modulations modulations;
Patch patch;
Settings settings;
Ui ui;
Voice voice;
Arp arpeggiator;

char shared_buffer[16384] = {};


#define BLOCK_SIZE 16
plaits::Voice::Frame outputPlaits[BLOCK_SIZE];

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {


  hw.ProcessAnalogControls();
  ui.Poll();

  
  modulations.level = 1;

  patch.frequency_modulation_amount = 1.f;
	patch.timbre_modulation_amount = 1.f;
	patch.morph_modulation_amount = 1.f;

  modulations.timbre_patched = true;
  modulations.frequency_patched = true;
  modulations.morph_patched = true;
  modulations.trigger_patched = true;
  modulations.level_patched = false;

  if(hw.gate_in1.Trig())
    {
        arpeggiator.Trig();
    }
  modulations.note = arpeggiator.GetArpNote(modulations.note);

  modulations.trigger = 5.f * (!hw.GateIn1() ? 1.f : 0.f);
  
  


  voice.Render(patch, modulations, outputPlaits, BLOCK_SIZE);

	for (size_t i = 0; i < size; i++) {
		OUT_L[i] = outputPlaits[i].out / 32768.f;
		OUT_R[i] = outputPlaits[i].aux / 32768.f;
	}
 
  //ui.set_active_engine(voice.active_engine());

}



void Init() {


  hw.Init(false);
	hw.SetAudioBlockSize(BLOCK_SIZE); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

  hw.ClearLeds();
  hw.UpdateLeds();
  
  
  stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
  voice.Init(&allocator);

  settings.Init(&hw);
  arpeggiator.Init();

  ui.Init(&patch, &modulations, &voice, &arpeggiator, &settings, &hw);
  
	

  hw.StartAdc();
	hw.StartAudio(AudioCallback);
  
}

int main(void) {
  Init();

  uint32_t last_save_time = System::GetNow(); 


  while (1) {
    
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
