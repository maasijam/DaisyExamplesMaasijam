#include <array>

#include "../daisy_margolis.h"
#include "daisysp.h"
#include "ui.h"



using namespace daisy;
using namespace daisysp;
using namespace margolis;
using namespace supersaw;

DaisyMargolis hw;
Ui ui;
static Adsr    env[3];


Oscillator   osc_a, osc_b, osc_c;


void Start_Led_Ani();
void LoadSettings();
void LoadState();
void SaveSettings();
void SaveState();



bool readyToRestore = false;
bool              gate;



void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	hw.ProcessAnalogControls();

    ui.Poll();

    
    float env_out[3];
    bool env_state;
   
    CvIns mycvin;
    

    /** Get Coarse, Fine, and V/OCT inputs from hardware 
     *  MIDI Note number are easy to use for defining ranges */
    float knob_coarse = hw.GetKnobValue(CV_1);
    float coarse_tune = fmap(knob_coarse, 12, 84);

    float knob_fine = hw.GetKnobValue(CV_2);
    float fine_tune = fmap(knob_fine, 0, 10);

    //float cv_voct = hw.GetCvValue(CV_VOCT);
    //float cv_voct = hw.GetWarpVoct();
    //float voct    = fmap(cv_voct, 0, 60);
    float voct    = hw.GetWarpVoct();

    /** Convert from MIDI note number to frequency */
    float midi_nn = fclamp(coarse_tune + fine_tune + voct, 0.f, 127.f);
    float freq_a  = mtof(midi_nn);

    /** Calculate a detune amount */
    float detune_amt = hw.GetKnobValue(KNOB_3);
    float freq_b     = freq_a + (0.05 * freq_a * detune_amt);
    float freq_c     = freq_a - (0.05 * freq_a * detune_amt);

    /** Set all three oscillators' frequencies */
    osc_a.SetFreq(freq_a);
    osc_b.SetFreq(freq_b);
    osc_c.SetFreq(freq_c);

    if(hw.gate_in.State())
            env_state = true;
        else
            env_state = false;
    float knob_attack = hw.GetKnobValue(KNOB_5);
    float knob_decay = hw.GetKnobValue(KNOB_6);
    float knob_sustain = hw.GetKnobValue(KNOB_7);
      float knob_release = hw.GetKnobValue(KNOB_4);
    

    for (size_t i = 0; i < 3; i++)
    {
        env[i].SetAttackTime(knob_attack);
        env[i].SetDecayTime(knob_decay);
        env[i].SetSustainLevel(knob_sustain);
        env[i].SetReleaseTime(knob_release);
    }

    
    
    

    /** Process each sample of the oscillator and send to the hardware outputs */
    for(size_t i = 0; i < size; i++)
    {
        
        
        // Use envelope to control the amplitude of the oscillator.
        for (size_t i = 0; i < 3; i++)
        {
            env_out[i] = env[i].Process(env_state);
        }

        
        
        osc_a.SetAmp(env_out[0]);
        osc_b.SetAmp(env_out[1]);
        osc_c.SetAmp(env_out[2]);
        
        
        float sig = osc_a.Process() + osc_b.Process() + osc_c.Process();
        
        OUT_L[i]  = sig;
        OUT_R[i]  = sig;

        
    }
    
}



int main(void)
{
    hw.Init();
    ui.Init(&hw);
    //hw.seed.qspi.EraseSector(4096*0);
    //hw.seed.qspi.EraseSector(4096*1);
    //hw.seed.qspi.EraseSector(4096*2);
    //hw.seed.qspi.EraseSector(4096*3);
    //settings.Init(&hw);
    
    
    //LoadState();

    float samplerate = hw.AudioSampleRate();
    osc_a.Init(samplerate);
    osc_b.Init(samplerate);
    osc_c.Init(samplerate);

    osc_a.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc_b.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc_c.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);

    

    //Set envelope 
    for (size_t i = 0; i < 3; i++)
    {
        env[i].Init(samplerate);
        env[i].SetTime(ADSR_SEG_ATTACK, .01);
        env[i].SetTime(ADSR_SEG_DECAY, .1);
        env[i].SetTime(ADSR_SEG_RELEASE, .06);

        env[i].SetSustainLevel(.05);
    }

    
    
    

    Start_Led_Ani();
    
    hw.StartAdc();
	hw.StartAudio(audio_callback);

	while(1)
	{	
         
         float  cvval = static_cast<int>(ui.GetTask());
        /*mycvin = CV_6;
        if(hw.cv[mycvin].Value() >= 0.f) {
            cvval = hw.cv[mycvin].Value();
        } else {
            cvval = hw.cv[mycvin].Value() * -1;
        }*/
        hw.seed.dac.WriteValue(
            DacHandle::Channel::ONE,
            uint16_t(cvval * 1024.f));
         
         if (hw.ReadyToSaveCal()) {
            /** Collect the data from where ever in the application it is */
            //SaveSettings();
            //SaveState();
            ui.SaveCalibrationData();
       
            /** And trigger the save */
            
            hw.ClearSaveCalFlag();
        }
        if (ui.readyToSaveState) {
            /** Collect the data from where ever in the application it is */
            //settings.RestoreSettings();

            /** And trigger the save */
            ui.SaveStateData();
            ui.readyToSaveState = false;
        }
        
	}
}




void Start_Led_Ani() {
    hw.ClearLeds();
    
    for(size_t i = 0; i < LEDDRIVER_LAST; i++)
    {
        if(i < 8) {
            hw.SetRGBColor(static_cast<LeddriverLeds>(i),RED);
        }
        
        hw.UpdateLeds();
        hw.DelayMs(50);
    }

    for(size_t i = 0; i < LEDDRIVER_LAST; i++)
    {
        if(i < 8) {
            hw.SetRGBColor(static_cast<LeddriverLeds>(i),GREEN);
        }
        
        hw.UpdateLeds();
        hw.DelayMs(50);
    }

    for(size_t i = 0; i < LEDDRIVER_LAST; i++)
    {
        if(i < 8) {
            hw.SetRGBColor(static_cast<LeddriverLeds>(i),BLUE);
        }
        
        hw.UpdateLeds();
        hw.DelayMs(50);
    }

    for(size_t i = 0; i < 3; i++)
    {
        hw.SetGreenLeds(static_cast<LeddriverLeds>(LEDDRIVER_LAST - (i + 1)),1.0f);
                
        hw.UpdateLeds();
        hw.DelayMs(100);
    }
    
}
