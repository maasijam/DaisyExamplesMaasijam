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
bool saveSt{false};

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
} RingoSetting;

void FlashLoad(uint8_t aSlot);
void FlashSave(uint8_t aSlot);
void FlashErase(uint8_t aSlot);
void FlashToSaul(RingoSetting *);
void SaulToFlash(RingoSetting *);

RingoSetting default_preset[PRESET_MAX] = {
{3, 0, 0, 0, 0,true}
};

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
    hw.ProcessAnalogControls();
    
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

    //FlashErase(0);
    FlashLoad(0);
    if(!flashloaded_) {
    //    hw.SetRGBLed(3,DaisySaul::yellow);
        FlashToSaul(&default_preset[0]);
    }


    hw.StartAdc();
    hw.StartAudio(AudioCallback);

//hw.seed.StartLog(false);
    while(1)
    {
        hw.ProcessDigitalControls();
        Update_Buttons();
        Update_Leds();
        if(saveSt)
        {
            FlashSave(0);
        }
    }
}

void Update_Buttons()
{  
    static uint32_t shiftTime{};

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

    if (hw.s[BTN_TAP].FallingEdge())    //when button is let go shift is off
    {
        saveSt = false;
    }

    if (hw.s[BTN_TAP].Pressed())
    {
        if ( (System::GetNow() - shiftTime) > shiftWait)
        {
            saveSt = true;
        } 
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

    if(saveSt) {
            hw.SetRGBLed(1,DaisySaul::red);
            hw.SetRGBLed(2,DaisySaul::red);
            hw.SetRGBLed(3,DaisySaul::red);
            hw.SetRGBLed(4,DaisySaul::red);
    } else {
        hw.SetRGBLed(1,DaisySaul::off);
        hw.SetRGBLed(2,DaisySaul::off);
        hw.SetRGBLed(3,DaisySaul::off);
        hw.SetRGBLed(4,DaisySaul::off);
    }
    
}

// Flash handling - load and save
// 8MB of flash
// 4kB blocks
// assume our settings < 4kB, so put one patch per block
#define FLASH_BLOCK 4096

uint8_t DSY_QSPI_BSS qspi_buffer[FLASH_BLOCK * 16];

void FlashLoad(uint8_t aSlot)
{
	RingoSetting saulLoad;
    size_t size = sizeof(RingoSetting);
	memcpy(&saulLoad, &qspi_buffer[aSlot * FLASH_BLOCK], size);
	FlashToSaul(&saulLoad);
}



void FlashSave(uint8_t aSlot)
{
	RingoSetting saulSave;
	SaulToFlash(&saulSave);
	size_t start_address = (size_t)qspi_buffer;
    size_t size = sizeof(RingoSetting);
	size_t slot_address = start_address + (aSlot * FLASH_BLOCK);
    hw.seed.qspi.Erase(slot_address, slot_address + size);
    hw.seed.qspi.Write(slot_address, size, (uint8_t*)&saulSave);
}

void FlashErase(uint8_t aSlot)
{
	size_t start_address = (size_t)qspi_buffer;
    size_t size = sizeof(RingoSetting);
	size_t slot_address = start_address + (aSlot * FLASH_BLOCK);
    hw.seed.qspi.Erase(slot_address, slot_address + size);
}



void FlashToSaul(RingoSetting *rs)
{
	noteStrumState = rs->NoteStrum ;
    exciterState = rs->Exciter;
    polyState = rs->Poly;
    eggFxState = rs->EggFx;
    modelState = rs->Model;
    flashloaded_ = rs->flashloaded;
}

void SaulToFlash(RingoSetting *rs)
{
	rs->NoteStrum = noteStrumState;
    rs->Exciter = exciterState;
    rs->Poly = polyState;
    rs->EggFx = eggFxState;
    rs->Model = modelState;
    rs->flashloaded = true;
}