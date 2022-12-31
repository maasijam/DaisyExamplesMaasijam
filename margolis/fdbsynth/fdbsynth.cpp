#include "../daisy_margolis.h"
#include "src/FeedbackSynthEngine.h"
#include "src/FeedbackSynthControls.h"

using namespace infrasonic;
using namespace daisy;
using namespace daisysp;


static const size_t kBlockSize = 4;

static DaisyMargolis hw;
static FeedbackSynth::Engine engine;
static FeedbackSynth::Controls controls;

WhiteNoise noise;

void levelMeter(float out);
float outlev;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    controls.Process();
    //noise.SetAmp(0.8f);
    for (size_t i=0; i<size; i++) {
        engine.Process(IN_L[i], OUT_L[i], OUT_R[i]);
        outlev = OUT_L[i];
    }
}

int main(void)
{
    hw.Init();
    float samplerate = hw.AudioSampleRate();

    //noise.Init();


    hw.SetAudioBlockSize(kBlockSize);
    engine.Init(samplerate);
    controls.Init(hw, engine);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1) {
        controls.Update(hw);
        levelMeter(outlev);
    }

    return 0;
}


void levelMeter(float out) {
    int outint = 0;
    if(out < 0.125f && out >= 0.01f){
        outint = 1;
    } else if(out < 0.25f && out >= 0.125f) {
        outint = 2;
    } else if(out < 0.375f && out >= 0.25f) {
        outint = 3;
    } else if(out < 0.5f && out >= 0.375f) {
        outint = 4;
    } else if(out < 0.625f && out >= 0.5f) {
        outint = 5;
    } else if(out < 0.75f && out >= 0.625f) {
        outint = 6;
    } else if(out < 0.875f && out >= 0.75f) {
        outint = 7;
    } else if(out <= 1.0f && out >= 0.875f) {
        outint = 8;
    }

    switch (outint)
    {
    case 0:
        hw.SetRGBColor(hw.LED_RGB_8,hw.off);
        hw.SetRGBColor(hw.LED_RGB_7,hw.off);
        hw.SetRGBColor(hw.LED_RGB_6,hw.off);
        hw.SetRGBColor(hw.LED_RGB_5,hw.off);
        hw.SetRGBColor(hw.LED_RGB_4,hw.off);
        hw.SetRGBColor(hw.LED_RGB_3,hw.off);
        hw.SetRGBColor(hw.LED_RGB_2,hw.off);
        hw.SetRGBColor(hw.LED_RGB_1,hw.off);
        break;
    case 1:
        hw.SetRGBColor(hw.LED_RGB_8,hw.green);
        break;
    case 2:
        hw.SetRGBColor(hw.LED_RGB_8,hw.green);
        hw.SetRGBColor(hw.LED_RGB_7,hw.green);
        break;
    case 3:
        hw.SetRGBColor(hw.LED_RGB_8,hw.green);
        hw.SetRGBColor(hw.LED_RGB_7,hw.green);
        hw.SetRGBColor(hw.LED_RGB_6,hw.green);
        break;
    case 4:
        hw.SetRGBColor(hw.LED_RGB_8,hw.green);
        hw.SetRGBColor(hw.LED_RGB_7,hw.green);
        hw.SetRGBColor(hw.LED_RGB_6,hw.green);
        hw.SetRGBColor(hw.LED_RGB_5,hw.green);
        break;
    case 5:
        hw.SetRGBColor(hw.LED_RGB_8,hw.green);
        hw.SetRGBColor(hw.LED_RGB_7,hw.green);
        hw.SetRGBColor(hw.LED_RGB_6,hw.green);
        hw.SetRGBColor(hw.LED_RGB_5,hw.green);
        hw.SetRGBColor(hw.LED_RGB_4,hw.green);
        break;
    case 6:
        hw.SetRGBColor(hw.LED_RGB_8,hw.green);
        hw.SetRGBColor(hw.LED_RGB_7,hw.green);
        hw.SetRGBColor(hw.LED_RGB_6,hw.green);
        hw.SetRGBColor(hw.LED_RGB_5,hw.green);
        hw.SetRGBColor(hw.LED_RGB_4,hw.green);
        hw.SetRGBColor(hw.LED_RGB_3,hw.yellow);
        break;
    case 7:
        hw.SetRGBColor(hw.LED_RGB_8,hw.green);
        hw.SetRGBColor(hw.LED_RGB_7,hw.green);
        hw.SetRGBColor(hw.LED_RGB_6,hw.green);
        hw.SetRGBColor(hw.LED_RGB_5,hw.green);
        hw.SetRGBColor(hw.LED_RGB_4,hw.green);
        hw.SetRGBColor(hw.LED_RGB_3,hw.yellow);
        hw.SetRGBColor(hw.LED_RGB_2,hw.yellow);
        break;
    case 8:
        hw.SetRGBColor(hw.LED_RGB_8,hw.green);
        hw.SetRGBColor(hw.LED_RGB_7,hw.green);
        hw.SetRGBColor(hw.LED_RGB_6,hw.green);
        hw.SetRGBColor(hw.LED_RGB_5,hw.green);
        hw.SetRGBColor(hw.LED_RGB_4,hw.green);
        hw.SetRGBColor(hw.LED_RGB_3,hw.yellow);
        hw.SetRGBColor(hw.LED_RGB_2,hw.yellow);
        hw.SetRGBColor(hw.LED_RGB_1,hw.red);
        break;
    }
    hw.UpdateLeds();
}