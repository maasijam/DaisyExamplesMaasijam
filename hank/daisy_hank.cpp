#include "daisy_hank.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE /**< & */
#endif

using namespace daisy;


// HANK Pins
#define PIN_POT1 15
#define PIN_POT2 16

#define PIN_CV1 17
#define PIN_CV2 18


#define PIN_S1 0
#define PIN_SW1A 1
#define PIN_SW1B 2

#define PIN_GATE_OUT 3
#define PIN_TRIG_IN 4

#define PIN_LED1 5
#define PIN_LED2 6
#define PIN_LED3 11
#define PIN_LED4 12
#define PIN_LED5 13
#define PIN_LED6 14
#define PIN_LED7 25
#define PIN_LED8 26
#define PIN_LED9 27
#define PIN_LED10 28

#define PIN_OLED_CS_EXT 7
#define PIN_OLED_SCK_EXT 8
#define PIN_OLED_CMD_EXT 9
#define PIN_OLED_DATA_EXT 10

#define PIN_ENC_A_EXT 19
#define PIN_ENC_B_EXT 20
#define PIN_ENC_CLICK_EXT 21

#define PIN_POT3_EXT 24
#define PIN_CV3_EXT 22











void DaisyHank::Init(bool boost)
{
    seed.Configure();
    seed.Init(boost);
    //seed.SetAudioBlockSize(48);

    sw.Init(seed.GetPin(PIN_SW1A), seed.GetPin(PIN_SW1B));
    s1.Init(seed.GetPin(PIN_S1));

   

    // gate in 
    dsy_gpio_pin gateIn1_gpio = seed.GetPin(PIN_TRIG_IN);
    gate_in1.Init(&gateIn1_gpio,false);


    // ADCs
    AdcChannelConfig adc_cfg[4];

    // POT/CV MUX
    adc_cfg[0].InitSingle(seed.GetPin(PIN_POT1));
    adc_cfg[1].InitSingle(seed.GetPin(PIN_POT2));
    adc_cfg[2].InitSingle(seed.GetPin(PIN_CV1));
    adc_cfg[3].InitSingle(seed.GetPin(PIN_CV2));
    
    seed.adc.Init(adc_cfg,4,daisy::AdcHandle::OVS_128);
 
    //seed.adc.Start();

    for(size_t i = 0; i < (KNOB_LAST + CV_LAST); i++) {
        if(i < 2) {
            knob[i].Init(seed.adc.GetPtr(i), AudioCallbackRate()); 
        } else {
            cv[i-2].InitBipolarCv(seed.adc.GetPtr(i), AudioCallbackRate()); 
        } 
    } 

    

    DacHandle::Config cfg;
    cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    cfg.buff_state = DacHandle::BufferState::ENABLED;
    cfg.mode       = DacHandle::Mode::POLLING;
    cfg.chn        = DacHandle::Channel::BOTH;
    seed.dac.Init(cfg);

    int direct_led_pins[10] = {PIN_LED1,PIN_LED2,PIN_LED3,PIN_LED4,PIN_LED5,PIN_LED6,PIN_LED7,PIN_LED8,PIN_LED9,PIN_LED10};
    for(size_t i = 0; i < DIRECT_LEDS_LAST; i++)
    {
        green_direct_leds[i].Init(seed.GetPin(direct_led_pins[i]),true);
    }

    gate_out_1.mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_out_1.pull = DSY_GPIO_NOPULL;
    gate_out_1.pin  = seed.GetPin(PIN_GATE_OUT);
    dsy_gpio_init(&gate_out_1);

    encoder.Init(seed.GetPin(PIN_ENC_A_EXT),
                 seed.GetPin(PIN_ENC_B_EXT),
                 seed.GetPin(PIN_ENC_CLICK_EXT));

    // OLED
    OledDisplay<SSD130x4WireSpi128x64Driver>::Config display_config;

    display_config.driver_config.transport_config.pin_config.dc
        = seed.GetPin(PIN_OLED_CMD_EXT);
    display_config.driver_config.transport_config.pin_config.reset
        = {DSY_GPIOX, 0}; // Not a real pin...

    display.Init(display_config);

    screen_update_period_ = 17; // roughly 60Hz
    screen_update_last_   = seed.system.GetNow();

        
}

void DaisyHank::SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate sr)
{
    seed.SetAudioSampleRate(sr);
    SetHidUpdateRates();
}

float DaisyHank::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisyHank::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyHank::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisyHank::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyHank::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyHank::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyHank::StopAudio()
{
    seed.StopAudio();
}

void DaisyHank::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyHank::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyHank::StartAdc()
{
    seed.adc.Start();
}
void DaisyHank::StopAdc()
{
    seed.adc.Stop();
}

/** Turns on the built-in 12-bit DAC on the Daisy Seed */
void DaisyHank::StartDac()
{
    //dsy_dac_start(DSY_DAC_CHN_BOTH);
}

void DaisyHank::ProcessAnalogControls()
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

bool DaisyHank::GateIn1()
{
    return !gate_in1.State();
}

bool DaisyHank::TrigIn1()
{
    return gate_in1.Trig();
}


void DaisyHank::ProcessDigitalControls()
{
    s1.Debounce();
    encoder.Debounce();
}


float DaisyHank::GetKnobValue(int idx) const
{
    //return (knob[idx].Value());
    return knob[idx < KNOB_LAST ? idx : 0].Value();
}
float DaisyHank::GetCvValue(int idx) const 
{
    //return (cv[idx].Value());
    return cv[idx < CV_LAST ? idx : 0].Value();
}

AnalogControl* DaisyHank::GetCv(size_t idx)
{
    return &cv[idx < CV_LAST ? idx : 0];
}

void DaisyHank::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyHank::SetHidUpdateRates()
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

void DaisyHank::SetCvOut1(uint16_t val)
{
    seed.dac.WriteValue(DacHandle::Channel::ONE, val);
}

void DaisyHank::SetCvOut2(uint16_t val)
{
    seed.dac.WriteValue(DacHandle::Channel::TWO, val);
}



void DaisyHank::ClearLeds()
{
    
    for(size_t i = 0; i < DIRECT_LEDS_LAST; i++)
    {
        SetGreenDirectLeds(static_cast<DirectLeds>(i), 0.0f);
    }
    
}

void DaisyHank::UpdateLeds()
{
    
    for(size_t i = 0; i < DIRECT_LEDS_LAST; i++)
    {
        green_direct_leds[i].Update();
    }
}


void DaisyHank::SetGreenDirectLeds(DirectLeds idx, float bright)
{
    green_direct_leds[idx].Set(bright);
}



float DaisyHank::CVKnobCombo(float CV_Val,float Pot_Val)
{
    float output{};
    output = CV_Val + Pot_Val;

    if(output < 0.0f)
    {
        output = 0.0f;
    }

    if(output > 1.0f)
    {
        output = 1.0f;
    }

    return output;
}