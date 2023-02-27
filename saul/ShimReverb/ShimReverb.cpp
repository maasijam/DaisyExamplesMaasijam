#include "../daisy_saul.h"
#include "dsp/ShimmerReverb.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace saul;


DaisySaul hw;
ShimmerReverb  shimrev;

#define LOOPER_MAX_SIZE (48000 * 30 * 1) // 1 minutes stereo of floats at 48 khz

float DSY_SDRAM_BSS mlooper_buf_1l[LOOPER_MAX_SIZE];
float DSY_SDRAM_BSS mlooper_buf_1r[LOOPER_MAX_SIZE];

float DSY_SDRAM_BSS mlooper_frozen_buf_1l[LOOPER_MAX_SIZE];

void Controls();


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    float out1, out2, in1, in2;
    
    Controls();
  
    
    for(size_t i = 0; i < size; i ++)
    {
        
        in1 = in[0][i];
        in2 = in[1][i];

        out1 = 0.f;
        out2 = 0.f;

        shimrev.Process(out1, out2, in1, in2);

        out[0][i] = out1;
        out[1][i] = out2;
    }
}



int main(void)
{
    float sample_rate;
    
    
    hw.Init();
    //hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();
    //global_sample_rate = sample_rate;
    shimrev.Init(mlooper_buf_1l,mlooper_buf_1r,mlooper_frozen_buf_1l,sample_rate);
    
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);


    while (1) {
        
    }
}

void UpdateKnobs()

{  
    float blend = hw.GetKnobValue(KNOB_8);
    //float speed = hw.GetKnobValue(KNOB_1); 
    float tone = hw.GetKnobValue(KNOB_2);
    float index = hw.GetKnobValue(KNOB_9);
    float regen = hw.GetKnobValue(KNOB_10); 
    //float size = hw.GetKnobValue(KNOB_5);
    float dense = hw.GetKnobValue(KNOB_6);

    shimrev.SetTone(tone);
    shimrev.SetShimmer(index);
    shimrev.SetFdbk(regen);
    shimrev.SetCompression(dense);
    shimrev.SetDryWet(blend);
    
}


void Controls()
{

    //reverb_drywet = 0;

    hw.ProcessAnalogControls();
    UpdateKnobs();

}