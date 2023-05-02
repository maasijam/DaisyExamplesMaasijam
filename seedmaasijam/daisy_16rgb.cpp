#include "daisy_16rgb.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE /**< & */
#endif



// Rev2 Pins

// Encoder
#define ENC_A_PIN 1
#define ENC_B_PIN 2
#define ENC_CLICK_PIN 0

using namespace daisy;


enum LedOrder
{
    LED_1_R = 15,
    LED_1_G = 14,
    LED_1_B = 13,
    LED_2_R = 10,
    LED_2_G = 11,
    LED_2_B = 12,
    LED_3_R = 39,
    LED_3_G = 8,
    LED_3_B = 9,
    LED_4_R = 38,
    LED_4_G = 37,
    LED_4_B = 32,
    LED_5_R = 35,
    LED_5_G = 34,
    LED_5_B = 33,
    LED_6_R = 46,
    LED_6_G = 47,
    LED_6_B = 36,
    LED_7_R = 43,
    LED_7_G = 44,
    LED_7_B = 45,
    LED_8_R = 40,
    LED_8_G = 41,
    LED_8_B = 42,
    LED_9_R = 18,
    LED_9_G = 17,
    LED_9_B = 16,
    LED_10_R = 23,
    LED_10_G = 20,
    LED_10_B = 19,
    LED_11_R = 31,
    LED_11_G = 21,
    LED_11_B = 22,
    LED_12_R = 28,
    LED_12_G = 29,
    LED_12_B = 30,
    LED_13_R = 25,
    LED_13_G = 26,
    LED_13_B = 27,
    LED_14_R = 4,
    LED_14_G = 3,
    LED_14_B = 24,
    LED_15_R = 7,
    LED_15_G = 6,
    LED_15_B = 5,
    LED_16_R = 2,
    LED_16_G = 1,
    LED_16_B = 0,
    LED_LAST = 48,
};


static constexpr I2CHandle::Config field_led_i2c_config
    = {I2CHandle::Config::Peripheral::I2C_1,
       {{DSY_GPIOB, 8}, {DSY_GPIOB, 9}},
       I2CHandle::Config::Speed::I2C_1MHZ};



static LedDriverPca9685<3, true>::DmaBuffer DMA_BUFFER_MEM_SECTION
    field_led_dma_buffer_a,
    field_led_dma_buffer_b;

void Daisy16rgb::Init(bool boost)
{
    seed.Configure();
    seed.Init(boost);
    seed.SetAudioBlockSize(48);

    InitLeds();
    InitEncoder();



}

void Daisy16rgb::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void Daisy16rgb::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void Daisy16rgb::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void Daisy16rgb::StopAudio()
{
    seed.StopAudio();
}

void Daisy16rgb::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void Daisy16rgb::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void Daisy16rgb::SetHidUpdateRates()
{
    //set the hids to the new update rate
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob[i].SetSampleRate(AudioCallbackRate());
    }
    for(size_t i = 0; i < CV_LAST; i++)
    {
        cv[i].SetSampleRate(AudioCallbackRate());
    }
}

void Daisy16rgb::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float Daisy16rgb::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void Daisy16rgb::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t Daisy16rgb::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float Daisy16rgb::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}


void Daisy16rgb::StartAdc()
{
    seed.adc.Start();
}

void Daisy16rgb::StopAdc()
{
    seed.adc.Stop();
}

/** Turns on the built-in 12-bit DAC on the Daisy Seed */
void Daisy16rgb::StartDac()
{
    //dsy_dac_start(DSY_DAC_CHN_BOTH);
}

void Daisy16rgb::ProcessAnalogControls()
{
   
}

void Daisy16rgb::ProcessDigitalControls()
{

    encoder.Debounce();
}



float Daisy16rgb::GetKnobValue(size_t idx) const
{
    return knob[idx < KNOB_LAST ? idx : 0].Value();
}

float Daisy16rgb::GetCvValue(size_t idx) const
{
    return cv[idx < CV_LAST ? idx : 0].Value();
}

Switch* Daisy16rgb::GetSwitch(size_t idx)
{
    return &sw[idx < SW_LAST ? idx : 0];
}

AnalogControl* Daisy16rgb::GetKnob(size_t idx)
{
    return &knob[idx < KNOB_LAST ? idx : 0];
}

AnalogControl* Daisy16rgb::GetCv(size_t idx)
{
    return &cv[idx < CV_LAST ? idx : 0];
}


void Daisy16rgb::VegasMode()
{
        size_t deltime = 30;
        
        for(size_t i = 0; i < RING_LED_LAST; i++)
        {
           ClearLeds();
           SetRingLed(static_cast<RingLed>(i),1.f,0,0);
           UpdateLeds();
           DelayMs(deltime);
        }
        for(size_t i = 0; i < RING_LED_LAST; i++)
        {
           ClearLeds();
           SetRingLed(static_cast<RingLed>(i),0,1.f,0);
           UpdateLeds();
           DelayMs(deltime);
        }
        for(size_t i = 0; i < RING_LED_LAST; i++)
        {
           ClearLeds();
           SetRingLed(static_cast<RingLed>(i),0,0,1.f);
           UpdateLeds();
           DelayMs(deltime);
        }
        
        
        
        
}

void Daisy16rgb::ClearLeds()
{
    
    for(size_t i = 0; i < RING_LED_LAST; i++)
    {
        SetRingLed(static_cast<RingLed>(i), 0.0f, 0.0f, 0.0f);
    }
    
}

void Daisy16rgb::UpdateLeds()
{
    led_driver.SwapBuffersAndTransmit();
}

void Daisy16rgb::SetRingLed(RingLed idx, float r, float g, float b)
{
    uint8_t r_addr[RING_LED_LAST] = {LED_1_R,
                                     LED_2_R,
                                     LED_3_R,
                                     LED_4_R,
                                     LED_5_R,
                                     LED_6_R,
                                     LED_7_R,
                                     LED_8_R,
                                     LED_9_R,
                                     LED_10_R,
                                     LED_11_R,
                                     LED_12_R,
                                     LED_13_R,
                                     LED_14_R,
                                     LED_15_R,
                                     LED_16_R};
    uint8_t g_addr[RING_LED_LAST] = {LED_1_G,
                                     LED_2_G,
                                     LED_3_G,
                                     LED_4_G,
                                     LED_5_G,
                                     LED_6_G,
                                     LED_7_G,
                                     LED_8_G,
                                     LED_9_G,
                                     LED_10_G,
                                     LED_11_G,
                                     LED_12_G,
                                     LED_13_G,
                                     LED_14_G,
                                     LED_15_G,
                                     LED_16_G};
    uint8_t b_addr[RING_LED_LAST] = {LED_1_B,
                                     LED_2_B,
                                     LED_3_B,
                                     LED_4_B,
                                     LED_5_B,
                                     LED_6_B,
                                     LED_7_B,
                                     LED_8_B,
                                     LED_9_B,
                                     LED_10_B,
                                     LED_11_B,
                                     LED_12_B,
                                     LED_13_B,
                                     LED_14_B,
                                     LED_15_B,
                                     LED_16_B};

    led_driver.SetLed(r_addr[idx], r);
    led_driver.SetLed(g_addr[idx], g);
    led_driver.SetLed(b_addr[idx], b);
 
    
}

void Daisy16rgb::InitLeds()
{
    // LEDs are on the LED Driver.
    // LEDs
    // 3x PCA9685 addresses 0x00, and 0x02
    uint8_t   addr[3] = {0x00, 0x01, 0x02};
    I2CHandle i2c;
    i2c.Init(field_led_i2c_config);
    led_driver.Init(i2c, addr, field_led_dma_buffer_a, field_led_dma_buffer_b);


    ClearLeds();
    UpdateLeds();
}

void Daisy16rgb::InitEncoder()
{
    dsy_gpio_pin a, b, click;
    a     = seed.GetPin(ENC_A_PIN);
    b     = seed.GetPin(ENC_B_PIN);
    click = seed.GetPin(ENC_CLICK_PIN);
    encoder.Init(a, b, click);
}