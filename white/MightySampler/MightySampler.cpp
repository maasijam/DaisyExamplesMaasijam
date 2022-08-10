// # MightySampler
// ## Description
// Fairly simply sample player.
// Loads 16
//
// Play .wav file from the SD Card.
//
#include <stdio.h>
#include <string.h>
#include <vector>

#include "./WavStream.h"
#include "../daisy_white.h"


using namespace daisy;


DaisyWhite    hw;
SdmmcHandler sdcard;
WavStream    sampler;



double speed = 1.0;

void AudioCallback(const float *in, float *out, size_t size)
{
    int iterator = 0;

    

    speed = hw.GetKnobValue(DaisyWhite::KNOB_0);

    for(size_t i = 0; i < size; i += 2)
    {
        sampler.Stream(speed);

        out[i] = sampler.data[0] * 0.5f;
        out[i + 1] = sampler.data[1] * 0.5f;
    }
}



int main(void)
{
    // Init hardware
    size_t blocksize = 48;
    //hw.Configure();
    hw.Init();

    //auto knobs = {KNOB_1_PIN, KNOB_2_PIN, KNOB_3_PIN, KNOB_4_PIN};

    

    double samplerate = hw.AudioSampleRate();

    

    //    hw.ClearLeds();
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sd_cfg.speed = SdmmcHandler::Speed::SLOW;
    sdcard.Init(sd_cfg);
    
    sampler.Init();

    hw.StartAdc();

    // Init Audio
    hw.SetAudioBlockSize(blocksize);
    hw.StartAudio(AudioCallback);


    // Loop forever...
    for(;;)
    {

    }
}




