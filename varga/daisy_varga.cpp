#include "daisy_varga.h"

using namespace daisy;
using namespace varga;


// VARGA Pins
#define PIN_CLOCK_IN 26
#define PIN_SW_MODEL 14
#define PIN_SW_EVIL 13
#define PIN_LED1_R 27
#define PIN_LED1_G 28
#define PIN_LED1_B 15
#define PIN_LED2_R 10
#define PIN_LED2_G 9
#define PIN_LED2_B 8
#define PIN_POT_FREQ 16
#define PIN_POT_HARM 17
#define PIN_POT_TIMBRE 18
#define PIN_POT_MORPH 19
#define PIN_CV_FM 20
#define PIN_CV_TIMBRE 21
#define PIN_CV_MORPH 22
#define PIN_CV_VOCT 23
#define PIN_CV_HARM 24
#define PIN_CV_MODEL 25




void DaisyVarga::Init(bool boost)
{
    seed.Configure();
    seed.Init(boost);
    //seed.SetAudioBlockSize(48);

    
    // Push Buttons
    uint8_t s_pin[] = {PIN_SW_MODEL,
                         PIN_SW_EVIL};
   
    uint8_t ledr_pin[]    = {PIN_LED1_R, PIN_LED2_R};
    uint8_t ledg_pin[]    = {PIN_LED1_G, PIN_LED2_G};
    uint8_t ledb_pin[]    = {PIN_LED1_B, PIN_LED2_B};
    

    // push buttons
    for(size_t i = 0; i < S_LAST; i++)
    {
        s[i].Init(seed.GetPin(s_pin[i]));
    }

    // gate in 
    dsy_gpio_pin gate_gpio = seed.GetPin(PIN_CLOCK_IN);
    gate.Init(&gate_gpio,true);


    // ADCs
    AdcChannelConfig adc_cfg[KNOB_LAST + CV_LAST];

    // POT/CV MUX
    adc_cfg[0].InitSingle(seed.GetPin(PIN_POT_FREQ));
    adc_cfg[1].InitSingle(seed.GetPin(PIN_POT_HARM));
    adc_cfg[2].InitSingle(seed.GetPin(PIN_POT_TIMBRE));
    adc_cfg[3].InitSingle(seed.GetPin(PIN_POT_MORPH));

    adc_cfg[4].InitSingle(seed.GetPin(PIN_CV_FM));
    adc_cfg[5].InitSingle(seed.GetPin(PIN_CV_TIMBRE));
    adc_cfg[6].InitSingle(seed.GetPin(PIN_CV_MORPH));
    adc_cfg[7].InitSingle(seed.GetPin(PIN_CV_VOCT));
    adc_cfg[8].InitSingle(seed.GetPin(PIN_CV_HARM));
    adc_cfg[9].InitSingle(seed.GetPin(PIN_CV_MODEL));
      
    seed.adc.Init(adc_cfg,KNOB_LAST + CV_LAST,daisy::AdcHandle::OVS_128);
 
    for(size_t i = 0; i < KNOB_LAST + CV_LAST; i++) {
        if(i < KNOB_LAST) {
            knob[i].Init(seed.adc.GetPtr(i), AudioCallbackRate()); 
        } else if(i > (KNOB_LAST-1) && i < (KNOB_LAST + CV_LAST)) {
            cv[i-KNOB_LAST].InitBipolarCv(seed.adc.GetPtr(i), AudioCallbackRate()); 
        } 
    }
    


    // RGB LEDs
    for(size_t i = 0; i < LED_LAST; i++)
    {
        dsy_gpio_pin r = seed.GetPin(ledr_pin[i]);
        dsy_gpio_pin g = seed.GetPin(ledg_pin[i]);
        dsy_gpio_pin b = seed.GetPin(ledb_pin[i]);
        leds[i].Init(r, g, b, true);
    }

    LoadCalibrationData();
    

}

void DaisyVarga::SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate sr)
{
    seed.SetAudioSampleRate(sr);
    SetHidUpdateRates();
}

float DaisyVarga::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisyVarga::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyVarga::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisyVarga::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyVarga::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyVarga::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyVarga::StopAudio()
{
    seed.StopAudio();
}

void DaisyVarga::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyVarga::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyVarga::StartAdc()
{
    seed.adc.Start();
}
void DaisyVarga::StopAdc()
{
    seed.adc.Stop();
}

void DaisyVarga::ProcessAnalogControls()
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

Switch* DaisyVarga::GetSwitch(size_t idx)
{
    return &s[idx < S_LAST ? idx : 0];
}


bool DaisyVarga::Gate()
{
    return !gate.State();
}

void DaisyVarga::ProcessDigitalControls()
{
    // Switches
    for(size_t i = 0; i < S_LAST; i++)
    {
        s[i].Debounce();
        
    }
}

float DaisyVarga::GetKnobValue(int idx) const
{
    //return (knob[idx].Value());
    return knob[idx < KNOB_LAST ? idx : 0].Value();
}
float DaisyVarga::GetCvValue(int idx) const 
{
    if(idx != CV_VOCT)
        return cv[idx < CV_LAST ? idx : 0].Value() - cv_offsets_[idx < CV_LAST ? idx : 0];
    else
        return cv[idx < CV_LAST ? idx : 0].Value();
}

AnalogControl* DaisyVarga::GetCv(size_t idx)
{
    return &cv[idx < CV_LAST ? idx : 0];
}
AnalogControl* DaisyVarga::GetKnob(size_t idx)
{
    return &knob[idx < KNOB_LAST ? idx : 0];
}

/** @brief Return a MIDI note number value from -60 to 60 corresponding to 
 *      the -5V to 5V input range of the Warp CV input.
 */
float DaisyVarga::GetWarpVoct()
{
    return voct_cal.ProcessInput(cv[CV_VOCT].Value());
}

void DaisyVarga::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyVarga::SetHidUpdateRates()
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


void DaisyVarga::SetLed(size_t idx, float red, float green, float blue)
{
    leds[idx].Set(red, green, blue);
}


void DaisyVarga::UpdateLeds()
{
    for(size_t i = 0; i < LED_LAST; i++)
    {
        leds[i].Update();
    }
}
void DaisyVarga::ClearLeds()
{
    for(size_t i = 0; i < LED_LAST; i++)
    {
        leds[i].Set(0,0,0);
    }
}


void DaisyVarga::SetRGBColor (size_t idx, Colors color)
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

float DaisyVarga::CVKnobCombo(float CV_Val,float Pot_Val)
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
void DaisyVarga::LoadCalibrationData()
{
    daisy::PersistentStorage<CalibrationData> cal_storage(seed.qspi);
    CalibrationData                           default_cal;
    cal_storage.Init(default_cal, FLASH_BLOCK);
    auto &cal_data = cal_storage.GetSettings();
    SetWarpCalData(cal_data.warp_scale, cal_data.warp_offset);
    SetCvOffsetData(cal_data.cv_offset);
}