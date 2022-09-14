#include "daisy_patch_sm.h"
#include "daisysp.h"

/** TODO: ADD CALIBRATION */

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

struct MyCalibrationData
{
    float offset, scale;

    /** Constructor sets defaults */
    MyCalibrationData() : offset(0.f), scale(60.f) {}

    /** @brief checks sameness */
    bool operator==(const MyCalibrationData &rhs)
    {
        if(scale != rhs.scale)
        {
            return false;
        }
        else if(offset != rhs.offset)
        {
            return false;
        }
        
        return true;
    }

    /** @brief Not equal operator */
    bool operator!=(const MyCalibrationData &rhs) { return !operator==(rhs); }
};

/** Order of events for state machine */
enum CalibrationProcedureState
{
    PATCH_1V,
    PATCH_3V,
    PATCH_DONE_CALIBRATING,
};

/** shorthand for our template-based storage class */
using MyStorageClass = PersistentStorage<MyCalibrationData>;


DaisyPatchSM patch;
Oscillator   osc_a, osc_b, osc_c;

/** The pieces related to calibration, and storage. */
CalibrationProcedureState cal_state;
VoctCalibration           cal;
MyStorageClass            cal_storage(patch.qspi);
MyCalibrationData         cal_data;
float                     value_1v; /**< Temporary value for 1V */

static constexpr uint32_t kCalibrationDataOffset = 4096;

/** UI elements for triggering the calibration */
Switch button;
Led    led,led2;
bool   trigger_save;

float myscale;

void LoadCalibrationData();

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    patch.ProcessAllControls();
    button.Debounce();

    /** Get Coarse, Fine, and V/OCT inputs from hardware 
     *  MIDI Note number are easy to use for defining ranges */
    float knob_coarse = patch.GetAdcValue(CV_1);
    float coarse_tune = fmap(knob_coarse, 12, 84);

    float knob_fine = patch.GetAdcValue(CV_2);
    float fine_tune = fmap(knob_fine, 0, 10);

    float cv_voct = patch.GetAdcValue(CV_5);
    

    /* VOCT CAL-------------- */
    float bright = 0.f; /**< LED Brightness */
    

    /** Handle the LED and button depending on the state of the calibration */
    switch(cal_state)
    {
        case PATCH_1V:
            if(button.RisingEdge())
            {
                value_1v  = cv_voct;
                cal_state = PATCH_3V;
            }
            /** Waiting for 1V, slow blink */
            bright = (System::GetNow() & 1023) > 511 ? 1.f : 0.f;
            break;
        case PATCH_3V:
            if(button.RisingEdge())
            {
                if(cal.Record(value_1v, cv_voct))
                {
                    /** Calibration is now complete. Let's trigger a save! */
                    trigger_save = true;
                    cal_state    = PATCH_DONE_CALIBRATING;
                    
                }
            }
            /** Waiting for 3V, fast blink */
            bright = (System::GetNow() & 255) > 127 ? 1.f : 0.f;
            break;
        case PATCH_DONE_CALIBRATING:
        default:
            /** Any other state the LED will be a short blip off, but long on */
            bright = (System::GetNow() & 2047) > 63 ? 1.f : 0.f;
            break;
    }
    /** Handle the LED */
    led.Set(bright);
    led.Update();
    
    
    /*----------------------------------------------------*/

    float voct    = cal.ProcessInput(cv_voct);
    //float voct    = fmap(cv_voct, 0, 60);

    /** Convert from MIDI note number to frequency */
    float midi_nn = fclamp(coarse_tune + fine_tune + voct, 0.f, 127.f);
    float freq_a  = mtof(midi_nn);

    /** Calculate a detune amount */
    float detune_amt = patch.GetAdcValue(CV_3);
    float freq_b     = freq_a + (0.05 * freq_a * detune_amt);
    float freq_c     = freq_a - (0.05 * freq_a * detune_amt);

    /** Set all three oscillators' frequencies */
    osc_a.SetFreq(freq_a);
    osc_b.SetFreq(freq_b);
    osc_c.SetFreq(freq_c);

    /** Process each sample of the oscillator and send to the hardware outputs */
    for(size_t i = 0; i < size; i++)
    {
        float sig = osc_a.Process() + osc_b.Process() + osc_c.Process();
        OUT_L[i]  = sig;
        OUT_R[i]  = sig;
    }
}

/** @brief Return a MIDI note number value from -60 to 60 corresponding to 
     *      the -5V to 5V input range of the Warp CV input.
     */
    inline float GetVoct()
    {
        return cal.ProcessInput(patch.GetAdcValue(CV_5));
    }

    /** @brief Sets the calibration data for 1V/Octave over Warp CV 
     *  typically set after reading stored data from external memory.
     */
    inline void SetCalData(float scale, float offset)
    {
        cal.SetData(scale, offset);
    }

    /** @brief Gets the current calibration data for 1V/Octave over Warp CV 
     *  typically used to prepare data for storing after successful calibration
     */
    inline void GetCalData(float &scale, float &offset)
    {
        cal.GetData(scale, offset);
    }

int main(void)
{
    patch.Init();

    button.Init(patch.B7, patch.AudioCallbackRate());
    led.Init(patch.B8, false);
    led2.Init(patch.D1, false);
    float bright2 = 0.f; 

    MyCalibrationData cal_defaults;
    cal_storage.Init(cal_defaults,kCalibrationDataOffset);
    cal_state    = PATCH_1V;
    trigger_save = false;

    /** Restore settings from previous power cycle */
    if(cal_storage.GetState() == MyStorageClass::State::FACTORY)
    {
       bright2 = 1.f; 
    }
    led2.Set(bright2);
    led2.Update();
    

    osc_a.Init(patch.AudioSampleRate());
    osc_b.Init(patch.AudioSampleRate());
    osc_c.Init(patch.AudioSampleRate());

    osc_a.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc_b.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc_c.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    //patch.StartLog(false);

    LoadCalibrationData();

    patch.StartAudio(AudioCallback);
    
    while(1) {
        if(trigger_save)
        {
            auto &data = cal_storage.GetSettings();
            GetCalData(data.scale, data.offset);
            cal_storage.Save();
            trigger_save = false;
        }
        //patch.PrintLine("Scale: %f", myscale );
    }
}


/** @brief Loads and sets calibration data */
void LoadCalibrationData()
{
    auto &data = cal_storage.GetSettings();
    SetCalData(data.scale, data.offset);
    //cal.SetData(data.scale, data.offset);
    //cal.SetData(60.f, 0.f);
    //myscale = data.scale;
    
}


