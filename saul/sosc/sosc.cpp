#include "../daisy_saul.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

/** TODO: ADD CALIBRATION */

DaisySaul hw;
Oscillator   osc;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    
    //float coarse_knob = hw.knob[DaisySaul::KNOB_8].Value();
    float coarse_knob = hw.GetKnobValue(DaisySaul::KNOB_8);
    float coarse      = fmap(coarse_knob, 36.f, 96.f);

    //float voct_cv = hw.cv[DaisySaul::CV_8].Value();
    float voct_cv = hw.GetCvValue(DaisySaul::CV_8);;
    float voct    = fmap(voct_cv, 0.f, 60.f);

    float midi_nn = fclamp(coarse + voct, 0.f, 127.f);
    float freq    = mtof(midi_nn);

    osc.SetFreq(freq);

    for(size_t i = 0; i < size; i++)
    {
        float sig = osc.Process();
        OUT_L[i]  = sig;
        OUT_R[i]  = sig;
    }
}

int main(void)
{
    hw.Init();
    osc.Init(hw.AudioSampleRate());
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1) {
            hw.ProcessAnalogControls();
        

    }
}
