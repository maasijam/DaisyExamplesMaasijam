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
#include "dsp/part.h"
#include "dsp/strummer.h"
#include "dsp/string_synth_part.h"
#include "cv_scaler.h"
#include "../daisy_saul.h"


using namespace daisy;
using namespace torus;

DaisySaul         hw;

uint16_t reverb_buffer[32768];

CvScaler        cv_scaler;
Part            part;
StringSynthPart string_synth;
Strummer        strummer;


void Update_Buttons();
void Update_Leds();

size_t noteStrumState = 0;
size_t exciterState = 0;
size_t polyState = 0;
size_t eggFxState = 0;
size_t modelState = 0;

// norm edit menu items
bool exciterIn;
bool strumIn;
bool noteIn;

//easter egg toggle
bool easterEggOn;

int oldModel = 0;
int old_poly = 0;

void ProcessControls(Patch* patch, PerformanceState* state)
{

    //polyphony setting
    int poly = polyState;
    if(old_poly != poly)
    {
        part.set_polyphony(0x01 << poly);
        string_synth.set_polyphony(0x01 << poly);
    }
    old_poly = poly;

    //model settings
    part.set_model((torus::ResonatorModel)modelState);
    string_synth.set_fx((torus::FxType)eggFxState);

    // normalization settings
    state->internal_note    = noteStrumState == 1 || noteStrumState == 3 ? false : true;
    state->internal_exciter = exciterState == 1 ? false : true;
    state->internal_strum   = noteStrumState == 2 || noteStrumState == 3 ? false : true;

    //strum
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
    hw.ProcessAllControls();

    PerformanceState performance_state;
    Patch            patch;

    ProcessControls(&patch, &performance_state);
    cv_scaler.Read(&patch, &performance_state);

    if(easterEggOn)
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
}

int main(void)
{
    hw.Init();
    float samplerate = hw.AudioSampleRate();
    float blocksize  = hw.AudioBlockSize();

    InitResources();

    strummer.Init(0.01f, samplerate / blocksize);
    part.Init(reverb_buffer);
    string_synth.Init(reverb_buffer);

    cv_scaler.Init();


    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1)
    {
       hw.ProcessDigitalControls();
       Update_Buttons();
       Update_Leds();
    }
}

void Update_Buttons()
{  
    
    if(hw.s[BTN_NOTE_STRUM].RisingEdge()){
        noteStrumState += 1;
        if(noteStrumState > 3) {
           noteStrumState = 0;     
        } 
    }
    if(hw.s[BTN_EXCITER].RisingEdge()){
        exciterState += 1;
        if(exciterState > 1) {
           exciterState = 0;     
        } 
    }
    if(hw.s[BTN_POLY].RisingEdge()){
        polyState += 1;
        if(polyState > 2) {
           polyState = 0;     
        } 
    }
    if(hw.s[BTN_EGG_FX].RisingEdge()){
        eggFxState += 1;
        if(eggFxState > 5) {
           eggFxState = 0;     
        } 
    }
    if(hw.s[BTN_MODEL].RisingEdge()){
        modelState += 1;
        if(modelState > 5) {
           modelState = 0;     
        } 
    }

    if(hw.sw[0].Read() == 1){
        easterEggOn = true;
    } else {
        easterEggOn = false;
    }
    
}

void Update_Leds()
{

    switch (noteStrumState)
    {
    case 1:
        hw.SetLed(LED_NOTE,false);
        hw.SetLed(LED_STRUM,true);
        break;
    case 2:
        hw.SetLed(LED_NOTE,true);
        hw.SetLed(LED_STRUM,false);
        break;
    case 3:
        hw.SetLed(LED_NOTE,false);
        hw.SetLed(LED_STRUM,false);
        break;
    default:
        hw.SetLed(LED_NOTE,true);
        hw.SetLed(LED_STRUM,true);
        break;
    }

    hw.SetLed(LED_EXCITER,!exciterState);

    switch (polyState)
    {
    case 1:
        hw.SetLed(LED_POLY2,false);
        hw.SetLed(LED_POLY4,true);
        break;
    case 2:
        hw.SetLed(LED_POLY2,true);
        hw.SetLed(LED_POLY4,false);
        break;
    case 3:
        hw.SetLed(LED_POLY2,false);
        hw.SetLed(LED_POLY4,false);
        break;
    default:
        hw.SetLed(LED_POLY2,true);
        hw.SetLed(LED_POLY4,true);
        break;
    }

    hw.SetRGBLed(1,eggFxState);
    hw.SetRGBLed(4,modelState);
    
}
