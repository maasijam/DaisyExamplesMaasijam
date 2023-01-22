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

#include "dsp/part.h"
#include "dsp/strummer.h"
#include "dsp/string_synth_part.h"
#include "cv_scaler.h"
#include "../daisy_white.h"

using namespace daisy;
using namespace torus;
using namespace white;


DaisyWhite         hw;

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
bool saveState{false};



// norm edit menu items
bool exciterIn;
bool strumIn;
bool noteIn;

//easter egg toggle
bool easterEggOn;

int oldModel = 0;
int old_poly = 0;

#define PRESET_MAX 1
bool flashloaded_ = false;

typedef struct
{
   	uint8_t NoteStrum;
	uint8_t Exciter;
	uint8_t Poly;
	uint8_t EggFx;
	uint8_t Model;
    bool flashloaded;	
} TorusSetting;

void FlashLoad(uint8_t aSlot);
void FlashSave(uint8_t aSlot);
void FlashErase(uint8_t aSlot);
void FlashToWhite(TorusSetting *);
void WhiteToFlash(TorusSetting *);

TorusSetting default_preset[PRESET_MAX] = {
{3, 0, 0, 0, 0,true}
};



void ProcessControls(Patch* patch, PerformanceState* state)
{
    // control settings
    //cv_scaler.channel_map_[0] = controlListValueOne.GetIndex();
    //cv_scaler.channel_map_[1] = controlListValueTwo.GetIndex();
    //cv_scaler.channel_map_[2] = controlListValueThree.GetIndex();
    //cv_scaler.channel_map_[3] = controlListValueFour.GetIndex();

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
    state->strum = hw.gate_in1.Trig();
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
    Update_Buttons();

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

    //InitUi();
    //InitUiPages();
    InitResources();
    //ui.OpenPage(mainMenu);

    strummer.Init(0.01f, samplerate / blocksize);
    part.Init(reverb_buffer);
    string_synth.Init(reverb_buffer);

    cv_scaler.Init();

    FlashLoad(1);
    if(!flashloaded_) {
    //    hw.SetRGBLed(3,DaisySaul::yellow);
        FlashToWhite(&default_preset[0]);
    }

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1)
    {
        Update_Leds();
        if(saveState)
        {
            FlashSave(1);
            saveState = false;
        }
    }
}

constexpr uint32_t shiftWait{1000};

void Update_Buttons()
{  
    static uint32_t shiftTime{};

    


    if(hw.SwitchRisingEdge(S2)){
        noteStrumState += 1;
        if(noteStrumState > 3) {
           noteStrumState = 0;     
        }
        saveState = true; 
    }
    if(hw.SwitchRisingEdge(S3)){
        exciterState += 1;
        if(exciterState > 1) {
           exciterState = 0;     
        } 
        saveState = true; 
    }
    if(hw.SwitchRisingEdge(S6)){
        polyState += 1;
        if(polyState > 2) {
           polyState = 0;     
        } 
        saveState = true; 
    }
    if(hw.SwitchRisingEdge(S4)){
        eggFxState += 1;
        if(eggFxState > 5) {
           eggFxState = 0;     
        } 
        saveState = true; 
    }
    if(hw.SwitchRisingEdge(S5)){
        modelState += 1;
        if(modelState > 5) {
           modelState = 0;     
        } 
        saveState = true; 
    }

    if(!hw.SwitchState(S0A)){
        easterEggOn = true;
    } else {
        easterEggOn = false;
    }
/*
    if (hw.SwitchRisingEdge(S8))    
    {
        shiftTime = System::GetNow();   //reset shift timer
    }

    if (hw.SwitchFallingEdge(S8))    //when button is let go shift is off
    {
        saveState = false;
        hw.SetRGBColor(RGB_LED_1,OFF);
        hw.SetRGBColor(RGB_LED_2,OFF);
        hw.SetRGBColor(RGB_LED_3,OFF);
        hw.SetRGBColor(RGB_LED_4,OFF);
    }

    if (!hw.SwitchState(S8))
    {
        if ( (System::GetNow() - shiftTime) > shiftWait)
        {
            saveState = true;
        } 
    }
*/    
}

void Update_Leds()
{
    hw.ClearLeds();
    switch (noteStrumState)
    {
    case 1:
        hw.SetGreenLeds(GREEN_LED_3,1);
        hw.SetGreenLeds(GREEN_LED_4,0);
        break;
    case 2:
        hw.SetGreenLeds(GREEN_LED_3,0);
        hw.SetGreenLeds(GREEN_LED_4,1);
        break;
    case 3:
        hw.SetGreenLeds(GREEN_LED_3,1);
        hw.SetGreenLeds(GREEN_LED_4,1);
        break;
    default:
        hw.SetGreenLeds(GREEN_LED_3,0);
        hw.SetGreenLeds(GREEN_LED_4,0);
        break;
    }

    hw.SetGreenDirectLeds(GREEN_D_LED_1,exciterState ? 1 : 0);

    switch (polyState)
    {
    case 1:
        hw.SetGreenDirectLeds(GREEN_D_LED_4,1);
        hw.SetGreenDirectLeds(GREEN_D_LED_3,0);
        break;
    case 2:
        hw.SetGreenDirectLeds(GREEN_D_LED_4,0);
        hw.SetGreenDirectLeds(GREEN_D_LED_3,1);
        break;
    case 3:
        hw.SetGreenDirectLeds(GREEN_D_LED_4,1);
        hw.SetGreenDirectLeds(GREEN_D_LED_3,1);
        break;
    default:
        hw.SetGreenDirectLeds(GREEN_D_LED_4,0);
        hw.SetGreenDirectLeds(GREEN_D_LED_3,0);
        break;
    }

    hw.SetRGBColor(RGB_LED_1,(Colors)eggFxState);
    hw.SetRGBColor(RGB_LED_4,(Colors)modelState);

    if(saveState) {
            hw.SetRGBColor(RGB_LED_1,RED);
            hw.SetRGBColor(RGB_LED_2,RED);
            hw.SetRGBColor(RGB_LED_3,RED);
            hw.SetRGBColor(RGB_LED_4,RED);
    } 
    hw.UpdateLeds();
    
}

// Flash handling - load and save
// 8MB of flash
// 4kB blocks
// assume our settings < 4kB, so put one patch per block
#define FLASH_BLOCK 4096

uint8_t DSY_QSPI_BSS qspi_buffer[FLASH_BLOCK * 16];

void FlashLoad(uint8_t aSlot)
{
	TorusSetting whiteLoad;
    size_t size = sizeof(TorusSetting);
	memcpy(&whiteLoad, &qspi_buffer[aSlot * FLASH_BLOCK], size);
	FlashToWhite(&whiteLoad);
}



void FlashSave(uint8_t aSlot)
{
	TorusSetting whiteSave;
	WhiteToFlash(&whiteSave);
	size_t start_address = (size_t)qspi_buffer;
    size_t size = sizeof(TorusSetting);
	size_t slot_address = start_address + (aSlot * FLASH_BLOCK);
    hw.seed.qspi.Erase(slot_address, slot_address + size);
    hw.seed.qspi.Write(slot_address, size, (uint8_t*)&whiteSave);
}

void FlashErase(uint8_t aSlot)
{
	size_t start_address = (size_t)qspi_buffer;
    size_t size = sizeof(TorusSetting);
	size_t slot_address = start_address + (aSlot * FLASH_BLOCK);
    hw.seed.qspi.Erase(slot_address, slot_address + size);
}



void FlashToWhite(TorusSetting *rs)
{
	noteStrumState = rs->NoteStrum ;
    exciterState = rs->Exciter;
    polyState = rs->Poly;
    eggFxState = rs->EggFx;
    modelState = rs->Model;
    flashloaded_ = rs->flashloaded;
}

void WhiteToFlash(TorusSetting *rs)
{
	rs->NoteStrum = noteStrumState;
    rs->Exciter = exciterState;
    rs->Poly = polyState;
    rs->EggFx = eggFxState;
    rs->Model = modelState;
    rs->flashloaded = true;
}