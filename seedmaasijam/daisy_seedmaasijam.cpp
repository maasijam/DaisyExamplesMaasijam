#include "daisy_seedmaasijam.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE /**< & */
#endif



// Rev2 Pins
#define PIN_GATE_IN 0
#define PIN_SPI_CS 7
#define PIN_SPI_SCK 8
#define PIN_BLUE_LED 9
#define PIN_SPI_MOSI 10
#define PIN_I2C_SCL 11
#define PIN_I2C_SDA 12
#define PIN_MIDI_OUT 13
#define PIN_MIDI_IN 14
#define PIN_GATE_OUT 15
#define PIN_ADC_POT_MUX 16
#define PIN_ADC_CV_1 17
#define PIN_ADC_CV_2 18
#define PIN_MUX_SEL_2 19
#define PIN_MUX_SEL_1 20
#define PIN_MUX_SEL_0 21
#define PIN_CD4021_D1 26
#define PIN_CD4021_CS 27
#define PIN_CD4021_CLK 28
#define PIN_SW_2 29
#define PIN_SW_1 30

// IT LOOKS LIKE THESE MAY NEED TO GET SWAPPED.....
#define PIN_DAC_2 22    // Jumped on Rev2 from 24
#define PIN_DAC_1 23    // Jumped on Rev2 from 25
#define PIN_ADC_CV_4 24 // Jumped on Rev2 from 22
#define PIN_ADC_CV_3 25 // Jumped on Rev2 from 23

using namespace daisy;


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
    LED_5_R,
    LED_5_G,
    LED_5_B,
    LED_6_R,
    LED_6_G,
    LED_6_B,
    LED_7_R,
    LED_7_G,
    LED_7_B,
    LED_8_R,
    LED_8_G,
    LED_8_B,
    LED_9_R,
    LED_9_G,
    LED_9_B,
    LED_10_R,
    LED_10_G,
    LED_10_B,
    LED_11_R,
    LED_11_G,
    LED_11_B,
    LED_LAST,
};


static constexpr I2CHandle::Config field_led_i2c_config
    = {I2CHandle::Config::Peripheral::I2C_1,
       {{DSY_GPIOB, 8}, {DSY_GPIOB, 9}},
       I2CHandle::Config::Speed::I2C_1MHZ};



static LedDriverPca9685<2, true>::DmaBuffer DMA_BUFFER_MEM_SECTION
    field_led_dma_buffer_a,
    field_led_dma_buffer_b;

void DaisySeedmaasijam::Init(bool boost)
{
    seed.Configure();
    seed.Init(boost);
    seed.SetAudioBlockSize(48);

    InitLeds();

    // Switches
    //uint8_t sw_pin[]  = {PIN_SW_1, PIN_SW_2};
    uint8_t adc_pin[] = {PIN_ADC_CV_1,
                         PIN_ADC_CV_2,
                         PIN_ADC_CV_3,
                         PIN_ADC_CV_4,
                         PIN_ADC_POT_MUX};

    /*for(size_t i = 0; i < SW_LAST; i++)
    {
        dsy_gpio_pin p = seed.GetPin(sw_pin[i]);
        sw[i].Init(p);
    }*/

    // ADCs
    AdcChannelConfig adc_cfg[CV_LAST + 1];
    for(size_t i = 0; i < CV_LAST; i++)
    {
        adc_cfg[i].InitSingle(seed.GetPin(adc_pin[i]));
    }
    // POT MUX
    adc_cfg[CV_LAST].InitMux(seed.GetPin(PIN_ADC_POT_MUX),
                             8,
                             seed.GetPin(PIN_MUX_SEL_0),
                             seed.GetPin(PIN_MUX_SEL_1),
                             seed.GetPin(PIN_MUX_SEL_2));
    seed.adc.Init(adc_cfg, 5);

    // Order of pots on the hardware connected to mux.
    size_t pot_order[KNOB_LAST] = {0, 3, 1, 4, 2, 5, 6, 7};
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob[i].Init(seed.adc.GetMuxPtr(4, pot_order[i]), AudioCallbackRate());
    }
    for(size_t i = 0; i < CV_LAST; i++)
    {
        cv[i].InitBipolarCv(seed.adc.GetPtr(i), AudioCallbackRate());
    }

    // Keyboard
    ShiftRegister4021<2>::Config keyboard_cfg;
    keyboard_cfg.clk     = seed.GetPin(PIN_CD4021_CLK);
    keyboard_cfg.latch   = seed.GetPin(PIN_CD4021_CS);
    keyboard_cfg.data[0] = seed.GetPin(PIN_CD4021_D1);
    keyboard_sr_.Init(keyboard_cfg);

    // OLED
    //OledDisplay<SSD130x4WireSpi128x64Driver>::Config display_config;

    /*display_config.driver_config.transport_config.pin_config.dc
        = seed.GetPin(PIN_OLED_CMD);
    display_config.driver_config.transport_config.pin_config.reset
        = {DSY_GPIOX, 0}; // Not a real pin...

    display.Init(display_config);*/

    

    // Gate In
    dsy_gpio_pin gate_in_pin;
    gate_in_pin = seed.GetPin(PIN_GATE_IN);
    gate_in.Init(&gate_in_pin);
    // Gate Out
    gate_out.mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_out.pull = DSY_GPIO_NOPULL;
    gate_out.pin  = seed.GetPin(PIN_GATE_OUT);
    dsy_gpio_init(&gate_out);

    //midi
    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);

    DacHandle::Config cfg;
    cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    cfg.buff_state = DacHandle::BufferState::ENABLED;
    cfg.mode       = DacHandle::Mode::POLLING;
    cfg.chn        = DacHandle::Channel::BOTH;
    seed.dac.Init(cfg);


}

void DaisySeedmaasijam::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisySeedmaasijam::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisySeedmaasijam::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisySeedmaasijam::StopAudio()
{
    seed.StopAudio();
}

void DaisySeedmaasijam::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisySeedmaasijam::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisySeedmaasijam::SetHidUpdateRates()
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

void DaisySeedmaasijam::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisySeedmaasijam::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisySeedmaasijam::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisySeedmaasijam::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisySeedmaasijam::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}


void DaisySeedmaasijam::StartAdc()
{
    seed.adc.Start();
}

void DaisySeedmaasijam::StopAdc()
{
    seed.adc.Stop();
}

/** Turns on the built-in 12-bit DAC on the Daisy Seed */
void DaisySeedmaasijam::StartDac()
{
    //dsy_dac_start(DSY_DAC_CHN_BOTH);
}

void DaisySeedmaasijam::ProcessAnalogControls()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
        knob[i].Process();
    for(size_t i = 0; i < CV_LAST; i++)
        cv[i].Process();
}

void DaisySeedmaasijam::ProcessDigitalControls()
{
    // Switches
    /*for(size_t i = 0; i < SW_LAST; i++)
    {
        sw[i].Debounce();
       
    }*/
    //dsy_sr_4021_update(&keyboard_sr_);
    keyboard_sr_.Update();
    for(size_t i = 0; i < 16; i++)
    {
        uint8_t keyidx, keyoffset;
        keyoffset = i > 7 ? 8 : 0;
        keyidx    = (7 - (i % 8)) + keyoffset;
        keyboard_state_[keyidx]
            = keyboard_sr_.State(i) | (keyboard_state_[keyidx] << 1);
    }
    // Gate Input
    gate_in_trig_ = gate_in.Trig();
}

void DaisySeedmaasijam::SetCvOut1(uint16_t val)
{
    seed.dac.WriteValue(DacHandle::Channel::ONE, val);
}

void DaisySeedmaasijam::SetCvOut2(uint16_t val)
{
    seed.dac.WriteValue(DacHandle::Channel::TWO, val);
}

bool DaisySeedmaasijam::KeyboardState(size_t idx) const
{
    return keyboard_state_[idx] == 0x00;
}

bool DaisySeedmaasijam::KeyboardRisingEdge(size_t idx) const
{
    return keyboard_state_[idx] == 0x80;
}

bool DaisySeedmaasijam::KeyboardFallingEdge(size_t idx) const
{
    return keyboard_state_[idx] == 0x7F;
}

float DaisySeedmaasijam::GetKnobValue(size_t idx) const
{
    return knob[idx < KNOB_LAST ? idx : 0].Value();
}

float DaisySeedmaasijam::GetCvValue(size_t idx) const
{
    return cv[idx < CV_LAST ? idx : 0].Value();
}

Switch* DaisySeedmaasijam::GetSwitch(size_t idx)
{
    return &sw[idx < SW_LAST ? idx : 0];
}

AnalogControl* DaisySeedmaasijam::GetKnob(size_t idx)
{
    return &knob[idx < KNOB_LAST ? idx : 0];
}

AnalogControl* DaisySeedmaasijam::GetCv(size_t idx)
{
    return &cv[idx < CV_LAST ? idx : 0];
}


void DaisySeedmaasijam::VegasMode()
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

void DaisySeedmaasijam::ClearLeds()
{
    // Using Color
    //    Color c;
    //    c.Init(Color::PresetColor::OFF);
    //    for(size_t i = 0; i < RING_LED_LAST; i++)
    //    {
    //        ring_led[i].SetColor(c);
    //    }
    for(size_t i = 0; i < RING_LED_LAST; i++)
    {
        SetRingLed(static_cast<RingLed>(i), 0.0f, 0.0f, 0.0f);
        if(i == 10) {
            rgb_last_blue.Set(0.f);
        }
    }
    
}

void DaisySeedmaasijam::UpdateLeds()
{
    led_driver.SwapBuffersAndTransmit();
    rgb_last_blue.Update();
}

void DaisySeedmaasijam::SetRingLed(RingLed idx, float r, float g, float b)
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
                                     LED_11_R};
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
                                     LED_11_G};
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
                                     LED_11_B};

    led_driver.SetLed(r_addr[idx], r);
    led_driver.SetLed(g_addr[idx], g);

    if(idx == RING_LED_LAST-1 && b > 0) {
        rgb_last_blue.Set(1.f);
    } else {
        led_driver.SetLed(b_addr[idx], b);
    }

    
    
}

void DaisySeedmaasijam::InitLeds()
{
    // LEDs are on the LED Driver.
    // LEDs
    // 2x PCA9685 addresses 0x00, and 0x02
    uint8_t   addr[2] = {0x00, 0x01};
    I2CHandle i2c;
    i2c.Init(field_led_i2c_config);
    led_driver.Init(i2c, addr, field_led_dma_buffer_a, field_led_dma_buffer_b);

    rgb_last_blue.Init(seed.GetPin(PIN_BLUE_LED),true);

    ClearLeds();
    UpdateLeds();
}