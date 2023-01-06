#include "../daisy_margolis.h"
#include "daisysp.h"


using namespace daisy;
using namespace daisysp;

DaisyMargolis hw;

Oscillator modulator;
Oscillator carrier;
AdEnv      env;

float carrierBaseFreq;
float modAmount;
float envVal;
float inLedVal;
float outLedVal;
float pitchOffset = 0;


void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	
    hw.ProcessAllControls();

    

    //pitchOffset += hw.encoder.Pressed() ? encInc / 12 : (encInc / 12) * 0.05;
    
    //float voct_cv = hw.cv[hw.CV_VOCT].Value();
	//pitchOffset    = fmap(voct_cv, 0.f, 60.f);
    pitchOffset = 0;

    carrierBaseFreq
        = pow(2,
              hw.cv[hw.CV_VOCT].Value() * 8
                  + 1 + pitchOffset)
          * 8;

    modAmount = pow(hw.knob[hw.KNOB_2].Value(), 2) * 10000;

    float modFreq
        = carrierBaseFreq
          * pow(2, hw.knob[hw.KNOB_3].Value() * 5);

    modulator.SetFreq(modFreq);

    if(hw.gate_in.Trig())
    {
        env.SetTime(ADENV_SEG_ATTACK, 0.005);
        env.SetTime(ADENV_SEG_DECAY,
                    hw.knob[hw.KNOB_4].Value());
        env.Trigger();
    }

    for(size_t i = 0; i < size; i++)
    {
       
        envVal   = env.Process();
        inLedVal = abs((in[0][i] + in[1][i]) / 2.0);

        float modA = modulator.Process() * modAmount * (envVal + 0);
        float modB = (in[0][i] + in[1][i]) * modAmount
                     * 0 / 2.0;

        carrier.SetFreq(carrierBaseFreq + modA + modB);
        carrier.SetAmp(envVal);

        float output = carrier.Process();
        outLedVal    = abs(output);
        // left out
        out[0][i] = output;
        // right out
        out[1][i] = output;
        //out[0][i] = sig;
        //out[1][i] = sig;

    }

    
}



int main(void)
{
    hw.Init();
    hw.ClearLeds();
    hw.UpdateLeds();

    modulator.Init(hw.AudioSampleRate());
    modulator.SetFreq(0);

    carrier.Init(hw.AudioSampleRate());
    carrier.SetFreq(0);

    env.Init(hw.AudioSampleRate());

    hw.StartAudio(audio_callback);
    hw.StartAdc();

	while(1)
	{	
		
        hw.SetRgbLeds(hw.LED_RGB_1,envVal, inLedVal, outLedVal);
        hw.SetRgbLeds(hw.LED_RGB_2,hw.knob[hw.KNOB_1].Value(), hw.knob[hw.KNOB_2].Value(), hw.knob[hw.KNOB_3].Value());
        hw.UpdateLeds();
	}
}

