// Copyright 2015 Emilie Gillet.
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

#include "constants.h"
#include "../daisy_saul.h"
#include "dsp/part.h"
#include "dsp/strummer.h"
#include "dsp/string_synth_part.h"
#include "cv_scaler.h"
#include "settings.h"
#include "ui.h"

// #define PROFILE_INTERRUPT 1

using namespace rinx;
using namespace stmlib;
using namespace daisy;

uint16_t  DSY_SDRAM_BSS reverb_buffer[32768];

DaisySaul hw;
CvScaler cv_scaler;
Part part;
Settings settings;
StringSynthPart string_synth;
Strummer strummer;
Ui ui;




void ProcessControls(Patch* patch, PerformanceState* state)
{
    //strum
    state->internal_strum = true;
    state->strum = hw.gate.Trig();
}

float input[kMaxBlockSize];
float output[kMaxBlockSize];
float aux[kMaxBlockSize];

const float kNoiseGateThreshold = 0.00003f;
float       in_level            = 0.0f;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAnalogControls();
    //hw.ProcessDigitalControls();
    //Update_Buttons();
    ui.Poll();
    
    PerformanceState performance_state;
    Patch            patch;

    //cv_scaler.DetectAudioNormalization(in, size);
    //cv_scaler.Read(&patch, &performance_state);

    ProcessControls(&patch, &performance_state);
    cv_scaler.Read(&patch, &performance_state);

    if(settings.state().easter_egg)
    {
        for(size_t i = 0; i < size; ++i)
        {
            input[i] = in[0][i];
        }
        strummer.Process(NULL, size, &performance_state);
        string_synth.Process(
            performance_state, patch, input, output, aux, size);
    }
    else
    {
        // Apply noise gate.
        for(size_t i = 0; i < size; i++)
        {
            float in_sample = in[0][i];
            float error, gain;
            error = in_sample * in_sample - in_level;
            in_level += error * (error > 0.0f ? 0.1f : 0.0001f);
            gain = in_level <= kNoiseGateThreshold
                       ? (1.0f / kNoiseGateThreshold) * in_level
                       : 1.0f;
            input[i] = gain * in_sample;
        }
        
        strummer.Process(input, size, &performance_state);
        part.Process(performance_state, patch, input, output, aux, size);
    }

    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = output[i];
        out[1][i] = aux[i];
    }
    ui.set_strumming_flag(performance_state.strum);
}

void Init() {
 
  hw.Init();
  float samplerate = hw.AudioSampleRate();
  float blocksize  = hw.AudioBlockSize();

    //InitResources();

    strummer.Init(0.01f, samplerate / blocksize);
    part.Init(reverb_buffer);
    string_synth.Init(reverb_buffer);

    settings.Init(&hw);
    //cv_scaler.Init(settings.mutable_calibration_data());
    ui.Init(&settings, &cv_scaler, &part, &string_synth, &hw);

    

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
}

int main(void) {
  Init();
  while (1) {
    ui.DoEvents();
  }
}

