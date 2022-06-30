#include "../daisy_saul.h"
#include "daisysp.h"
#include "granular_processor.h"

//#define SHOW_KNOB_VALUES
#define TOGGLE_FREEZE_ON_HIGH //as opposed to Freezing only while the gate is open

#define AUDIO_BLOCK_SIZE 32 //DO NOT CHANGE!

#define MAIN_LOOP_DELAY 6                      //milliseconds
#define CV_FREEZE_UPDATE_DEBOUNCE_INTERVAL 500 //milliseconds
#define CV_FREEZE_TRIGGER_THRESHOLD 0.65f //TODO: make these last 2 parameters

#define NUM_KNOBS 9
#define NUM_PARAMS 9

#define KNOB_CHANGE_TOLERANCE .001f
#define KNOB_CATCH_TOLERANCE .075f

//#define PARAM_BUFFER_SIZE 8

using namespace daisysp;
using namespace daisy;

enum DEVICE_STATE
{
    RUNNING,
    CV_MAPPING,
};


enum
{
    PLAYBACK_QUAL_16B_ST = 0,
    PLAYBACK_QUAL_16B_MO = 1,
    PLAYBACK_QUAL_8B_ST  = 2,
    PLAYBACK_QUAL_8B_MO  = 3,
};

enum
{
    LED_PLAYBACK_QUAL_16B_ST        = 0,
    LED_PLAYBACK_QUAL_16B_MO        = 1,
    LED_PLAYBACK_QUAL_8B_ST         = 2,
    LED_PLAYBACK_QUAL_8B_MO         = 3,
    LED_MAP_CV_2                    = 4,
    LED_MAP_CV_3                    = 5,
    LED_MAP_CV_4                    = 6,
    LED_SHIFT                       = 7,
    LED_BYPASS                      = 8,
    LED_SILENCE                     = 9,
    LED_FREEZE                      = 10,
    LED_PLAYBACK_MODE_SPECTRAL      = 12,
    LED_PLAYBACK_MODE_LOOPING_DELAY = 13,
    LED_PLAYBACK_MODE_STRETCH       = 14,
    LED_PLAYBACK_MODE_GRANULAR      = 15,
};

enum
{
    BUTTON_PLAYBACK_MODE_GRANULAR      = 0,
    BUTTON_PLAYBACK_MODE_STRETCH       = 1,
    BUTTON_PLAYBACK_MODE_LOOPING_DELAY = 2,
    BUTTON_PLAYBACK_MODE_SPECTRAL      = 3,
    BUTTON_FREEZE                      = 5,
    BUTTON_SILENCE                     = 6,
    BUTTON_BYPASS                      = 7,
    BUTTON_PLAYBACK_QUAL_16B_ST        = 8,
    BUTTON_PLAYBACK_QUAL_16B_MO        = 9,
    BUTTON_PLAYBACK_QUAL_8B_ST         = 10,
    BUTTON_PLAYBACK_QUAL_8B_MO         = 11,
    BUTTON_MAP_CV_2                    = 12,
    BUTTON_MAP_CV_3                    = 13,
    BUTTON_MAP_CV_4                    = 14,
    BUTTON_SHIFT                       = 15,
};

enum MAPPABLE_CVS
{
    NONE,
    CV2,
    CV3,
    CV4,
};

inline float Constrain(float var, float min, float max)
{
    if(var < min)
    {
        var = min;
    }
    else if(var > max)
    {
        var = max;
    }
    return var;
}

class ParamControl
{
  public:
    ParamControl() {}
    ~ParamControl() {}

    void Init(AnalogControl* knob,
              Parameters*    params,
              int            param_num,
              MAPPABLE_CVS   mapped_cv = NONE)
    {
        knob_             = knob;
        params_           = params;
        param_num_        = param_num;
        param_val_        = 0.5f;
        mapped_cv_        = mapped_cv;
        knob_val_         = 0.f;
        knob_val_changed_ = false;
    }

    bool HasParamChanged() { return param_val_changed_; }
    bool HasKnobChanged() { return knob_val_changed_; }

    bool ControlChange(float newval, bool catch_val = true)
    {
        auto delta = fabsf(newval - param_val_);
        auto ret
            = delta > KNOB_CHANGE_TOLERANCE && delta < KNOB_CATCH_TOLERANCE;

        if(ret)
        {
            param_val_         = newval;
            param_val_changed_ = true;

#ifdef SHOW_KNOB_VALUES
            //snprintf(val_str, PARAM_BUFFER_SIZE, "%d", int(param_val_ * 100));
#endif
        }
        else
        {
            param_val_changed_ = false;
        };
        return ret;
    }

    float       GetValue() { return param_val_; }
    int         GetParamNum() { return param_num_; }

    MAPPABLE_CVS GetMappedCV() { return mapped_cv_; }
    void         SetMappedCV(MAPPABLE_CVS mapped_cv) { mapped_cv_ = mapped_cv; }

    void Process()
    {
        float val;

        auto new_knob_val = knob_->Process();
        knob_val_changed_
            = fabsf(new_knob_val - knob_val_) > KNOB_CHANGE_TOLERANCE;
        knob_val_ = new_knob_val;

        if(mapped_cv_ == NONE)
        {
            val = new_knob_val;

            if(!ControlChange(val))
            {
                return;
            }
        }
        else
        {
            //Ignore the knob setting if parameter is mapped to a CV since param_val will be set by the CV
            //TODO: Use the knob as an offset to the incoming CV value
            val = param_val_;
        }

        switch(param_num_)
        {
            case 0: params_->position = val; break;
            case 1: params_->size = val; break;
            case 2:
                params_->pitch = powf(9.798f * (val - .5f), 2.f);
                params_->pitch *= val < .5f ? -1.f : 1.f;
                break;
            case 3: params_->density = val; break;
            case 4: params_->texture = val; break;
            case 5: params_->dry_wet = val; break;
            case 6: params_->stereo_spread = val; break;
            case 7: params_->feedback = val; break;
            case 8: params_->reverb = val; break;
        }
    }

  private:
    AnalogControl* knob_;
    Parameters*    params_;
    int            param_num_;
    float          param_val_;
    bool           param_val_changed_;
    float          knob_val_;
    bool           knob_val_changed_;

    MAPPABLE_CVS mapped_cv_;
};

GranularProcessorClouds processor;
DaisySaul              saul;

// Pre-allocate big blocks in main memory and CCM. No malloc here.
uint8_t block_mem[118784];
uint8_t block_ccm[65536 - 128];


bool         param_val_changed_;
ParamControl param_controls[NUM_PARAMS];
Parameters*  parameters;
DEVICE_STATE current_device_state = RUNNING;
MAPPABLE_CVS currently_mapping_cv = NONE;
bool         can_map[4]           = {true};
bool is_silenced, is_bypassed, is_frozen_by_button, is_frozen_by_cv;

uint32_t last_freeze_cv_update;

void Controls();
void UpdateLEDs();
void ProcessButtons();

int Mod(int n, int m)
{
    return ((n % m) + m) % m;
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    
    Controls();

    FloatFrame input[size];
    FloatFrame output[size];

    for(size_t i = 0; i < size; i++)
    {
        input[i].l  = in[0][i] * .5f;
        input[i].r  = in[1][i] * .5f;
        output[i].l = output[i].r = 0.f;
    }

    processor.Process(input, output, size);

    for(size_t i = 0; i < size; i++)
    {
        out[0][i]       = output[i].l;
        out[1][i]       = output[i].r;
    }
}

void InitParams()
{
    param_controls[0].Init(&saul.knob[8],
                           parameters,
                           0);

    param_controls[1].Init(&saul.knob[9],
                           parameters,
                           1);

    param_controls[2].Init(&saul.knob[10],
                           parameters,
                           2);

    param_controls[3].Init(&saul.knob[1],
                           parameters,
                           3);

    param_controls[4].Init(&saul.knob[2],
                           parameters,
                           4);

    param_controls[5].Init(&saul.knob[4],
                           parameters,
                           5);

    param_controls[6].Init(&saul.knob[5],
                           parameters,
                           6);

    param_controls[7].Init(&saul.knob[6],
                           parameters,
                           7);

    param_controls[8].Init(&saul.knob[7],
                           parameters,
                           8);
}

int main(void)
{
    saul.Init();
    saul.SetAudioBlockSize(AUDIO_BLOCK_SIZE);
    float sample_rate = saul.AudioSampleRate();

    //init the luts
    InitResources(sample_rate);

    processor.Init(sample_rate,
                   block_mem,
                   sizeof(block_mem),
                   block_ccm,
                   sizeof(block_ccm));

    parameters = processor.mutable_parameters();

    processor.set_playback_mode(PLAYBACK_MODE_GRANULAR);
    processor.set_quality(0);

    InitParams();

    //Process all params once to set inital state
    for(int i = NUM_PARAMS - 1; i >= 0; i--)
    {
        param_controls[i].Process();
    }

    //Delay for a second to show the splash screen
    System::Delay(1000);

    saul.StartAdc();
    saul.StartAudio(AudioCallback);

    while(1)
    {
        processor.Prepare();

        UpdateLEDs();

        //And we probably dont need to call Prepare so often so we can sleep a bit
        System::Delay(MAIN_LOOP_DELAY);
    }
}



void UpdateLEDs()
{
    

    auto currentPlaybackMode = processor.playback_mode();
/*
    field.led_driver.SetLed(
        LED_PLAYBACK_MODE_GRANULAR,
        currentPlaybackMode == PLAYBACK_MODE_GRANULAR ? 1.f : 0.5f);
    field.led_driver.SetLed(
        LED_PLAYBACK_MODE_STRETCH,
        currentPlaybackMode == PLAYBACK_MODE_STRETCH ? 1.f : 0.5f);
    field.led_driver.SetLed(
        LED_PLAYBACK_MODE_LOOPING_DELAY,
        currentPlaybackMode == PLAYBACK_MODE_LOOPING_DELAY ? 1.f : 0.5f);
    field.led_driver.SetLed(
        LED_PLAYBACK_MODE_SPECTRAL,
        currentPlaybackMode == PLAYBACK_MODE_SPECTRAL ? 1.f : 0.5f);

    auto currentPlaybackQuality = processor.quality();

    field.led_driver.SetLed(
        LED_PLAYBACK_QUAL_16B_ST,
        currentPlaybackQuality == PLAYBACK_QUAL_16B_ST ? 1.f : 0.5f);
    field.led_driver.SetLed(
        LED_PLAYBACK_QUAL_16B_MO,
        currentPlaybackQuality == PLAYBACK_QUAL_16B_MO ? 1.f : 0.5f);
    field.led_driver.SetLed(
        LED_PLAYBACK_QUAL_8B_ST,
        currentPlaybackQuality == PLAYBACK_QUAL_8B_ST ? 1.f : 0.5f);
    field.led_driver.SetLed(
        LED_PLAYBACK_QUAL_8B_MO,
        currentPlaybackQuality == PLAYBACK_QUAL_8B_MO ? 1.f : 0.5f);

    field.led_driver.SetLed(LED_MAP_CV_2,
                            current_device_state == CV_MAPPING
                                    && currently_mapping_cv == CV2
                                ? 1.f
                                : 0.f);
    field.led_driver.SetLed(LED_MAP_CV_3,
                            current_device_state == CV_MAPPING
                                    && currently_mapping_cv == CV3
                                ? 1.f
                                : 0.f);
    field.led_driver.SetLed(LED_MAP_CV_4,
                            current_device_state == CV_MAPPING
                                    && currently_mapping_cv == CV4
                                ? 1.f
                                : 0.f);

    field.led_driver.SetLed(LED_FREEZE, processor.frozen() ? 1.f : 0.5f);
    field.led_driver.SetLed(LED_SILENCE, is_silenced ? 1.f : 0.5f);
    field.led_driver.SetLed(LED_BYPASS, is_bypassed ? 1.f : 0.5f);
    field.led_driver.SetLed(LED_SHIFT, is_shifted ? 1.f : 0.5f);

    field.led_driver.SwapBuffersAndTransmit();
*/
}

void ProcessParam(ParamControl& pc, bool auto_page_change)
{
    pc.Process();

    switch(current_device_state)
    {
        case RUNNING:
            if(pc.HasKnobChanged())
            {
                //If control is mapped to CV then don't change the page when its value is changed
                if(pc.GetMappedCV() == NONE && auto_page_change)
                {
                    
                }
            }
            break;

        case CV_MAPPING:
            if(can_map[currently_mapping_cv] == true)
            {
                //The knob has changed from the last iteration so we are mapping
                if(pc.HasKnobChanged())
                {
                    //If control has not been mapped yet or mapped to another CV then map it to the new CV
                    if(pc.GetMappedCV() != currently_mapping_cv)
                    {
                        pc.SetMappedCV(currently_mapping_cv);
                        //Only allow one mapping action per key press
                        can_map[currently_mapping_cv] = false;
                    }
                    else
                    {
                        //If the control is already mapped and the knob has been moved then unmap it
                        pc.SetMappedCV(NONE);
                        //Only allow one mapping action per key press
                        can_map[currently_mapping_cv] = false;
                    }
                }
            }
            break;

        default: break;
    }
}

void ProcessParams(bool auto_page_change = true)
{
    for(int i = NUM_PARAMS - 1; i >= 0; i--)
    {
        
            ProcessParam(param_controls[i], auto_page_change);
       
    }
}

void ProcessButtons()
{
    /*
    if(field.KeyboardRisingEdge(BUTTON_PLAYBACK_QUAL_16B_ST))
    {
        processor.set_quality(PLAYBACK_QUAL_16B_ST);
        
    }

    if(field.KeyboardRisingEdge(BUTTON_PLAYBACK_QUAL_16B_MO))
    {
        processor.set_quality(PLAYBACK_QUAL_16B_MO);
       
    }

    if(field.KeyboardRisingEdge(BUTTON_PLAYBACK_QUAL_8B_ST))
    {
        processor.set_quality(PLAYBACK_QUAL_8B_ST);
       
    }

    if(field.KeyboardRisingEdge(BUTTON_PLAYBACK_QUAL_8B_MO))
    {
        processor.set_quality(PLAYBACK_QUAL_8B_MO);
        
    }

    if(field.KeyboardRisingEdge(BUTTON_PLAYBACK_MODE_GRANULAR))
    {
        processor.set_playback_mode(PLAYBACK_MODE_GRANULAR);
        
    }

    if(field.KeyboardRisingEdge(BUTTON_PLAYBACK_MODE_STRETCH))
    {
        processor.set_playback_mode(PLAYBACK_MODE_STRETCH);
        
    }

    if(field.KeyboardRisingEdge(BUTTON_PLAYBACK_MODE_LOOPING_DELAY))
    {
        processor.set_playback_mode(PLAYBACK_MODE_LOOPING_DELAY);
        
    }

    if(field.KeyboardRisingEdge(BUTTON_PLAYBACK_MODE_SPECTRAL))
    {
        processor.set_playback_mode(PLAYBACK_MODE_SPECTRAL);
       
    }

    if(field.KeyboardRisingEdge(BUTTON_FREEZE))
    {
        is_frozen_by_button = !is_frozen_by_button;
        processor.ToggleFreeze();
        
    }

    if(field.KeyboardRisingEdge(BUTTON_SILENCE))
    {
        is_silenced = !is_silenced;
        processor.set_silence(is_silenced);
        
    }

    if(field.KeyboardRisingEdge(BUTTON_BYPASS))
    {
        is_bypassed = !is_bypassed;
        processor.set_bypass(is_bypassed);
        
    }

    

    if(field.sw[0].RisingEdge())
    {
       
    }

    if(field.sw[1].RisingEdge())
    {
       
    }

    if(field.KeyboardRisingEdge(BUTTON_MAP_CV_2))
    {
        current_device_state = CV_MAPPING;
        currently_mapping_cv = CV2;
        can_map[CV2]         = true;
        
    }

    if(field.KeyboardFallingEdge(BUTTON_MAP_CV_2))
    {
        current_device_state = RUNNING;
        currently_mapping_cv = NONE;
    }

    if(field.KeyboardRisingEdge(BUTTON_MAP_CV_3))
    {
        current_device_state = CV_MAPPING;
        currently_mapping_cv = CV3;
        can_map[CV3]         = true;
        
    }

    if(field.KeyboardFallingEdge(BUTTON_MAP_CV_3))
    {
        current_device_state = RUNNING;
        currently_mapping_cv = NONE;
    }

    if(field.KeyboardRisingEdge(BUTTON_MAP_CV_4))
    {
        current_device_state = CV_MAPPING;
        currently_mapping_cv = CV4;
        can_map[CV4]         = true;
       
    }

    if(field.KeyboardFallingEdge(BUTTON_MAP_CV_4))
    {
        current_device_state = RUNNING;
        currently_mapping_cv = NONE;
    }

    processor.set_silence(is_silenced);
    processor.set_bypass(is_bypassed);
    */
}

void ProcessGatesTriggersCv()
{
    //Using CV1 in as a gate to freeze and unfreeze the processor
    //0.7f should map to 3.5 volts for HIGH state
    //Debounced
    if(!is_frozen_by_button)
    {
#ifdef TOGGLE_FREEZE_ON_HIGH
        //Has the debounce interval elapsed? If not then we just disregard this value
        if(System::GetNow()
           > last_freeze_cv_update + CV_FREEZE_UPDATE_DEBOUNCE_INTERVAL)
        {
            auto  cv1               = saul.GetCv(saul.CV_0);
            float new_freeze_cv_val = cv1->Process();
            if(new_freeze_cv_val > CV_FREEZE_TRIGGER_THRESHOLD)
            {
                //Toggle freeze for the processor if the gate is held high
                is_frozen_by_cv    = !is_frozen_by_cv;
                parameters->freeze = is_frozen_by_cv;
            }
            last_freeze_cv_update = System::GetNow();
        }
#else
        auto  cv1               = field.GetCv(field.CV_1);
        float new_freeze_cv_val = cv1->Process();
        //Only freeze the processor while the gate is held high
        parameters->freeze = is_frozen_by_cv
            = (new_freeze_cv_val > GATE_TRIGGER_THRESHOLD);
#endif
    }
    else
    {
        is_frozen_by_cv = false;
    }

    parameters->trigger = saul.gate.Trig();
    parameters->gate    = saul.Gate();

    //Send CV to all the mapped parameters
    for(int i = 0; i < NUM_PARAMS; i++)
    {
        if(param_controls[i].GetMappedCV() != NONE)
        {
            float cv_value = saul.GetCvValue(param_controls[i].GetMappedCV());
            float clamped_cv_value = Constrain(cv_value, 0.0f, 1.0f);
            param_controls[i].ControlChange(clamped_cv_value);
            param_controls[i].Process();
        }
    }
}

void Controls()
{
    saul.ProcessAllControls();
    ProcessParams();
    ProcessGatesTriggersCv();
    ProcessButtons();
}