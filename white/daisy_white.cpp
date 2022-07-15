#include "daisy_white.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE /**< & */
#endif

using namespace daisy;


// SAUL Pins
#define PIN_CLOCK_IN1 0
#define PIN_CLOCK_IN2 30

#define PIN_TOGGLE3_0A 18
#define PIN_TOGGLE3_0B 19
#define PIN_TOGGLE3_1A 20
#define PIN_TOGGLE3_1B 21

#define PIN_ADC_KNOB_MUX 15
#define PIN_ADC_CV_MUX 16
#define PIN_ADC_KNOB_CV_MUX 17
#define PIN_SW_MUX 24
#define PIN_MUX_SEL_2 9
#define PIN_MUX_SEL_1 8
#define PIN_MUX_SEL_0 7

#define PIN_CD4021_D1 26
#define PIN_CD4021_CS 27
#define PIN_CD4021_CLK 28

enum LedOrder
{
    LED_1_R,
    LED_1_G,
    LED_1_B,
    LED_2_R,
    LED_2_G,
    LED_2_B,
    LED_3_R,
    LED_3_G,
    LED_3_B,
    LED_4_R,
    LED_4_G,
    LED_4_B,
    LED_GREEN_1,
    LED_GREEN_2,
    LED_GREEN_3,
    LED_GREEN_4,
    LED_LAST,
};


static constexpr I2CHandle::Config field_led_i2c_config
    = {I2CHandle::Config::Peripheral::I2C_1,
       {{DSY_GPIOB, 8}, {DSY_GPIOB, 9}},
       I2CHandle::Config::Speed::I2C_1MHZ};

static LedDriverPca9685<1, true>::DmaBuffer DMA_BUFFER_MEM_SECTION
    field_led_dma_buffer_a,
    field_led_dma_buffer_b;




const int _rgbLedPins[4][3] = {
    {DaisyWhite::LED_8,DaisyWhite::LED_9,DaisyWhite::LED_10}, 
    {DaisyWhite::LED_11,DaisyWhite::LED_12,DaisyWhite::LED_13}, 
    {DaisyWhite::LED_14,DaisyWhite::LED_15,DaisyWhite::LED_16}, 
    {DaisyWhite::LED_17,DaisyWhite::LED_18,DaisyWhite::LED_19}
};


void DaisyWhite::Init(bool boost)
{
    seed.Configure();
    seed.Init(boost);
    //seed.SetAudioBlockSize(48);

    
    uint8_t toggle_pina[] = {PIN_TOGGLE3_0A, PIN_TOGGLE3_1A};
    uint8_t toggle_pinb[] = {PIN_TOGGLE3_0B, PIN_TOGGLE3_1B};

   

    // 3-position switches
    for(size_t i = 0; i < SW_LAST; i++)
    {
        sw[i].Init(seed.GetPin(toggle_pina[i]), seed.GetPin(toggle_pinb[i]));
    }

    // push buttons
    for(size_t i = 0; i < S_LAST; i++)
    {
        //s[i].Init(seed.GetPin(s_pin[i]));
    }

    // gate in 
    dsy_gpio_pin gate_gpio = seed.GetPin(PIN_CLOCK_IN1);
    gate.Init(&gate_gpio,false);


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

    // LEDs
    // 2x PCA9685 addresses 0x00, and 0x02
    uint8_t   addr[1] = {0x00};
    I2CHandle i2c;
    i2c.Init(field_led_i2c_config);
    led_driver.Init(i2c, addr, field_led_dma_buffer_a, field_led_dma_buffer_b); 

    // Keyboard
    ShiftRegister4021<1>::Config switches_cfg;
    switches_cfg.clk     = seed.GetPin(PIN_CD4021_CLK);
    switches_cfg.latch   = seed.GetPin(PIN_CD4021_CS);
    switches_cfg.data[0] = seed.GetPin(PIN_CD4021_D1);
    switches_sr_.Init(switches_cfg);

   

    
}

void DaisyWhite::SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate sr)
{
    seed.SetAudioSampleRate(sr);
    SetHidUpdateRates();
}

float DaisyWhite::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisyWhite::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyWhite::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisyWhite::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyWhite::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyWhite::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyWhite::StopAudio()
{
    seed.StopAudio();
}

void DaisyWhite::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyWhite::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyWhite::StartAdc()
{
    seed.adc.Start();
}
void DaisyWhite::StopAdc()
{
    seed.adc.Stop();
}

void DaisyWhite::ProcessAnalogControls()
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

Switch* DaisyWhite::GetSwitch(size_t idx)
{
    return &s[idx < S_LAST ? idx : 0];
}


bool DaisyWhite::Gate()
{
    return !gate.State();
}

void DaisyWhite::ProcessDigitalControls()
{
    // Switches
    for(size_t i = 0; i < S_LAST; i++)
    {
        s[i].Debounce();
        
    }
    switches_sr_.Update();
    for(size_t i = 0; i < 8; i++)
    {
        uint8_t keyidx, keyoffset;
        keyoffset = i > 7 ? 8 : 0;
        keyidx    = (7 - (i % 8)) + keyoffset;
        switches_state_[keyidx]
            = switches_sr_.State(i) | (switches_state_[keyidx] << 1);
    }
}

float DaisyWhite::GetKnobValue(int idx) const
{
    //return (knob[idx].Value());
    return knob[idx < KNOB_LAST ? idx : 0].Value();
}
float DaisyWhite::GetCvValue(int idx) const 
{
    //return (cv[idx].Value());
    return cv[idx < CV_LAST ? idx : 0].Value();
}

AnalogControl* DaisyWhite::GetCv(size_t idx)
{
    return &cv[idx < CV_LAST ? idx : 0];
}

void DaisyWhite::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyWhite::SetHidUpdateRates()
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

void DaisyWhite::SetCvOut1(uint16_t val)
{
    seed.dac.WriteValue(DacHandle::Channel::ONE, val);
}

void DaisyWhite::SetCvOut2(uint16_t val)
{
    seed.dac.WriteValue(DacHandle::Channel::TWO, val);
}

bool DaisyWhite::SwitchState(size_t idx) const
{
    return switches_state_[idx] == 0x00;
}

bool DaisyWhite::SwitchRisingEdge(size_t idx) const
{
    return switches_state_[idx] == 0x80;
}

bool DaisyWhite::SwitchFallingEdge(size_t idx) const
{
    return switches_state_[idx] == 0x7F;
}

