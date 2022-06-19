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
    LED_LFO1_WAVE_TRI,
    LED_LFO1_RATE_SLOW,
    LED_LFO1_RATE_FAST,
    LED_LFO2_WAVE_SQUARE,
    LED_LFO2_WAVE_TRI,
    LED_LFO2_RATE_SLOW,
    LED_LFO2_RATE_FAST, /*--------*/
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
    BTN_SYNC,
    BTN_REVERSE,
    BTN_LFO2_WAVE,
    BTN_LFO2_RATE,
    BTN_TAP
};

DaisySaul hw;


AnalogBassDrum bd;
Switch        tapbtn;
Metro tick;
Overdrive ov;
WhiteNoise wn;
AdEnv env;

void AudioCallback(AudioHandle::InterleavingInputBuffer in, AudioHandle::InterleavingOutputBuffer out, size_t size)
{
    
    

    float sig,ovsig,noise,envSig,noiseSig;

    //float *out_left, *out_right;
    //out_left  = out[0];
    //out_right = out[1];

// Handle Triggering the Plucks
   
    
        
     
  
    
    for(size_t i = 0; i < size; i += 2)
    {

       bool t = tick.Process();
      if (t) {
      bd.SetTone(hw.GetKnobValue(DaisySaul::KNOB_0));
      bd.SetDecay(100.f);
      bd.SetSelfFmAmount(hw.GetKnobValue(DaisySaul::KNOB_1));
      bd.SetAttackFmAmount(hw.GetKnobValue(DaisySaul::KNOB_2));
      ov.SetDrive(hw.GetKnobValue(DaisySaul::KNOB_3));
      env.Trigger();
    }

        noise = wn.Process();
        envSig = env.Process();
        noiseSig = noise * envSig * .5f;
        sig = bd.Process(t);
        sig = sig + noiseSig;
       ovsig = ov.Process(sig);
        out[i] = ovsig;
            out[i+1] = ovsig;
    }
}



int main(void)
{
    float samplerate;
    
    hw.Init();
    //hw.SetAudioBlockSize(4);
    samplerate = hw.AudioSampleRate();
    //hw.SetAudioBlockSize(1);     //set blocksize.
    tapbtn.Init(hw.seed.GetPin(30),0.0F,Switch::TYPE_MOMENTARY,Switch::POLARITY_NORMAL,Switch::PULL_UP);
    bd.Init(samplerate);
 bd.SetFreq(50.f);

      tick.Init(2.f, samplerate);  

      ov.Init();
      wn.Init();
      env.Init(samplerate);
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);


    while (1) {
    
        hw.ProcessAnalogControls(); // Normalize CV inputs
        //hw.UpdateExample();

        //float accent = 0.5;
        
        float fm,decay,tone;
        //bd.SetAccent(accent);
       
        //fm = (hw.GetKnobValue(DaisyVersio::KNOB_0) * 200.f);
        //bd.SetFreq(fm);

    // Read knobs for tone;
    //tone = hw.GetKnobValue(DaisyVersio::KNOB_1) *0.2 + 0.5;
    //bd.SetTone(tone);

    //decay = hw.GetKnobValue(DaisyVersio::KNOB_0) * 10.0f;
    //bd.SetDecay(decay);
    tapbtn.Debounce();
    if(tapbtn.FallingEdge() || hw.Gate()) {
           
            bd.Trig();
    }
    }
}