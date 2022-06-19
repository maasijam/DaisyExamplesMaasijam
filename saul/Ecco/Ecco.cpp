// Copyright 2021 Adam Fulford / 2022 maasijam
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


//#include "QSPI_Settings.h"
#include "../daisy_saul.h"
#include "daisysp.h"
//#include "delayline_multitap.h" //modified delayline
//#include "delayline_reverse.h"  //reverse delayline
//#include "envFollow.h"
//#include "scale.h"
//#include "constants.h"
//#include "taptempo.h"
//#include "LEDs.h"
//#include "DelayMulti.h"
#include <math.h>
#include <string.h>
#include <atomic>




using namespace daisy;
using namespace daisysp;



enum LEDS
{
    LED_LFO1_WAVE_SQUARE,
    LED_LFO1_RATE_SLOW,
    LED_LFO1_WAVE_TRI,
    LED_LFO1_RATE_FAST,
    LED_LFO2_WAVE_TRI,
    LED_LFO2_RATE_FAST, /*--------*/
    LED_LFO2_WAVE_SQUARE,
    LED_LFO2_RATE_SLOW,
    LED_SYNC_RED, //2-1
    LED_SYNC_GREEN, //2-2
    LED_SYNC_BLUE, //2-3
    LED_CLOCK_RED, //2-4
    LED_CLOCK_GREEN, //2-5
    LED_CLOCK_BLUE, //2-6 /*--------*/
    LED_REVERSE_LEFT_RED, //2-7
    LED_REVERSE_LEFT_GREEN, //2-8
    LED_REVERSE_LEFT_BLUE, //3-1
    LED_REVERSE_RIGHT_RED, //3-2
    LED_REVERSE_RIGHT_GREEN, //3-3
    LED_REVERSE_RIGHT_BLUE, //3-4
    LED_TIME_LEFT, //3-5
    LED_TIME_RIGHT //3-6
};

enum PUSHBUTTONS
{
    BTN_LFO1_WAVE,
    BTN_LFO1_RATE,
    BTN_REVERSE,
    BTN_SYNC,
    BTN_LFO2_WAVE,
    BTN_LFO2_RATE,
    BTN_TAP
};

DaisySaul hw;
Oscillator   osc[5];

Parameter  z_freq;

float zofreq;
size_t lfo1_w;
size_t lfo2_w;
size_t lfo1_r;
size_t lfo2_r;

size_t reverseState = 0;

bool    btnPressed[hw.S_LAST];



void Update_LfoLeds();
void Update_LfoBtns();
void Update_Buttons();
void Update_Leds();


void UpdateControls()
{
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();
    Update_Buttons();
    Update_Leds();
}


void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    UpdateControls();
    

    float knob_coarse = hw.GetKnobValue(3);
    float coarse_tune = fmap(knob_coarse, 0, 15);

    float knob_fine = hw.GetKnobValue(4);
    float fine_tune = fmap(knob_fine, 0, 10);

    float cv_voct = hw.GetKnobValue(5);
    float voct    = fmap(cv_voct, 0, 60);

    /** Convert from MIDI note number to frequency */
    float midi_nn = fclamp(coarse_tune + fine_tune + voct, 0.f, 127.f);
    float freq_a  = mtof(midi_nn);

    float knob_coarseb = hw.GetKnobValue(0);
    float coarse_tuneb = fmap(knob_coarseb, 0, 15);

    float knob_fineb = hw.GetKnobValue(1);
    float fine_tuneb = fmap(knob_fineb, 0, 10);

    float cv_voctb = hw.GetKnobValue(2);
    float voctb    = fmap(cv_voctb, 0, 60);

    /** Convert from MIDI note number to frequency */
    float midi_nnb = fclamp(coarse_tuneb + fine_tuneb + voctb, 0.f, 127.f);
    float freq_b  = mtof(midi_nnb);

    float knob_coarsec = hw.GetKnobValue(8);
    float coarse_tunec = fmap(knob_coarsec, 0, 15);

    float knob_finec = hw.GetKnobValue(9);
    float fine_tunec = fmap(knob_finec, 0, 10);

    float cv_voctc = hw.GetKnobValue(10);
    float voctc    = fmap(cv_voctc, 0, 60);

    /** Convert from MIDI note number to frequency */
    float midi_nnc = fclamp(coarse_tunec + fine_tunec + voctc, 0.f, 127.f);
    float freq_c  = mtof(midi_nnc);

    float knob_coarsed = hw.GetCvValue(0);
    float coarse_tuned = fmap(knob_coarsed, 0, 15);

    float knob_fined = hw.GetCvValue(1);
    float fine_tuned = fmap(knob_fined, 0, 10);

    float cv_voctd = hw.GetCvValue(2);
    float voctd    = fmap(cv_voctd, 0, 60);

    /** Convert from MIDI note number to frequency */
    float midi_nnd = fclamp(coarse_tuned + fine_tuned + voctd, 0.f, 127.f);
    float freq_d  = mtof(midi_nnd);

    float knob_coarsee = hw.GetCvValue(8);
    float coarse_tunee = fmap(knob_coarsee, 0, 84);

    float knob_finee = hw.GetCvValue(9);
    float fine_tunee = fmap(knob_finee, 0, 10);

    float cv_vocte = hw.GetCvValue(10);
    float vocte    = fmap(cv_vocte, 0, 60);

    /** Convert from MIDI note number to frequency */
    float midi_nne = fclamp(coarse_tunee + fine_tunee + vocte, 0.f, 127.f);
    float freq_e  = mtof(midi_nne);

    
    osc[0].SetFreq(freq_a);
     osc[1].SetFreq(freq_b);
     osc[2].SetFreq(freq_c);
     osc[3].SetFreq(freq_d);
     osc[4].SetFreq(freq_e);
  
    
    for (size_t i = 0; i < size; i++)
    {

       //float sig = osc_a.Process() + osc_b.Process() + osc_c.Process() + osc_d.Process() + osc_e.Process() + osc_f.Process() + osc_g.Process() + osc_h.Process() + osc_i.Process() + osc_j.Process() + osc_k.Process();
        //float sig = osc_a.Process() + osc_b.Process();
        //out[i] = sig;
        //out[i+1] = sig;
        float mix = 0;
        //Process and output the three oscillators
        for(size_t chn = 0; chn < 5; chn++)
        {
            float sig = osc[chn].Process();
            mix += sig * .25f;
           //out[chn][i] = sig;
        }
        //output the mixed oscillators
        out[0][i] = out[1][i] = mix;
    }
}

void SetupOsc(float samplerate)
{
    for(int i = 0; i < 5; i++)
    {
        osc[i].Init(samplerate);
        osc[i].SetAmp(.7);
    }
}


int main(void)
{
    float samplerate;
    
    hw.Init();
    //hw.SetAudioBlockSize(4);
    samplerate = hw.AudioSampleRate();
    //hw.SetAudioBlockSize(1);     //set blocksize.

    SetupOsc(samplerate);

        
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    lfo1_w = 1;
    lfo2_w = 1;
    lfo1_r = 0;
    lfo2_r = 0;

    for (size_t i = 0; i < hw.S_LAST; i ++)
    {
        btnPressed[i] = false;
    }
    


    while (1)
    {
        //Update_LfoLeds();
        //Update_LfoBtns();
        //hw.UpdateExample();
    }
}

void Update_LfoLeds()
{
    
}

void Update_LfoBtns()
{
    
}

void Update_Buttons()
{
        
    for (size_t i = 0; i < hw.S_LAST-1; i ++)
    {
        
        if(hw.s[i].RisingEdge())
        {
            btnPressed[i] = !btnPressed[i];
        }
    }
    if(hw.s[BTN_REVERSE].RisingEdge()){
        reverseState += 1;
        if(reverseState > 3) {
           reverseState = 0;     
        } 
    }

}

void Update_Leds()
{
    
    
    if(btnPressed[BTN_LFO1_WAVE]) {
        hw.SetLed(LED_LFO1_WAVE_SQUARE,false);
        hw.SetLed(LED_LFO1_WAVE_TRI,true);
    } else {
        hw.SetLed(LED_LFO1_WAVE_SQUARE,true);
        hw.SetLed(LED_LFO1_WAVE_TRI,false);
    }

    if(btnPressed[BTN_LFO1_RATE]) {
        hw.SetLed(LED_LFO1_RATE_SLOW,false);
        hw.SetLed(LED_LFO1_RATE_FAST,true);
    } else {
        hw.SetLed(LED_LFO1_RATE_SLOW,true);
        hw.SetLed(LED_LFO1_RATE_FAST,false);
    }

    if(btnPressed[BTN_LFO2_WAVE]) {
        hw.SetLed(LED_LFO2_WAVE_SQUARE,false);
        hw.SetLed(LED_LFO2_WAVE_TRI,true);
    } else {
        hw.SetLed(LED_LFO2_WAVE_SQUARE,true);
        hw.SetLed(LED_LFO2_WAVE_TRI,false);
    }

    if(btnPressed[BTN_LFO2_RATE]) {
        hw.SetLed(LED_LFO2_RATE_SLOW,false);
        hw.SetLed(LED_LFO2_RATE_FAST,true);
    } else {
        hw.SetLed(LED_LFO2_RATE_SLOW,true);
        hw.SetLed(LED_LFO2_RATE_FAST,false);
    }

    if(btnPressed[BTN_SYNC]) {
        hw.SetLed(LED_SYNC_GREEN,false);
    } else {
        hw.SetLed(LED_SYNC_GREEN,true);
    }

    switch (reverseState)
    {
    case 1:
        hw.SetLed(LED_REVERSE_LEFT_BLUE,false);
        hw.SetLed(LED_REVERSE_RIGHT_BLUE,true);
        break;
    case 2:
        hw.SetLed(LED_REVERSE_LEFT_BLUE,true);
        hw.SetLed(LED_REVERSE_RIGHT_BLUE,false);
        break;
    case 3:
        hw.SetLed(LED_REVERSE_LEFT_BLUE,false);
        hw.SetLed(LED_REVERSE_RIGHT_BLUE,false);
        break;
    default:
        hw.SetLed(LED_REVERSE_LEFT_BLUE,true);
        hw.SetLed(LED_REVERSE_RIGHT_BLUE,true);
        break;
    }

    if(hw.s[BTN_TAP].Pressed()){
        hw.SetLed(LED_CLOCK_RED,false);
    } else {
        hw.SetLed(LED_CLOCK_RED,true);
    }

    hw.SetLed(LED_TIME_LEFT,((hw.sw[0].Read() == 1 || hw.sw[0].Read() == 2) ? false : true));
    hw.SetLed(LED_TIME_RIGHT,((hw.sw[1].Read() == 1 || hw.sw[1].Read() == 2) ? false : true));
   
       
}

