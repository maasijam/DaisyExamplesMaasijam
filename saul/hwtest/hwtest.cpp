#include "../daisy_saul.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace saul;


DaisySaul hw;
Oscillator osc[4];
Svf filter;

bool rgbOnoff[4];
int swLeds[4];
float baseFreq = 100.f;




void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    float sig, vol;
    
    
    hw.ProcessAnalogControls();
    
    if(hw.gate.State()) {
        hw.SetRGBLed(2,RED);
    } else {
        hw.SetRGBLed(2,OFF);
    }

    

    vol = hw.CVKnobCombo(hw.cv[CV_10].Value(),hw.knob[KNOB_10].Value());

    osc[0].SetAmp(hw.knob[KNOB_4].Value() * vol);
    osc[1].SetAmp(hw.knob[KNOB_5].Value() * vol);
    osc[2].SetAmp(hw.knob[KNOB_6].Value() * vol);
    osc[3].SetAmp(hw.knob[KNOB_7].Value() * vol);

    
    for (size_t i = 0; i < 4; i++)
    {
        float coarse_knob = hw.GetKnobValue(i);
        float coarse      = fmap(coarse_knob, 36.f, 96.f);

        //float voct_cv = patch.GetAdcValue(CV_5);
        //float voct    = fmap(voct_cv, 0.f, 60.f);

        float midi_nn = fclamp(coarse, 0.f, 127.f);
        float freq    = mtof(midi_nn);
        osc[i].SetFreq(freq);
    }
    
    filter.SetFreq(hw.knob[KNOB_8].Value() * 10000.f +200.f);   
    filter.SetRes(hw.knob[KNOB_9].Value());   
    
    
    for(size_t i = 0; i < size; i ++)
    {
        
        sig = osc[0].Process() + osc[1].Process() + osc[2].Process() + osc[3].Process();
        filter.Process(sig);
        
        out[0][i] = filter.Low();
        out[1][i] = filter.Low();
    }
}

void updateDigital() {
    hw.ProcessDigitalControls();
    
    if(hw.s[S_6].Pressed()) {
        hw.SetRGBLed(1,GREEN);
    } else {
        hw.SetRGBLed(1,OFF);
    }

    if(hw.s[S_2].RisingEdge()) {
        rgbOnoff[2] = !rgbOnoff[2];
    }
    hw.SetRGBLed(3,rgbOnoff[2] ? YELLOW : OFF);

    if(hw.s[S_3].RisingEdge()) {
        rgbOnoff[3] = !rgbOnoff[3];
    }
    hw.SetRGBLed(4,rgbOnoff[3] ? PURPLE : OFF);

    if(hw.s[S_0].RisingEdge()) {
        swLeds[0]++;
        if(swLeds[0] == 4) {
            swLeds[0] = 0;
        }
    }
    hw.SetLed(LED_0,swLeds[0] == 1 || swLeds[0] == 3 ? false : true);
    hw.SetLed(LED_2,swLeds[0] == 2 || swLeds[0] == 3 ? false : true);

    if(hw.s[S_1].RisingEdge()) {
        swLeds[1]++;
        if(swLeds[1] == 4) {
            swLeds[1] = 0;
        }
    }
    hw.SetLed(LED_1,swLeds[1] == 1 || swLeds[1] == 3 ? false : true);
    hw.SetLed(LED_3,swLeds[1] == 2 || swLeds[1] == 3 ? false : true);

    if(hw.s[S_4].RisingEdge()) {
        swLeds[2]++;
        if(swLeds[2] == 4) {
            swLeds[2] = 0;
        }
    }
    hw.SetLed(LED_6,swLeds[2] == 1 || swLeds[2] == 3 ? false : true);
    hw.SetLed(LED_4,swLeds[2] == 2 || swLeds[2] == 3 ? false : true);

    if(hw.s[S_5].RisingEdge()) {
        swLeds[3]++;
        if(swLeds[3] == 4) {
            swLeds[3] = 0;
        }
    }
    hw.SetLed(LED_7,swLeds[3] == 1 || swLeds[3] == 3 ? false : true);
    hw.SetLed(LED_5,swLeds[3] == 2 || swLeds[3] == 3 ? false : true);

    hw.SetLed(LED_20,hw.sw[SW_0].Read() == 1 ? false : true);
    hw.SetRGBLed(1,hw.sw[SW_0].Read() == 2 ? AQUA : (hw.sw[SW_0].Read() == 0 ? BLUE : OFF));

    hw.SetLed(LED_21,hw.sw[SW_1].Read() == 1 ? false : true);
    hw.SetRGBLed(2,hw.sw[SW_1].Read() == 2 ? AQUA : (hw.sw[SW_1].Read() == 0 ? BLUE : OFF));

}


int main(void)
{
    float samplerate;
    
    
    hw.Init();
    //hw.SetAudioBlockSize(4);
    samplerate = hw.AudioSampleRate();
    
    for (size_t i = 0; i < 4; i++)
    {
        osc[i].Init(samplerate);
        osc[i].SetWaveform(osc->WAVE_SAW);
        osc[i].SetAmp(0.25f);
        
    }
    osc[0].SetFreq(baseFreq);
    osc[1].SetFreq(baseFreq*2);
    osc[2].SetFreq(baseFreq*4);
    osc[3].SetFreq(baseFreq*6);
    
    
    for (size_t i = 0; i < 4; i++)
    {
        rgbOnoff[i] = false;
    }
    for (size_t i = 0; i < 4; i++)
    {
        swLeds[i] = 0;
    }

    filter.Init(samplerate);
    filter.SetFreq(500.0);
    filter.SetRes(0.85);
    filter.SetDrive(0.5);
    

    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);


    while (1) {
        updateDigital();
    }
}