#include "daisy_vertigo.h"

using namespace daisy;

#define PIN_TRIG_IN 24
#define PIN_TAP 30
#define PIN_SW_LEFT 7
#define PIN_SW_RIGHT 2
#define PIN_TOGGLE3_0A 6
#define PIN_TOGGLE3_0B 5
#define PIN_TOGGLE3_1A 1
#define PIN_TOGGLE3_1B 0

#define PIN_LED0_R 10
#define PIN_LED0_G 3
#define PIN_LED0_B 4
#define PIN_LED1_R 12
#define PIN_LED1_G 13
#define PIN_LED1_B 11
#define PIN_LED2_R 25
#define PIN_LED2_G 26
#define PIN_LED2_B 14
#define PIN_LED3_R 29
#define PIN_LED3_G 27
#define PIN_LED3_B 15

#define PIN_SW_L_LED1 20
#define PIN_SW_L_LED2 18
#define PIN_SW_R_LED1 8
#define PIN_SW_R_LED2 9

#define PIN_ADC_CV0 21
#define PIN_ADC_CV1 22
#define PIN_ADC_CV2 28
#define PIN_ADC_CV3 23
#define PIN_ADC_CV4 16
#define PIN_ADC_CV5 17
#define PIN_ADC_CV6 19


void DaisyVertigo::Init(bool boost)
{
    // seed init
    seed.Configure();
    seed.Init(boost);
    seed.SetAudioBlockSize(48);
    float blockrate_ = seed.AudioSampleRate() / (float)seed.AudioBlockSize();

    // pin numbers
    uint8_t toggle_pina[] = {PIN_TOGGLE3_0A, PIN_TOGGLE3_1A};
    uint8_t toggle_pinb[] = {PIN_TOGGLE3_0B, PIN_TOGGLE3_1B};
    uint8_t ledr_pin[]    = {PIN_LED0_R, PIN_LED1_R, PIN_LED2_R, PIN_LED3_R};
    uint8_t ledg_pin[]    = {PIN_LED0_G, PIN_LED1_G, PIN_LED2_G, PIN_LED3_G};
    uint8_t ledb_pin[]    = {PIN_LED0_B, PIN_LED1_B, PIN_LED2_B, PIN_LED3_B};
    uint8_t adc_pin[]     = {PIN_ADC_CV0,
                         PIN_ADC_CV1,
                         PIN_ADC_CV2,
                         PIN_ADC_CV3,
                         PIN_ADC_CV4,
                         PIN_ADC_CV5,
                         PIN_ADC_CV6};
    uint8_t s_pin[]     = {PIN_SW_LEFT,
                         PIN_TAP,
                         PIN_SW_RIGHT};
    uint8_t sw_led_pin[]     = {PIN_SW_L_LED1,
                         PIN_SW_L_LED2,
                         PIN_SW_R_LED1,
                         PIN_SW_R_LED2};
    

    for(size_t i = 0; i < S_LAST; i++)
    {
        s[i].Init(seed.GetPin(s_pin[i]));
    }
    // gate in and momentary switch
    dsy_gpio_pin gate_gpio = seed.GetPin(PIN_TRIG_IN);
    gate.Init(&gate_gpio);


    // 3-position switches
    for(size_t i = 0; i < SW_LAST; i++)
    {
        sw[i].Init(seed.GetPin(toggle_pina[i]), seed.GetPin(toggle_pinb[i]));
    }

    // ADC
    AdcChannelConfig adc_cfg[KNOB_LAST];
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        adc_cfg[i].InitSingle(seed.GetPin(adc_pin[i]));
    }
    seed.adc.Init(adc_cfg, 7);

    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knobs[i].Init(seed.adc.GetPtr(i), blockrate_, true);
    }

    // RGB LEDs
    for(size_t i = 0; i < LED_LAST; i++)
    {
        dsy_gpio_pin r = seed.GetPin(ledr_pin[i]);
        dsy_gpio_pin g = seed.GetPin(ledg_pin[i]);
        dsy_gpio_pin b = seed.GetPin(ledb_pin[i]);
        leds[i].Init(r, g, b, true);
    }

    // Switch LEDs
    for(size_t i = 0; i < S_LED_LAST; i++)
    {
        
        sw_led[i].Init(seed.GetPin(sw_led_pin[i]),false);
    }
}

void DaisyVertigo::SetHidUpdateRates()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knobs[i].SetSampleRate(AudioCallbackRate());
    }
}


void DaisyVertigo::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyVertigo::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyVertigo::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyVertigo::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyVertigo::StopAudio()
{
    seed.StopAudio();
}

void DaisyVertigo::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyVertigo::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

void DaisyVertigo::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisyVertigo::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

float DaisyVertigo::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyVertigo::StartAdc()
{
    seed.adc.Start();
}

void DaisyVertigo::StopAdc()
{
    seed.adc.Stop();
}

void DaisyVertigo::ProcessAnalogControls()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knobs[i].Process();
    }
}

bool DaisyVertigo::SwitchPressed(int idx)
{
    return s[idx].Pressed();
}

bool DaisyVertigo::Gate()
{
    return !gate.State();
}

void DaisyVertigo::SetLed(size_t idx, float red, float green, float blue)
{
    leds[idx].Set(red, green, blue);
}

void DaisyVertigo::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyVertigo::UpdateLeds()
{
    for(size_t i = 0; i < LED_LAST; i++)
    {
        leds[i].Update();
    }
    for(size_t i = 0; i < S_LED_LAST; i++)
    {
        sw_led[i].Update();
    }
}

float DaisyVertigo::GetKnobValue(int idx)
{
    return knobs[idx].Value();
}

void DaisyVertigo::UpdateExample()
{
    for(size_t i = 0; i < S_LAST; i++)
    {
        s[i].Debounce();
    }
    for(size_t i = 0; i < LED_LAST - 1; i++)
        SetLed(i, 0, 0, 0);

    SetLed(sw[0].Read(), knobs[0].Value(), knobs[1].Value(), knobs[2].Value());
    SetLed(sw[1].Read(), knobs[3].Value(), knobs[4].Value(), knobs[5].Value());

    SetLed(3, Gate(), SwitchPressed(DaisyVertigo::S_Tap), knobs[6].Value());

    if(SwitchPressed(DaisyVertigo::S_Left)) {
        sw_led[0].Set(1.f);
        sw_led[1].Set(1.f);
    } else {
        sw_led[0].Set(0.f);
        sw_led[1].Set(0.f);
    }

    if(SwitchPressed(DaisyVertigo::S_Right)) {
        sw_led[2].Set(1.f);
        sw_led[3].Set(1.f);
    } else {
        sw_led[2].Set(0.f);
        sw_led[3].Set(0.f);
    }
    

}
