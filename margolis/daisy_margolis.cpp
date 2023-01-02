#include "daisy_margolis.h"


using namespace daisy;



// MARGOLIS Pins
#define PIN_S1 0
#define PIN_S2 13
#define PIN_S3 14
#define PIN_S4 25
#define PIN_S5 26
#define PIN_S6 27

#define PIN_CV1 17
#define PIN_CV2 19
#define PIN_CV3 18
#define PIN_CV4 21
#define PIN_CV5 16
#define PIN_CV6 20
#define PIN_CV7 15

#define PIN_TRIG_IN 10

#define PIN_KNOB_MUX 24
#define PIN_MUX_C 9
#define PIN_MUX_B 8
#define PIN_MUX_A 7

enum LedOrder
{
    LED_1_R = 30,
    LED_1_G = 31,
    LED_1_B = 0,
    LED_2_R = 28,
    LED_2_G = 29,
    LED_2_B = 1,
    LED_3_R = 26,
    LED_3_G = 27,
    LED_3_B = 2,
    LED_4_R = 24,
    LED_4_G = 25,
    LED_4_B = 3,
    LED_5_R = 17,
    LED_5_G = 16,
    LED_5_B = 4,
    LED_6_R = 19,
    LED_6_G = 18,
    LED_6_B = 5,
    LED_7_R = 21,
    LED_7_G = 20,
    LED_7_B = 6,
    LED_8_R = 23,
    LED_8_G = 22,
    LED_8_B = 7,
    LED_GREEN_1 = 10,
    LED_GREEN_2 = 9,
    LED_GREEN_3 = 8,
    LED_LAST = 27,
};



/** @brief const used internally within Firmware to manage buffer memory 
 *  This can be safely exceeded in custom firmware.
 */
static constexpr int kMaxBlockSize = 96;


static constexpr I2CHandle::Config margolis_led_i2c_config
    = {I2CHandle::Config::Peripheral::I2C_1,
       {{DSY_GPIOB, 8}, {DSY_GPIOB, 9}},
       I2CHandle::Config::Speed::I2C_1MHZ};


static LedDriverPca9685<2, true>::DmaBuffer DMA_BUFFER_MEM_SECTION
    margolis_led_dma_buffer_a,
    margolis_led_dma_buffer_b;








void DaisyMargolis::Init(bool boost)
{
    seed.Configure();
    seed.Init(boost);
    //seed.SetAudioBlockSize(48);
    InitLeds();
    
    // Push Buttons
    uint8_t s_pin[] = {PIN_S1,
                         PIN_S2,
                         PIN_S3,
                         PIN_S4,
                         PIN_S5,
                         PIN_S6};
     // push buttons
    for(size_t i = 0; i < S_LAST; i++)
    {
        s[i].Init(seed.GetPin(s_pin[i]));
    }

   

    // gate in 
    dsy_gpio_pin gateIn1_gpio = seed.GetPin(PIN_TRIG_IN);
    gate_in.Init(&gateIn1_gpio,true);


    // ADCs
    AdcChannelConfig adc_cfg[CV_LAST + 1];

    int cv_pins[7] = {PIN_CV1,PIN_CV2,PIN_CV3,PIN_CV4,PIN_CV5,PIN_CV6,PIN_CV7};
    for(int i = 0; i < CV_LAST; i++)
        {
            adc_cfg[i].InitSingle(seed.GetPin(cv_pins[i]));
        }

    // POT/CV MUX
    adc_cfg[KNOB_LAST].InitMux(seed.GetPin(PIN_KNOB_MUX),
                             8,
                             seed.GetPin(PIN_MUX_A),
                             seed.GetPin(PIN_MUX_B),
                             seed.GetPin(PIN_MUX_C));
    
    
    
    seed.adc.Init(adc_cfg,CV_LAST + 1);
 
    
    // init cv as bipolar analog controls
    for(size_t i = 0; i < CV_LAST; i++)
    {
        cv[i].InitBipolarCv(seed.adc.GetPtr(i), AudioCallbackRate());
    }

    // init pots as analog controls
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob[i].Init(seed.adc.GetMuxPtr(CV_LAST, i),
                            AudioCallbackRate());
    }

    

    DacHandle::Config cfg;
    cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    cfg.buff_state = DacHandle::BufferState::ENABLED;
    cfg.mode       = DacHandle::Mode::POLLING;
    cfg.chn        = DacHandle::Channel::BOTH;
    seed.dac.Init(cfg);

    


        
}

void DaisyMargolis::SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate sr)
{
    seed.SetAudioSampleRate(sr);
    SetHidUpdateRates();
}

float DaisyMargolis::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisyMargolis::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyMargolis::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisyMargolis::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyMargolis::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyMargolis::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyMargolis::StopAudio()
{
    seed.StopAudio();
}

void DaisyMargolis::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyMargolis::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyMargolis::StartAdc()
{
    seed.adc.Start();
}
void DaisyMargolis::StopAdc()
{
    seed.adc.Stop();
}

/** Turns on the built-in 12-bit DAC on the Daisy Seed */
void DaisyMargolis::StartDac()
{
    //dsy_dac_start(DSY_DAC_CHN_BOTH);
}

void DaisyMargolis::ProcessAnalogControls()
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

bool DaisyMargolis::Gate()
{
    return !gate_in.State();
}


void DaisyMargolis::ProcessDigitalControls()
{
    // Switches
    for(size_t i = 0; i < S_LAST; i++)
    {
        s[i].Debounce();
        
    }
}


float DaisyMargolis::GetKnobValue(int idx) const
{
    //return (knob[idx].Value());
    return knob[idx < KNOB_LAST ? idx : 0].Value();
}
float DaisyMargolis::GetCvValue(int idx) const 
{
    //return (cv[idx].Value());
    return cv[idx < CV_LAST ? idx : 0].Value();
}

AnalogControl* DaisyMargolis::GetCv(size_t idx)
{
    return &cv[idx < CV_LAST ? idx : 0];
}

void DaisyMargolis::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyMargolis::SetHidUpdateRates()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob[i].SetSampleRate(AudioCallbackRate());
    }
    for(size_t i = 0; i < CV_LAST; i++)
    {
        cv[i].SetSampleRate(AudioCallbackRate());
    }
    for(int i = 0; i < S_LAST; i++)
    {
        s[i].SetUpdateRate(AudioCallbackRate());
    }
}

void DaisyMargolis::SetCvOut1(uint16_t val)
{
    seed.dac.WriteValue(DacHandle::Channel::ONE, val);
}

void DaisyMargolis::SetCvOut2(uint16_t val)
{
    seed.dac.WriteValue(DacHandle::Channel::TWO, val);
}



void DaisyMargolis::ClearLeds()
{
    

    for (size_t i = 0; i < LEDDRIVER_LAST; i++)
    {
        if(i < 8) {
            SetRgbLeds(static_cast<LeddriverLeds>(i), 0.0f, 0.0f, 0.0f);
        } else  {
            //SetGreenLeds(static_cast<LeddriverLeds>(i), 0.0f);
            SetGreenLeds((i), 0.0f);
        } 
    }
    
    
}

void DaisyMargolis::UpdateLeds()
{
    
    
    led_driver_.SwapBuffersAndTransmit();
}

void DaisyMargolis::SetRgbLeds(LeddriverLeds idx, float r, float g, float b)
{
    uint8_t r_addr[8] = {LED_1_R,
                                     LED_2_R,
                                     LED_3_R,
                                     LED_4_R,
                                     LED_5_R,
                                     LED_6_R,
                                     LED_7_R,
                                     LED_8_R};
    uint8_t g_addr[8] = {LED_1_G,
                                     LED_2_G,
                                     LED_3_G,
                                     LED_4_G,
                                     LED_5_G,
                                     LED_6_G,
                                     LED_7_G,
                                     LED_8_G};
    uint8_t b_addr[8] = {LED_1_B,
                                     LED_2_B,
                                     LED_3_B,
                                     LED_4_B,
                                     LED_5_B,
                                     LED_6_B,
                                     LED_7_B,
                                     LED_8_B};


    led_driver_.SetLed(r_addr[idx], r);
    led_driver_.SetLed(g_addr[idx], g);
    led_driver_.SetLed(b_addr[idx], b);
}
void DaisyMargolis::SetGreenLeds(size_t idx, float bright)
{
    led_driver_.SetLed(idx, bright);
}


void DaisyMargolis::SetRGBColor (LeddriverLeds idx, Colors color)
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






float DaisyMargolis::CVKnobCombo(float CV_Val,float Pot_Val)
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

void DaisyMargolis::InitLeds()
{
    // LEDs are on the LED Driver.

    // Need to figure out how we want to handle that.
    uint8_t   addr[2] = {0x00, 0x01};
    I2CHandle i2c;
    i2c.Init(margolis_led_i2c_config);
    led_driver_.Init(i2c, addr, margolis_led_dma_buffer_a, margolis_led_dma_buffer_b);
    ClearLeds();
    UpdateLeds();
}


