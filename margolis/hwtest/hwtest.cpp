#include <array>

#include "../daisy_margolis.h"
#include "daisysp.h"
#include "settings.h"


using namespace daisy;
using namespace daisysp;






DaisyMargolis hw;
Settings settings;





static Oscillator osc;
Parameter  freqctrl, wavectrl, ampctrl, finectrl;




void Update_Digital();
void Start_Led_Ani();


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
        waveform = osc_lfo.WAVE_SQUARE;
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
    settings.Init(&hw);

    float samplerate = hw.AudioSampleRate();
    int   num_waves = Oscillator::WAVE_LAST - 1;
            	
    
    
    Start_Led_Ani();

    osc.Init(samplerate);


    // Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SAW);
    osc.SetFreq(600);
    osc.SetAmp(0.75);

    lfo.Init(samplerate, hw.knob[4], hw.knob[5]);

    freqctrl.Init(hw.cv[hw.CV_VOCT], 10.0, 110.0f, Parameter::LINEAR);
    finectrl.Init(hw.knob[hw.KNOB_2], 0.f, 7.f, Parameter::LINEAR);
    wavectrl.Init(hw.knob[hw.KNOB_3], 0.0, num_waves, Parameter::LINEAR);
    ampctrl.Init(hw.knob[hw.KNOB_4], 0.0, 0.5f, Parameter::LINEAR);

    
    hw.StartAdc();
	hw.StartAudio(audio_callback);

	while(1)
	{	
		 hw.ProcessAllControls();
         Update_Digital(); 
         if (readyToSave) {
            /** Collect the data from where ever in the application it is */
            settings.SaveSettings();
            settings.SaveStateSettings();

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
        hw.SetRGBColor(hw.LED_RGB_8,hw.blue);
    }

    if(hw.s[hw.S1].RisingEdge()) {
        settings.engine = 0;
        settings.ledcount++;
    }

    if(hw.s[hw.S2].RisingEdge()) {
        settings.engine = 1;
        settings.ledcount++;
    }

    if(hw.s[hw.S3].RisingEdge()) {
        settings.engine = 2;
        settings.ledcount++;
    }
    
    if(settings.ledcount == 8) {
        settings.ledcount = 0;
    }

    switch (settings.engine)
    {
    case 0:
        hw.SetRGBColor(static_cast<DaisyMargolis::LeddriverLeds>(settings.ledcount),hw.red);
        break;
    case 1:
        hw.SetRGBColor(static_cast<DaisyMargolis::LeddriverLeds>(settings.ledcount),hw.yellow);
        break;
    case 2:
        hw.SetRGBColor(static_cast<DaisyMargolis::LeddriverLeds>(settings.ledcount),hw.green);
        break;
    }

    if(hw.s[hw.S4].RisingEdge()) {
        //settings.ledatt[0] = !settings.ledatt[0];
        settings.decay++;
        
    }
    if(settings.decay == 2) {
        settings.decay = 0;
    }
    if(settings.decay == 1) {
            hw.SetRGBColor(hw.LED_RGB_2,hw.darkorange);
    }

    if(hw.s[hw.S5].RisingEdge()) {
        settings.ledatt[1] = !settings.ledatt[1];
        
    }
    if(settings.ledatt[1]) {
            hw.SetGreenLeds(hw.LED_GREEN_2,hw.knob[hw.KNOB_6].Value());
        }

    if(hw.s[hw.S6].RisingEdge()) {
        settings.ledatt[2] = !settings.ledatt[2];
        
    }
    if(settings.ledatt[2]) {
            hw.SetGreenLeds(hw.LED_GREEN_3,hw.knob[hw.KNOB_7].Value());
        }
    
    if(hw.s[hw.S6].TimeHeldMs() > 1000) {
        hw.SetRGBColor(hw.LED_RGB_1,hw.purple);
        hw.SetRGBColor(hw.LED_RGB_2,hw.purple);
        hw.SetRGBColor(hw.LED_RGB_3,hw.purple);
        hw.SetRGBColor(hw.LED_RGB_4,hw.purple);
        hw.SetRGBColor(hw.LED_RGB_5,hw.purple);
        hw.SetRGBColor(hw.LED_RGB_6,hw.purple);
        hw.SetRGBColor(hw.LED_RGB_7,hw.purple);
        hw.SetRGBColor(hw.LED_RGB_8,hw.purple);
        readyToSave = true;
    }
    if(hw.s[hw.S4].TimeHeldMs() > 1000) {
        hw.SetRGBColor(hw.LED_RGB_1,hw.red);
        hw.SetRGBColor(hw.LED_RGB_2,hw.red);
        hw.SetRGBColor(hw.LED_RGB_3,hw.red);
        hw.SetRGBColor(hw.LED_RGB_4,hw.red);
        hw.SetRGBColor(hw.LED_RGB_5,hw.red);
        hw.SetRGBColor(hw.LED_RGB_6,hw.red);
        hw.SetRGBColor(hw.LED_RGB_7,hw.red);
        hw.SetRGBColor(hw.LED_RGB_8,hw.red);
        readyToRestore = true;
    }

    
    
    
    hw.UpdateLeds();

    
}

void Update_Controls() {
    hw.ProcessAllControls();
    
}

void Start_Led_Ani() {
    
    
    for(size_t i = 0; i < hw.LEDDRIVER_LAST; i++)
    {
        if(i < 8) {
            hw.SetRGBColor(static_cast<DaisyMargolis::LeddriverLeds>(i),hw.red);
        }
        
        hw.UpdateLeds();
        hw.DelayMs(50);
    }

    for(size_t i = 0; i < hw.LEDDRIVER_LAST; i++)
    {
        if(i < 8) {
            hw.SetRGBColor(static_cast<DaisyMargolis::LeddriverLeds>(i),hw.green);
        }
        
        hw.UpdateLeds();
        hw.DelayMs(50);
    }

    for(size_t i = 0; i < hw.LEDDRIVER_LAST; i++)
    {
        if(i < 8) {
            hw.SetRGBColor(static_cast<DaisyMargolis::LeddriverLeds>(i),hw.blue);
        }
        
        hw.UpdateLeds();
        hw.DelayMs(50);
    }

    for(size_t i = 0; i < 3; i++)
    {
        hw.SetGreenLeds(static_cast<DaisyMargolis::LeddriverLeds>(hw.LEDDRIVER_LAST - (i + 1)),1.0f);
                
        hw.UpdateLeds();
        hw.DelayMs(100);
    }
    
}