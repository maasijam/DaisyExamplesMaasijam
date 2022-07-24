#include <array>

#include "../daisy_white.h"
#include "daisysp.h"




using namespace daisy;
using namespace daisysp;



DaisyWhite hw;


void Update_Leds();
void Update_Buttons();

enum
{
    S1      = 15,
    S2      = 14,
    S3      = 13,
    S4      = 12,
    S5      = 11,
    S6      = 10,
    S7      = 9,
    S8      = 8,
    S0A     = 7,
    S0B     = 6,
    S1A     = 5,
    S1B     = 4,
};


struct lfoStruct
{
    Oscillator osc;
    Parameter  freqCtrl;
    Parameter  ampCtrl;
    float      amp_;
    float      freq_;
    int        waveform;
    float      value;

    void Init(float samplerate, float freq, float amp)
    {
        osc.Init(samplerate);
        osc.SetAmp(1);
        waveform = 0;
        freq_ = freq;
        amp_ = amp;
    }

    void Process(DacHandle::Channel chn)
    {
        //read the knobs and set params
        osc.SetFreq(freq_);
        osc.SetWaveform(waveform);

        //write to the DAC
        hw.seed.dac.WriteValue(
            chn,
            uint16_t((osc.Process() + 1.f) * .5f * amp_ * 4095.f));
    }
};

lfoStruct lfos[2];


void audio_callback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	for(size_t i = 0; i < size; i++)
    {
        lfos[0].Process(DacHandle::Channel::ONE);
        lfos[1].Process(DacHandle::Channel::TWO);
    }
}


int main(void)
{
	hw.Init();
    float samplerate = hw.AudioSampleRate();

    //init the lfos
    lfos[0].Init(samplerate, 0.5f, 0.5f);
    lfos[1].Init(samplerate, 10.f, 0.8f);
    
    

    	
    hw.StartAdc();
	hw.StartAudio(audio_callback);

    

	while(1)
	{	
		hw.ProcessAllControls();
		Update_Buttons();
        //Update_Leds();

        //wait 1 ms
        System::Delay(1);	

        	
	}
}

void Update_Leds() {
   
	
}

void Update_Buttons() {

    hw.ClearLeds();

	if(!hw.SwitchState(S1)){
       hw.SetRgbLeds(DaisyWhite::RGB_LED_1, 1.f,0.f,0.f);
    } 
    if(!hw.SwitchState(S2)){
        hw.SetRgbLeds(DaisyWhite::RGB_LED_2, 1.f,0.f,0.2f);
    } 
    if(!hw.SwitchState(S3)){
        hw.SetRgbLeds(DaisyWhite::RGB_LED_3, 0.f,1.f,0.f);
    } 
    if(!hw.SwitchState(S4)){
        hw.SetRgbLeds(DaisyWhite::RGB_LED_4, 0.f,0.f,1.f);
    } 
    if(!hw.SwitchState(S5)){
        hw.SetGreenLeds(DaisyWhite::GREEN_LED_1, 1.f);
    } 
    if(!hw.SwitchState(S6)){
        hw.SetGreenLeds(DaisyWhite::GREEN_LED_2, 1.f);
    } 
    if(!hw.SwitchState(S7)){
        hw.SetGreenLeds(DaisyWhite::GREEN_LED_3, 1.f);
    } 
    if(!hw.SwitchState(S8)){
        hw.SetGreenLeds(DaisyWhite::GREEN_LED_4, 1.f);
    } 
    if(!hw.SwitchState(S0A)){
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_1, 1.f);
    } 
    if(!hw.SwitchState(S0B)){
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_2, 1.f);
    } 
    if(!hw.SwitchState(S1A)){
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_3, 1.f);
    } 
    if(!hw.SwitchState(S1B)){
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_4, 1.f);
    } 
    
      hw.UpdateLeds();
}