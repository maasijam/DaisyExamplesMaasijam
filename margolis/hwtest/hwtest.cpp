#include <array>

#include "../daisy_margolis.h"
#include "daisysp.h"
#include "settings.h"


using namespace daisy;
using namespace daisysp;
using namespace margolis;

DaisyMargolis hw;
Settings settings;

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


int ledcount, engine;
bool ledatt[3];
uint8_t decay;

void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
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
    //hw.seed.qspi.EraseSector(4096*0);
    //hw.seed.qspi.EraseSector(4096*1);
    //hw.seed.qspi.EraseSector(4096*2);
    //hw.seed.qspi.EraseSector(4096*3);
    settings.Init(&hw);

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
		 hw.ProcessAllControls();
        Update_Digital(); 
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

    if(!hw.Gate()){
        hw.SetRGBColor(LED_RGB_8,BLUE);
    }

    if(hw.s[S1].RisingEdge()) {
        engine = 0;
        ledcount++;
    }

    if(hw.s[S2].RisingEdge()) {
        engine = 1;
        ledcount++;
    }

    if(hw.s[S3].RisingEdge()) {
        engine = 2;
        ledcount++;
    }
    
    if(ledcount == 8) {
        ledcount = 0;
    }

    switch (engine)
    {
    case 0:
        hw.SetRGBColor(static_cast<LeddriverLeds>(ledcount),RED);
        break;
    case 1:
        hw.SetRGBColor(static_cast<LeddriverLeds>(ledcount),YELLOW);
        break;
    case 2:
        hw.SetRGBColor(static_cast<LeddriverLeds>(ledcount),GREEN);
        break;
    }

    if(hw.s[S4].RisingEdge()) {
        //settings.ledatt[0] = !settings.ledatt[0];
        decay++;
        
    }
    if(decay == 2) {
        decay = 0;
    }
    if(decay == 1) {
            hw.SetRGBColor(LED_RGB_2,DARKORANGE);
    }

    if(hw.s[S5].RisingEdge()) {
        ledatt[1] = !ledatt[1];
        
    }
    if(ledatt[1]) {
            hw.SetGreenLeds(LED_GREEN_2,hw.knob[KNOB_6].Value());
        }

    if(hw.s[S6].RisingEdge()) {
        ledatt[2] = !ledatt[2];
        
    }
    if(ledatt[2]) {
            hw.SetGreenLeds(LED_GREEN_3,hw.knob[KNOB_7].Value());
        }
    
    if(hw.s[S6].TimeHeldMs() > 1000) {
        hw.SetRGBColor(LED_RGB_1,PURPLE);
        hw.SetRGBColor(LED_RGB_2,PURPLE);
        hw.SetRGBColor(LED_RGB_3,PURPLE);
        hw.SetRGBColor(LED_RGB_4,PURPLE);
        hw.SetRGBColor(LED_RGB_5,PURPLE);
        hw.SetRGBColor(LED_RGB_6,PURPLE);
        hw.SetRGBColor(LED_RGB_7,PURPLE);
        hw.SetRGBColor(LED_RGB_8,PURPLE);
        readyToSave = true;
    }
    if(hw.s[S4].TimeHeldMs() > 1000) {
        hw.SetRGBColor(LED_RGB_1,RED);
        hw.SetRGBColor(LED_RGB_2,RED);
        hw.SetRGBColor(LED_RGB_3,RED);
        hw.SetRGBColor(LED_RGB_4,RED);
        hw.SetRGBColor(LED_RGB_5,RED);
        hw.SetRGBColor(LED_RGB_6,RED);
        hw.SetRGBColor(LED_RGB_7,RED);
        hw.SetRGBColor(LED_RGB_8,RED);
        readyToRestore = true;
    }

    
    
    
    hw.UpdateLeds();

    
}

void Update_Controls() {
    hw.ProcessAllControls();
    
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

void LoadSettings(){
    const HwtestSettings& hwtestsettings = settings.hwtestsettings();
    ledcount = hwtestsettings.ledcount;
    engine = hwtestsettings.engine;
    
    for(int i = 0; i < 3; i++)
    {
        ledatt[i] = hwtestsettings.ledatt[i];
    }
}

void LoadState(){
    const StateSettings& statesettings = settings.statesettings();
    decay = statesettings.decay;
}

void SaveSettings(){
    HwtestSettings* hwtestsettings = settings.margolis_hwtestsettings();
    hwtestsettings->ledcount = ledcount;
    hwtestsettings->engine = engine;
    
    for(int i = 0; i < 3; i++)
    {
        hwtestsettings->ledatt[i] = ledatt[i];
    }
    
    settings.SaveSettings();
}

void SaveState(){
    StateSettings* statesettings = settings.margolis_statesettings();
    statesettings->decay = decay;
    
    settings.SaveStateSettings();
}
