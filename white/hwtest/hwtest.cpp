#include <array>

#include "../daisy_white.h"
#include "daisysp.h"
#include "settings.h"
#include "ui.h"


using namespace daisy;
using namespace daisysp;
using namespace white;

DaisyWhite hw;
Settings settings;
Ui ui;

static Oscillator osc;
Parameter  freqctrl, wavectrl, ampctrl, finectrl;

void Update_Digital();
void Start_Led_Ani();
void LoadSettings();
void LoadState();
void SaveSettings();
void SaveState();


bool readyToSave = false;
bool readyToRestore = false;


struct lfoStruct
{
    Oscillator osc_lfo;
    Parameter  rateCtrl;
    Parameter  levelCtrl;
    float      amp;
    float      freq;
    int        waveform;
    float      value;

    void Init(float samplerate, AnalogControl rateKnob, AnalogControl levelKnob)
    {
        osc_lfo.Init(samplerate);
        osc_lfo.SetAmp(1);
        waveform = osc_lfo.WAVE_SIN;
        rateCtrl.Init(rateKnob, .1, 35, Parameter::LOGARITHMIC);
        levelCtrl.Init(levelKnob, 0, 1, Parameter::LINEAR);
    }

    void Process(DacHandle::Channel chn)
    {
        //read the knobs and set params
        osc_lfo.SetFreq(rateCtrl.Process());
        osc_lfo.SetWaveform(waveform);

        //write to the DAC
        hw.seed.dac.WriteValue(
            chn,
            uint16_t((osc_lfo.Process() + 1.f) * .5f * levelCtrl.Process() * 4095.f));
    }
};

lfoStruct lfo;




uint8_t decay;

void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	ui.Poll();
    
    float sig, freq, amp;
    size_t wave;
    
    for(size_t i = 0; i < size; i++)
    {
        freq = mtof(freqctrl.Process() + finectrl.Process());
        wave = wavectrl.Process();
        amp  = ampctrl.Process();

        osc.SetFreq(freq);
        osc.SetWaveform(wave);
        osc.SetAmp(amp);
        
        sig = osc.Process();

        // left out
        out[0][i] = sig;
        out[1][i] = sig;

        lfo.Process(DacHandle::Channel::ONE);

        

    }

    
}



int main(void)
{
    hw.Init();
    settings.Init(&hw);
  
    ui.Init(&settings, &hw);

    LoadSettings();
    LoadState();

    float samplerate = hw.AudioSampleRate();
    int   num_waves = Oscillator::WAVE_LAST - 1;
            	
    
    
    Start_Led_Ani();

    osc.Init(samplerate);


    // Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SAW);
    osc.SetFreq(30);
    osc.SetAmp(0.75);

    lfo.Init(samplerate, hw.knob[4], hw.knob[5]);

    freqctrl.Init(hw.knob[KNOB_1], 10.0, 110.0f, Parameter::LINEAR);
    finectrl.Init(hw.knob[KNOB_2], 0.f, 7.f, Parameter::LINEAR);
    wavectrl.Init(hw.knob[KNOB_3], 0.0, num_waves, Parameter::LINEAR);
    ampctrl.Init(hw.knob[KNOB_4], 0.0, 0.5f, Parameter::LINEAR);

    
    hw.StartAdc();
	hw.StartAudio(audio_callback);

	while(1)
	{	
		 //hw.ProcessAllControls();
        //Update_Digital(); 
         if (readyToSave) {
            /** Collect the data from where ever in the application it is */
            SaveSettings();
            SaveState();

            /** And trigger the save */
            
            readyToSave = false;
        }
        if (readyToRestore) {
            /** Collect the data from where ever in the application it is */
            settings.RestoreSettings();

            /** And trigger the save */
            
            readyToRestore = false;
        }
        
	}
}


void Update_Digital() {

    hw.ClearLeds();    
    hw.UpdateLeds();

    
}

void Update_Controls() {
    hw.ProcessAllControls();
    
}

void Start_Led_Ani() {
    size_t deltime = 100;
    //hw.ClearLeds();
    
    for(size_t i = 0; i < RGB_LED_4+1; i++)
    {
        hw.SetRGBColor(static_cast<LeddriverLeds>(i),RED);        
        hw.UpdateLeds();
        hw.DelayMs(deltime);
    }
    for(size_t i = 0; i < RGB_LED_4+1; i++)
    {
        hw.SetRGBColor(static_cast<LeddriverLeds>(i),GREEN);        
        hw.UpdateLeds();
        hw.DelayMs(deltime);
    }
    for(size_t i = 0; i < RGB_LED_4+1; i++)
    {
        hw.SetRGBColor(static_cast<LeddriverLeds>(i),BLUE);        
        hw.UpdateLeds();
        hw.DelayMs(deltime);
    }

    hw.SetGreenLeds(GREEN_LED_1,1.f);
    hw.UpdateLeds();
    hw.DelayMs(deltime);
    hw.SetGreenLeds(GREEN_LED_2,1.f);
    hw.UpdateLeds();
    hw.DelayMs(deltime);
    hw.SetGreenLeds(GREEN_LED_3,1.f);
    hw.UpdateLeds();
    hw.DelayMs(deltime);
    hw.SetGreenLeds(GREEN_LED_4,1.f);
    hw.UpdateLeds();
    hw.DelayMs(deltime);

    hw.SetGreenDirectLeds(GREEN_D_LED_1,1.f);
    hw.UpdateLeds();
    hw.DelayMs(deltime);
    hw.SetGreenDirectLeds(GREEN_D_LED_2,1.f);
    hw.UpdateLeds();
    hw.DelayMs(deltime);
    hw.SetGreenDirectLeds(GREEN_D_LED_3,1.f);
    hw.UpdateLeds();
    hw.DelayMs(deltime);
    hw.SetGreenDirectLeds(GREEN_D_LED_4,1.f);
    hw.UpdateLeds();
    hw.DelayMs(deltime);
    hw.SetGreenDirectLeds(GREEN_D_LED_5,1.f);
    hw.UpdateLeds();
    hw.DelayMs(deltime);
    hw.SetGreenDirectLeds(GREEN_D_LED_6,1.f);
    hw.UpdateLeds();
    hw.DelayMs(1000);
    

    //hw.UpdateLeds();
}

void LoadSettings(){
    const HwtestSettings& hwtestsettings = settings.hwtestsettings();
    
    
    for(int i = 0; i < 14; i++)
    {
        //ledstate[i] = hwtestsettings.ledstate[i];
    }
}

void LoadState(){
    const StateSettings& statesettings = settings.statesettings();
    decay = statesettings.decay;
}

void SaveSettings(){
    HwtestSettings* hwtestsettings = settings.margolis_hwtestsettings();
   
    
    for(int i = 0; i < 14; i++)
    {
        //hwtestsettings->ledstate[i] = ledstate[i];
    }
    
    settings.SaveSettings();
}

void SaveState(){
    StateSettings* statesettings = settings.margolis_statesettings();
    statesettings->decay = decay;
    
    settings.SaveStateSettings();
}
