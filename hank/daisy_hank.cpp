#include "daisy_hank.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE /**< & */
#endif

using namespace daisy;


// HANK Pins
#define PIN_POT1 22
#define PIN_POT2 23
#define PIN_POT3 24
#define PIN_POT4 25

#define PIN_CV1 28



#define PIN_S1 0

#define PIN_TRIG_IN 1

#define PIN_RGB1_R 2
#define PIN_RGB1_G 3
#define PIN_RGB1_B 4
#define PIN_RGB2_R 5
#define PIN_RGB2_G 6
#define PIN_RGB2_B 7
#define PIN_RGB3_R 8
#define PIN_RGB3_G 9
#define PIN_RGB3_B 10
#define PIN_RGB4_R 11
#define PIN_RGB4_G 26
#define PIN_RGB4_B 27



void DaisyHank::Init(bool boost)
{
    seed.Configure();
    seed.Init(boost);
    seed.SetAudioBlockSize(48);

    s1.Init(seed.GetPin(PIN_S1));

   uint8_t ledr_pin[]    = {PIN_RGB1_R, PIN_RGB2_R, PIN_RGB3_R, PIN_RGB4_R};
    uint8_t ledg_pin[]    = {PIN_RGB1_G, PIN_RGB2_G, PIN_RGB3_G, PIN_RGB4_G};
    uint8_t ledb_pin[]    = {PIN_RGB1_B, PIN_RGB2_B, PIN_RGB3_B, PIN_RGB4_B};

    // gate in 
    dsy_gpio_pin gateIn1_gpio = seed.GetPin(PIN_TRIG_IN);
    gate_in1.Init(&gateIn1_gpio,false);


    // ADCs
    AdcChannelConfig adc_cfg[5];

    // POT/CV MUX
    adc_cfg[0].InitSingle(seed.GetPin(PIN_POT1));
    adc_cfg[1].InitSingle(seed.GetPin(PIN_POT2));
    adc_cfg[2].InitSingle(seed.GetPin(PIN_POT3));
    adc_cfg[3].InitSingle(seed.GetPin(PIN_POT4));
    adc_cfg[4].InitSingle(seed.GetPin(PIN_CV1));
    
    seed.adc.Init(adc_cfg,5);
 
    //seed.adc.Start();

    for(size_t i = 0; i < (KNOB_LAST + CV_LAST); i++) {
        if(i < KNOB_LAST) {
            knob[i].Init(seed.adc.GetPtr(i), AudioCallbackRate()); 
        } else {
            cv[i-KNOB_LAST].InitBipolarCv(seed.adc.GetPtr(i), AudioCallbackRate()); 
        } 
    } 

    // RGB LEDs
    for(size_t i = 0; i < RGB_LED_LAST; i++)
    {
        dsy_gpio_pin r = seed.GetPin(ledr_pin[i]);
        dsy_gpio_pin g = seed.GetPin(ledg_pin[i]);
        dsy_gpio_pin b = seed.GetPin(ledb_pin[i]);
        rgb[i].Init(r, g, b, true);
    }

     LoadCalibrationData();   
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

/** @brief Return a MIDI note number value from -60 to 60 corresponding to 
 *      the -5V to 5V input range of the Warp CV input.
 */
float DaisyHank::GetWarpVoct()
{
    return voct_cal.ProcessInput(cv[CV_1].Value());
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

void DaisyHank::SetLed(size_t idx, float red, float green, float blue)
{
    rgb[idx].Set(red, green, blue);
}

void DaisyHank::SetRGBColor (size_t idx, Colors color)
{
    
    switch (color)
    {
    case RED:
        SetLed(idx,1.f,0.f,0.f);
        break;
    case GREEN:
        SetLed(idx,0.f,0.7f,0.f);
        break;
    case BLUE:
        SetLed(idx,0.f,0.f,1.f);
        break;
    case YELLOW:
        SetLed(idx,1.f,0.7f,0.f);
        break;
    case CYAN:
        SetLed(idx,0.f,1.f,1.f);
        break;
    case PURPLE:
        SetLed(idx,1.f,0.f,1.f);
        break;
    case ORANGE:
        SetLed(idx,1.f,0.3f,0.f);
        break;
    case DARKGREEN:
        SetLed(idx,0.f,0.2f,0.f);
        break;
    case DARKBLUE:
        SetLed(idx,0.2f,0.2f,0.6f);
        break;
    case DARKRED:
        SetLed(idx,0.4f,0.f,0.f);
        break;
    case TURQ:
        SetLed(idx,0.f,0.5f,0.5f);
        break;
    case GREY:
        SetLed(idx,0.75f,0.75f,0.75f);
        break;
    case DARKORANGE:
        SetLed(idx,0.5f,0.2f,0.f);
        break;
    case WHITE:
        SetLed(idx,1.f,1.f,1.f);
        break;
    case OFF:
        SetLed(idx,0.f,0.f,0.f);
        break;
    }
    
}





void DaisyHank::ClearLeds()
{
    
    for(size_t i = 0; i < RGB_LED_LAST; i++)
    {
        rgb[i].Set(0, 0, 0);
    }
    
}

void DaisyHank::UpdateLeds()
{
    
    for(size_t i = 0; i < RGB_LED_LAST; i++)
    {
        rgb[i].Update();
    }
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

/** @brief Loads and sets calibration data */
void DaisyHank::LoadCalibrationData()
{
    daisy::PersistentStorage<CalibrationData> cal_storage(seed.qspi);
    CalibrationData                           default_cal;
    cal_storage.Init(default_cal, FLASH_BLOCK);
    auto &cal_data = cal_storage.GetSettings();
    SetWarpCalData(cal_data.warp_scale, cal_data.warp_offset);
    //SetCvOffsetData(cal_data.cv_offset);
}