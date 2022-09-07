#include "daisysp.h"
#include "../daisy_patch_3060.h"

using namespace daisy;
using namespace daisysp;



DaisyPatch3060 patch;
Oscillator osc;

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    float  sig;
    

    

    for(size_t i = 0; i < size; i++)
    {
        // Read Knobs
        //freq = mtof(freqctrl.Process() + finectrl.Process());
        //wave = wavectrl.Process();
        //amp  = ampctrl.Process();
        // Set osc params
        osc.SetFreq(500.f);
        osc.SetWaveform(osc.WAVE_POLYBLEP_SAW);
        osc.SetAmp(0.8f);
        //.Process
        sig = osc.Process();
        // Assign Synthesized Waveform to all four outputs.
        for(size_t chn = 0; chn < 4; chn++)
        {
            out[chn][i] = sig;
        }
    }
}



int main(void)
{
    patch.Init(); // Initialize hardware (daisy seed, and patch)
    float samplerate = patch.AudioSampleRate();

    osc.Init(samplerate); 

    patch.StartAdc();
    patch.StartAudio(AudioCallback);

    while(1)
    {
        patch.DelayMs(1);
    }
}



