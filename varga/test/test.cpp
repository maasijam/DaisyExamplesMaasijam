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
#include "../daisy_varga.h"
#include "daisysp.h"




using namespace daisy;
using namespace daisysp;


DaisyVarga hw;


Oscillator osc1,osc2;

void ledAni();
void updateButton();


void AudioCallback(AudioHandle::InterleavingInputBuffer in, AudioHandle::InterleavingOutputBuffer out, size_t size)
{
    
    

    float osc1_out,osc2_out;

    osc1.SetFreq(mtof(hw.GetKnobValue(hw.KNOB_0) * 127));
    osc1.SetAmp(hw.GetKnobValue(hw.KNOB_1));
    osc2.SetFreq(mtof(hw.GetKnobValue(hw.KNOB_2) * 127));
    osc2.SetAmp(hw.GetKnobValue(hw.KNOB_3));
    
    //osc1.SetFreq(mtof(hw.GetCvValue(hw.CV_4) * 127));    
    //osc2.SetFreq(mtof(hw.GetCvValue(hw.CV_5) * 127));    
  
    
    for(size_t i = 0; i < size; i += 2)
    {

        osc1_out = osc1.Process();
        osc2_out = osc2.Process();
        out[i] = osc1_out;
        out[i+1] = osc2_out;
    }
}



int main(void)
{
    float samplerate;
    
    hw.Init();
    hw.SetAudioBlockSize(4);
    samplerate = hw.AudioSampleRate();
    //hw.SetAudioBlockSize(1);     //set blocksize.
    //Set up oscillator
    osc1.Init(samplerate);
    osc1.SetWaveform(osc1.WAVE_POLYBLEP_SAW);
    osc1.SetAmp(1.f);
    osc1.SetFreq(1000);
    osc2.Init(samplerate);
    osc2.SetWaveform(osc2.WAVE_POLYBLEP_SQUARE);
    osc2.SetAmp(1.f);
    osc2.SetFreq(1000);
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    ledAni();
    while (1) {
    
        hw.ProcessAllControls(); // Normalize CV inputs
        updateButton();
    }
}

void ledAni() {
    hw.SetRGBColor(0,hw.red);
    hw.UpdateLeds();
    hw.DelayMs(500);
    hw.SetRGBColor(0,hw.green);
    hw.UpdateLeds();
    hw.DelayMs(500);
    hw.SetRGBColor(0,hw.blue);
    hw.UpdateLeds();
    hw.DelayMs(500);
    hw.SetRGBColor(1,hw.red);
    hw.UpdateLeds();
    hw.DelayMs(500);
    hw.SetRGBColor(1,hw.green);
    hw.UpdateLeds();
    hw.DelayMs(500);
    hw.SetRGBColor(1,hw.blue);
    hw.UpdateLeds();
    hw.DelayMs(500);

}

void updateButton() {
    if(hw.s[hw.S_0].RisingEdge()) {
        hw.SetRGBColor(0,hw.yellow);
        hw.UpdateLeds();
    }

    if(hw.s[hw.S_1].RisingEdge()) {
        hw.SetRGBColor(1,hw.yellow);
        hw.UpdateLeds();
    }

    if(hw.Gate()) {
        hw.SetRGBColor(0,hw.purple);
        hw.UpdateLeds();
    } else {
        hw.SetRGBColor(0,hw.off);
        hw.UpdateLeds();
    }
}