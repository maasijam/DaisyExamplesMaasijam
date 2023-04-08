#include "daisy_srtest.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE /**< & */
#endif



// Rev2 Pins

#define PIN_CD4021_D1 26
#define PIN_CD4021_CS 27
#define PIN_CD4021_CLK 28


using namespace daisy;

enum LedOrder
{
    LED_1_R = 29,
    LED_1_G = 8,
    LED_1_B = -1,
    LED_2_R = 1,
    LED_2_G = 9,
    LED_2_B = -1,
    LED_3_R = 2,
    LED_3_G = 10,
    LED_3_B = -1,
    LED_4_R = 3,
    LED_4_G = 11,
    LED_4_B = -1,
    LED_5_R = 4,
    LED_5_G = 12,
    LED_5_B = -1,
    LED_6_R = 5,
    LED_6_G = 13,
    LED_6_B = -1,
    LED_7_R = 6,
    LED_7_G = 14,
    LED_7_B = -1,
    LED_8_R = 7,
    LED_8_G = 30,
    LED_8_B = -1,
    LED_LAST,
};


void DaisySrtest::Init(bool boost)
{
    seed.Configure();
    seed.Init(boost);
    seed.SetAudioBlockSize(48);

    InitLeds();
    
    // Keyboard
    ShiftRegister4021<2>::Config keyboard_cfg;
    keyboard_cfg.clk     = seed.GetPin(PIN_CD4021_CLK);
    keyboard_cfg.latch   = seed.GetPin(PIN_CD4021_CS);
    keyboard_cfg.data[0] = seed.GetPin(PIN_CD4021_D1);
    keyboard_sr_.Init(keyboard_cfg);

    

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

void DaisySrtest::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisySrtest::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisySrtest::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisySrtest::StopAudio()
{
    seed.StopAudio();
}

void DaisySrtest::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisySrtest::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisySrtest::SetHidUpdateRates()
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

void DaisySrtest::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisySrtest::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisySrtest::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisySrtest::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisySrtest::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}


void DaisySrtest::StartAdc()
{
    seed.adc.Start();
}

void DaisySrtest::StopAdc()
{
    seed.adc.Stop();
}

/** Turns on the built-in 12-bit DAC on the Daisy Seed */
void DaisySrtest::StartDac()
{
    //dsy_dac_start(DSY_DAC_CHN_BOTH);
}

void DaisySrtest::ProcessAnalogControls()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
        knob[i].Process();
    for(size_t i = 0; i < CV_LAST; i++)
        cv[i].Process();
}

void DaisySrtest::ProcessDigitalControls()
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
    
}

void DaisySrtest::SetCvOut1(uint16_t val)
{
    seed.dac.WriteValue(DacHandle::Channel::ONE, val);
}

void DaisySrtest::SetCvOut2(uint16_t val)
{
    seed.dac.WriteValue(DacHandle::Channel::TWO, val);
}

bool DaisySrtest::KeyboardState(size_t idx) const
{
    return keyboard_state_[idx] == 0x00;
}

bool DaisySrtest::KeyboardRisingEdge(size_t idx) const
{
    return keyboard_state_[idx] == 0x80;
}

bool DaisySrtest::KeyboardFallingEdge(size_t idx) const
{
    return keyboard_state_[idx] == 0x7F;
}

float DaisySrtest::GetKnobValue(size_t idx) const
{
    return knob[idx < KNOB_LAST ? idx : 0].Value();
}

float DaisySrtest::GetCvValue(size_t idx) const
{
    return cv[idx < CV_LAST ? idx : 0].Value();
}



AnalogControl* DaisySrtest::GetKnob(size_t idx)
{
    return &knob[idx < KNOB_LAST ? idx : 0];
}

AnalogControl* DaisySrtest::GetCv(size_t idx)
{
    return &cv[idx < CV_LAST ? idx : 0];
}


void DaisySrtest::VegasMode()
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

void DaisySrtest::ClearLeds()
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
        
    }
    
}

void DaisySrtest::UpdateLeds()
{
    for(size_t i = 0; i < RING_LED_LAST; i++)
    {
        //SetRingLed(static_cast<RingLed>(i), 0.0f, 0.0f, 0.0f);
        ring_led[i].Update();
    }
    
}

void DaisySrtest::SetRingLed(RingLed idx, float r, float g, float b)
{
    
    

    ring_led[idx].Set(r,g,b);

    
    
}


void DaisySrtest::InitLeds()
{
    
    

    ring_led[0].Init(seed.GetPin(LED_1_R),seed.GetPin(LED_1_G),seed.GetPin(LED_1_B),true);
    ring_led[1].Init(seed.GetPin(LED_2_R),seed.GetPin(LED_2_G),seed.GetPin(LED_2_B),true);
    ring_led[2].Init(seed.GetPin(LED_3_R),seed.GetPin(LED_3_G),seed.GetPin(LED_3_B),true);
    ring_led[3].Init(seed.GetPin(LED_4_R),seed.GetPin(LED_4_G),seed.GetPin(LED_4_B),true);
    ring_led[4].Init(seed.GetPin(LED_5_R),seed.GetPin(LED_5_G),seed.GetPin(LED_5_B),true);
    ring_led[5].Init(seed.GetPin(LED_6_R),seed.GetPin(LED_6_G),seed.GetPin(LED_6_B),true);
    ring_led[6].Init(seed.GetPin(LED_7_R),seed.GetPin(LED_7_G),seed.GetPin(LED_7_B),true);
    ring_led[7].Init(seed.GetPin(LED_8_R),seed.GetPin(LED_8_G),seed.GetPin(LED_8_B),true);

    ClearLeds();
    UpdateLeds();
}