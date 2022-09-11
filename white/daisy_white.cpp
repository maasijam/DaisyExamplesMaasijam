#include "daisy_white.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE /**< & */
#endif

using namespace daisy;


// WHITE Pins
#define PIN_GATE_IN1 10
#define PIN_GATE_IN2 0
#define PIN_GATE_OUT1 13
#define PIN_GATE_OUT2 14


#define PIN_ADC_KNOB_MUX 15
#define PIN_ADC_CV_MUX 16
#define PIN_ADC_KNOB_CV_MUX 17

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
    LED_GREEN_5,
    LED_GREEN_6,
    LED_GREEN_7,
    LED_GREEN_8,
    LED_GREEN_9,
    LED_GREEN_10,
    LED_LAST,
};


static constexpr I2CHandle::Config field_led_i2c_config
    = {I2CHandle::Config::Peripheral::I2C_1,
       {{DSY_GPIOB, 8}, {DSY_GPIOB, 9}},
       I2CHandle::Config::Speed::I2C_1MHZ};

static LedDriverPca9685<1, true>::DmaBuffer DMA_BUFFER_MEM_SECTION
    field_led_dma_buffer_a,
    field_led_dma_buffer_b;



void DaisyWhite::Init(bool boost)
{
    seed.Configure();
    seed.Init(boost);
    //seed.SetAudioBlockSize(48);
   

    // gate in 
    dsy_gpio_pin gateIn1_gpio = seed.GetPin(PIN_GATE_IN1);
    dsy_gpio_pin gateIn2_gpio = seed.GetPin(PIN_GATE_IN2);
    gate_in1.Init(&gateIn1_gpio,false);
    gate_in2.Init(&gateIn2_gpio,false);


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

    for(size_t i = 0; i < 20; i++) {
        if(i < 8) {
            knob[i].Init(seed.adc.GetMuxPtr(0, i), AudioCallbackRate()); 
        } else if(i > 7 && i < 16) {
            cv[i-8].InitBipolarCv(seed.adc.GetMuxPtr(1, i-8), AudioCallbackRate()); 
        } else if(i > 15 && i < 20) {
            knob[i-8].Init(seed.adc.GetMuxPtr(2, i-16), AudioCallbackRate());
        } 
    } 

    // LEDs
    // 2x PCA9685 addresses 0x00, and 0x02
    uint8_t   addr[1] = {0x00};
    I2CHandle i2c;
    i2c.Init(field_led_i2c_config);
    led_driver_.Init(i2c, addr, field_led_dma_buffer_a, field_led_dma_buffer_b); 

    // Keyboard
    ShiftRegister4021<2>::Config switches_cfg;
    switches_cfg.clk     = seed.GetPin(PIN_CD4021_CLK);
    switches_cfg.latch   = seed.GetPin(PIN_CD4021_CS);
    switches_cfg.data[0] = seed.GetPin(PIN_CD4021_D1);
    switches_sr_.Init(switches_cfg);

    DacHandle::Config cfg;
    cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    cfg.buff_state = DacHandle::BufferState::ENABLED;
    cfg.mode       = DacHandle::Mode::POLLING;
    cfg.chn        = DacHandle::Channel::BOTH;
    seed.dac.Init(cfg);

    int direct_led_pins[6] = {25,24,21,20,19,18};
    for(size_t i = 0; i < DIRECT_LEDS_LAST; i++)
    {
        green_direct_leds[i].Init(seed.GetPin(direct_led_pins[i]),true);
    }

    gate_out_1.mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_out_1.pull = DSY_GPIO_NOPULL;
    gate_out_1.pin  = seed.GetPin(PIN_GATE_OUT1);
    dsy_gpio_init(&gate_out_1);

    gate_out_2.mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_out_2.pull = DSY_GPIO_NOPULL;
    gate_out_2.pin  = seed.GetPin(PIN_GATE_OUT2);
    dsy_gpio_init(&gate_out_2);

    
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

/** Turns on the built-in 12-bit DAC on the Daisy Seed */
void DaisyWhite::StartDac()
{
    //dsy_dac_start(DSY_DAC_CHN_BOTH);
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

bool DaisyWhite::GateIn1()
{
    return !gate_in1.State();
}
bool DaisyWhite::GateIn2()
{
    return !gate_in2.State();
}
bool DaisyWhite::TrigIn1()
{
    return gate_in1.Trig();
}
bool DaisyWhite::TrigIn2()
{
    return gate_in2.Trig();
}

void DaisyWhite::ProcessDigitalControls()
{
    switches_sr_.Update();
    for(size_t i = 0; i < 16; i++)
    {
        uint16_t keyidx, keyoffset;
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

void DaisyWhite::ClearLeds()
{
    // Using Color
    //    Color c;
    //    c.Init(Color::PresetColor::OFF);
    //    for(size_t i = 0; i < RING_LED_LAST; i++)
    //    {
    //        ring_led[i].SetColor(c);
    //    }
    /*
    for(size_t i = 0; i < RGB_LED_LAST; i++)
    {
        SetRgbLeds(static_cast<RgbLeds>(i), 0.0f, 0.0f, 0.0f);
    }
    for(size_t i = 0; i < GREEN_LED_LAST; i++)
    {
        SetGreenLeds(static_cast<GreenLeds>(i), 0.0f);
    }
    for(size_t i = 0; i < GREEN_D_LED_LAST; i++)
    {
        SetGreenDirectLeds(static_cast<GreenDirectLeds>(i), 0.0f);
    }
    */
    for (size_t i = 0; i < LEDDRIVER_LEDS_LAST; i++)
    {
        if(i < 4) {
            SetRgbLeds(static_cast<LeddriverLeds>(i), 0.0f, 0.0f, 0.0f);
        } else  {
            //SetGreenLeds(static_cast<LeddriverLeds>(i), 0.0f);
            SetGreenLeds((i+8), 0.0f);
        } 
    }
    for(size_t i = 0; i < DIRECT_LEDS_LAST; i++)
    {
        SetGreenDirectLeds(static_cast<DirectLeds>(i), 0.0f);
    }
    
}

void DaisyWhite::UpdateLeds()
{
    led_driver_.SwapBuffersAndTransmit();
    for(size_t i = 0; i < DIRECT_LEDS_LAST; i++)
    {
        green_direct_leds[i].Update();
    }
}

void DaisyWhite::SetRgbLeds(LeddriverLeds idx, float r, float g, float b)
{
    uint8_t r_addr[4] = {LED_1_R,
                                     LED_2_R,
                                     LED_3_R,
                                     LED_4_R};
    uint8_t g_addr[4] = {LED_1_G,
                                     LED_2_G,
                                     LED_3_G,
                                     LED_4_G};
    uint8_t b_addr[4] = {LED_1_B,
                                     LED_2_B,
                                     LED_3_B,
                                     LED_4_B};


    led_driver_.SetLed(r_addr[idx], r);
    led_driver_.SetLed(g_addr[idx], g);
    led_driver_.SetLed(b_addr[idx], b);
}
void DaisyWhite::SetGreenLeds(size_t idx, float bright)
{
    led_driver_.SetLed(idx, bright);
}

void DaisyWhite::SetGreenDirectLeds(DirectLeds idx, float bright)
{
    green_direct_leds[idx].Set(bright);
}

void DaisyWhite::SetRGBColor (LeddriverLeds idx, Colors color)
{
    
    switch (color)
    {
    case red:
        SetRgbLeds(idx,1.f,0.f,0.f);
        break;
    case green:
        SetRgbLeds(idx,0.f,0.7f,0.f);
        break;
    case blue:
        SetRgbLeds(idx,0.f,0.f,1.f);
        break;
    case yellow:
        SetRgbLeds(idx,1.f,0.7f,0.f);
        break;
    case cyan:
        SetRgbLeds(idx,0.f,1.f,1.f);
        break;
    case purple:
        SetRgbLeds(idx,1.f,0.f,1.f);
        break;
    case orange:
        SetRgbLeds(idx,1.f,0.3f,0.f);
        break;
    case darkgreen:
        SetRgbLeds(idx,0.f,0.2f,0.f);
        break;
    case darkblue:
        SetRgbLeds(idx,0.2f,0.2f,0.6f);
        break;
    case darkred:
        SetRgbLeds(idx,0.4f,0.f,0.f);
        break;
    case turq:
        SetRgbLeds(idx,0.f,0.5f,0.5f);
        break;
    case grey:
        SetRgbLeds(idx,0.75f,0.75f,0.75f);
        break;
    case darkorange:
        SetRgbLeds(idx,0.5f,0.2f,0.f);
        break;
    case white:
        SetRgbLeds(idx,1.f,1.f,1.f);
        break;
    case off:
        SetRgbLeds(idx,0.f,0.f,0.f);
        break;
    }
    
}

float DaisyWhite::CVKnobCombo(float CV_Val,float Pot_Val)
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