#include "daisy_saul.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE /**< & */
#endif

using namespace daisy;


// SAUL Pins
#define PIN_CLOCK_IN 0
#define PIN_TAP 22
#define PIN_S1 25
#define PIN_S2 10
#define PIN_S3 24
#define PIN_S4 23
#define PIN_S5 29
#define PIN_S6 30
#define PIN_TOGGLE3_0A 18
#define PIN_TOGGLE3_0B 19
#define PIN_TOGGLE3_1A 20
#define PIN_TOGGLE3_1B 21

#define PIN_ADC_KNOB_MUX 15
#define PIN_MUX_SEL_2 9
#define PIN_MUX_SEL_1 8
#define PIN_MUX_SEL_0 7


#define PIN_ADC_CV_MUX 16
#define PIN_ADC_KNOB_CV_MUX 17

#define PIN_HC595_D1 14
#define PIN_HC595_CS 12
#define PIN_HC595_CLK 11

const int _rgbLedPins[4][3] = {
    {DaisySaul::LED_8,DaisySaul::LED_9,DaisySaul::LED_10}, 
    {DaisySaul::LED_11,DaisySaul::LED_12,DaisySaul::LED_13}, 
    {DaisySaul::LED_14,DaisySaul::LED_15,DaisySaul::LED_16}, 
    {DaisySaul::LED_17,DaisySaul::LED_18,DaisySaul::LED_19}
};


void DaisySaul::Init(bool boost)
{
    seed.Configure();
    seed.Init(boost);
    //seed.SetAudioBlockSize(48);

    
    uint8_t toggle_pina[] = {PIN_TOGGLE3_0A, PIN_TOGGLE3_1A};
    uint8_t toggle_pinb[] = {PIN_TOGGLE3_0B, PIN_TOGGLE3_1B};

    // Push Buttons
    uint8_t s_pin[] = {PIN_S1,
                         PIN_S2,
                         PIN_S3,
                         PIN_S4,
                         PIN_S5,
                         PIN_S6,
                         PIN_TAP};
   

    // 3-position switches
    for(size_t i = 0; i < SW_LAST; i++)
    {
        sw[i].Init(seed.GetPin(toggle_pina[i]), seed.GetPin(toggle_pinb[i]));
    }

    // push buttons
    for(size_t i = 0; i < S_LAST; i++)
    {
        s[i].Init(seed.GetPin(s_pin[i]));
    }

    // gate in 
    dsy_gpio_pin gate_gpio = seed.GetPin(PIN_CLOCK_IN);
    gate.Init(&gate_gpio);


    // ADCs
    AdcChannelConfig adc_cfg[3];

    // POT/CV MUX
    adc_cfg[0].InitMux(seed.GetPin(PIN_ADC_KNOB_MUX),
                             8,
                             seed.GetPin(PIN_MUX_SEL_0),
                             seed.GetPin(PIN_MUX_SEL_1),
                             seed.GetPin(PIN_MUX_SEL_2));
    adc_cfg[1].InitMux(seed.GetPin(PIN_ADC_CV_MUX),
                             8,
                             seed.GetPin(PIN_MUX_SEL_0),
                             seed.GetPin(PIN_MUX_SEL_1),
                             seed.GetPin(PIN_MUX_SEL_2));
    adc_cfg[2].InitMux(seed.GetPin(PIN_ADC_KNOB_CV_MUX),
                             8,
                             seed.GetPin(PIN_MUX_SEL_0),
                             seed.GetPin(PIN_MUX_SEL_1),
                             seed.GetPin(PIN_MUX_SEL_2));
    
    seed.adc.Init(adc_cfg,3,daisy::AdcHandle::OVS_128);
 
    //seed.adc.Start();

    for(size_t i = 0; i < 24; i++) {
        if(i < 8) {
            knob[i].Init(seed.adc.GetMuxPtr(0, i), AudioCallbackRate()); 
        } else if(i > 7 && i < 16) {
            cv[i-8].InitBipolarCv(seed.adc.GetMuxPtr(1, i-8), AudioCallbackRate()); 
        } else if(i > 15 && i < 19) {
            knob[i-8].Init(seed.adc.GetMuxPtr(2, i-16), AudioCallbackRate());
        } else if(i > 18 && i < 22) {
            cv[i-11].InitBipolarCv(seed.adc.GetMuxPtr(2, i-16), AudioCallbackRate());
        }
    }  

   

    dsy_gpio_pin allleds_cfg[3];
    allleds_cfg[0]    = seed.GetPin(PIN_HC595_CS);
    allleds_cfg[1]    = seed.GetPin(PIN_HC595_CLK);
    allleds_cfg[2]  = seed.GetPin(PIN_HC595_D1);
    all_leds.Init(allleds_cfg,3);

    for(size_t i = 0; i < LED_LAST; i++)
    {
        //all_leds.Set(i,true);
        SetLed(i, true);
    }
    
    //all_leds.Write();
}

void DaisySaul::SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate sr)
{
    seed.SetAudioSampleRate(sr);
    SetHidUpdateRates();
}

float DaisySaul::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisySaul::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisySaul::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisySaul::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisySaul::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisySaul::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisySaul::StopAudio()
{
    seed.StopAudio();
}

void DaisySaul::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisySaul::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisySaul::StartAdc()
{
    seed.adc.Start();
}
void DaisySaul::StopAdc()
{
    seed.adc.Stop();
}

void DaisySaul::ProcessAnalogControls()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob[i].Process();
    }
    for(size_t i = 0; i < CV_LAST; i++)
    {
        cv[i].Process();
    }
}

Switch* DaisySaul::GetSwitch(size_t idx)
{
    return &s[idx < S_LAST ? idx : 0];
}


bool DaisySaul::Gate()
{
    return !gate.State();
}

void DaisySaul::ProcessDigitalControls()
{
    // Switches
    for(size_t i = 0; i < S_LAST; i++)
    {
        s[i].Debounce();
        
    }
}

float DaisySaul::GetKnobValue(int idx) const
{
    //return (knob[idx].Value());
    return knob[idx < KNOB_LAST ? idx : 0].Value();
}
float DaisySaul::GetCvValue(int idx) const 
{
    //return (cv[idx].Value());
    return cv[idx < CV_LAST ? idx : 0].Value();
}

void DaisySaul::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisySaul::SetHidUpdateRates()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob[i].SetSampleRate(AudioCallbackRate());
    }
    for(size_t i = 0; i < CV_LAST; i++)
    {
        cv[i].SetSampleRate(AudioCallbackRate());
    }
}

void DaisySaul::InitControls()
{
    
}

void DaisySaul::SetLed(uint8_t idx, bool state)
{
    all_leds.Set(idx,state);
    all_leds.Write();
}

void DaisySaul::SetRGBLed(uint8_t idx, uint8_t color)
{
    uint8_t  redIdx,greenIdx,blueIdx;
    if(idx > 0 && idx < 5) {
        redIdx = _rgbLedPins[idx-1][0];
        greenIdx = _rgbLedPins[idx-1][1];
        blueIdx = _rgbLedPins[idx-1][2];
    }
    SetRGBColor(redIdx,greenIdx,blueIdx,color);
    
}

void DaisySaul::SetRGBColor(uint8_t ridx,uint8_t gidx,uint8_t bidx, uint8_t color)
{
    switch (color) {
            case 0:
                SetLed(ridx, false);
                SetLed(gidx, true);
                SetLed(bidx, true);
                // red
              break;
             case 1:
                SetLed(ridx, true);
                SetLed(gidx, false);
                SetLed(bidx, true);
               // green
              break;
             case 2:
                SetLed(ridx, true);
                SetLed(gidx, true);
                SetLed(bidx, false);
              // blue
              break;
            case 3:
                SetLed(ridx, false);
                SetLed(gidx, false);
                SetLed(bidx, true);
               // yellow
              break;
            case 4:
                SetLed(ridx, false);
                SetLed(gidx, true);
                SetLed(bidx, false);
               // purple
              break;
            case 5:
                SetLed(ridx, true);
                SetLed(gidx, false);
                SetLed(bidx, false);
               // aqua
              break;
            case 6:
                SetLed(ridx, true);
                SetLed(gidx, true);
                SetLed(bidx, true);
              // off
              break;
            case 7:
                SetLed(ridx, false);
                SetLed(gidx, false);
                SetLed(bidx, false);
              // white
              break;
          }  
}