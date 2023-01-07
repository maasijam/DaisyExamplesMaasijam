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
//Settings settings;

Oscillator   osc_a, osc_b, osc_c;


void Start_Led_Ani();
void LoadSettings();
void LoadState();
void SaveSettings();
void SaveState();


bool readyToSave = false;
bool readyToRestore = false;



void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	hw.ProcessAnalogControls();

    ui.Poll();

    float  cvval;
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

    /** Process each sample of the oscillator and send to the hardware outputs */
    for(size_t i = 0; i < size; i++)
    {
        float sig = osc_a.Process() + osc_b.Process() + osc_c.Process();
        OUT_L[i]  = sig;
        OUT_R[i]  = sig;
        mycvin = CV_6;
        if(hw.cv[mycvin].Value() >= 0.f) {
            cvval = hw.cv[mycvin].Value();
        } else {
            cvval = hw.cv[mycvin].Value() * -1;
        }
        hw.seed.dac.WriteValue(
            DacHandle::Channel::ONE,
            uint16_t(cvval * 1023.f));
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

    Start_Led_Ani();
    
    hw.StartAdc();
	hw.StartAudio(audio_callback);

	while(1)
	{	
         if (readyToSave) {
            /** Collect the data from where ever in the application it is */
            //SaveSettings();
            //SaveState();

            /** And trigger the save */
            
            readyToSave = false;
        }
        if (readyToRestore) {
            /** Collect the data from where ever in the application it is */
            //settings.RestoreSettings();

            /** And trigger the save */
            
            readyToRestore = false;
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
