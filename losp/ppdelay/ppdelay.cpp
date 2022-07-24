#include "../daisy_losp.h"
#include "daisysp.h"
#include <string>

using namespace daisy;
using namespace daisysp;

DaisyLosp hw;

#define MAX_DELAY ((size_t)(5.0f * 48000.0f))

static DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS dell;
static DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delr;

float smooth_time;

// This runs at a fixed rate, to prepare audio samples
void callback(AudioHandle::InputBuffer  in,
                AudioHandle::OutputBuffer out,
                size_t                    size)
{
        float dry_sig, dellsig, delrsig;
        float deltime, delfb, kval;  // Delay Vars
        float dry, send, wetl, wetr; // Effects Vars
        float samplerate;

        // Assign Output Buffers
        float *out_left, *out_right;
        out_left  = out[0];
        out_right = out[1];

        

        samplerate = hw.AudioSampleRate();
        hw.ProcessDigitalControls();
        hw.ProcessAnalogControls();

        // Get Delay Parameters from knobs.
        kval    = hw.GetKnobValue(DaisyLosp::CONTROL_KNOB_TOP);
        deltime = (0.001f + (kval * kval) * 5.0f) * samplerate;
        delfb   = hw.GetKnobValue(DaisyLosp::CONTROL_KNOB_BOTTOM);
    
        // Audio is interleaved stereo by default
        for(size_t i = 0; i < size; i ++)
        {
            float in_left = in[0][i];
            float in_right = in[1][i];
            
            // Smooth delaytime, and set.
            fonepole(smooth_time, deltime, 0.0005f);
            dell.SetDelay(smooth_time);
            delr.SetDelay(smooth_time);

            dry_sig = in_left + in_right;

            dellsig = dell.Read();
            delrsig = delr.Read();

            dell.Write(dry_sig + (delrsig * delfb));           
            delr.Write((dellsig * delfb));

            // Output
            out_left[i]  = dellsig;
            out_right[i] = delrsig;
        }
}

int main(void)
{
    // Initialize Versio hardware and start audio, ADC
    float samplerate;
    hw.Init();
    samplerate = hw.AudioSampleRate();

    dell.Init();
    delr.Init();

    dell.SetDelay(samplerate * 0.8f); // half second delay
    delr.SetDelay(samplerate * 0.8f);

   

    hw.StartAudio(callback);
    hw.StartAdc();

    while(1)
    {
        
    }
}
