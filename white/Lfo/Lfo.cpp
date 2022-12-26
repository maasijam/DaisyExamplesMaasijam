#include <array>

#include "../daisy_white.h"
#include "daisysp.h"



using namespace daisy;
using namespace daisysp;


#define MAX_WAVE Oscillator::WAVE_POLYBLEP_TRI


DaisyWhite hw;

enum
{
    S1      = 15,
    S2      = 14,
    S3      = 13,
    S4      = 12,
    S5      = 11,
    S6      = 10,
    S7      = 9,
    S8      = 8,
    S0A     = 7,
    S0B     = 6,
    S1A     = 5,
    S1B     = 4,
};

enum Mode {
    lfo,
    env,
    sh,
    modeLast
};

void Update_Controls();
void Update_Digital();
void AppMode(float samplerate);

int appMode[2];


struct lfoStruct
{
    Oscillator osc;
    Parameter  freqCtrl;
    Parameter  ampCtrl;
    float      amp;
    float      freq;
    int        waveform;
    float      value;
    float      ledvalue;
    bool       eoc;
    int        eocCounter;

    void Init(float samplerate, AnalogControl freqKnob, AnalogControl ampKnob)
    {
        osc.Init(samplerate);
        osc.SetAmp(1);
        waveform = 0;
        eoc = false;
        eocCounter = 0;
        freqCtrl.Init(freqKnob, .1, 35, Parameter::LOGARITHMIC);
        ampCtrl.Init(ampKnob, 0, 1, Parameter::LINEAR);
    }

    void Process(DacHandle::Channel chn)
    {
        //read the knobs and set params
        osc.SetFreq(freqCtrl.Process());
        osc.SetWaveform(waveform);
        if(osc.IsRising()) {
            eoc = true;
        } 
        if(osc.IsFalling()) {
            eoc = false;
        }

        

        //write to the DAC
        hw.seed.dac.WriteValue(
            chn,
            uint16_t((osc.Process() + 1.f) * .5f * ampCtrl.Process() * 4095.f));

        ledvalue = (osc.Process() + 1.f) * .5f * ampCtrl.Process();
        
        
    }
};

struct envStruct {
    Adsr       env;
    Parameter  attCtrl;
    Parameter  decayCtrl;
    Parameter  susCtrl;
    Parameter  relCtrl;

    bool env_state;
    

    void Init(float samplerate, AnalogControl attKnob, AnalogControl decayKnob, AnalogControl susKnob, AnalogControl relKnob)
    {
        env.Init(samplerate);
        
        attCtrl.Init(attKnob, 0, 1, Parameter::LINEAR);
        decayCtrl.Init(decayKnob, .08, 1, Parameter::LINEAR);
        susCtrl.Init(susKnob, 0, 1, Parameter::LINEAR);
        relCtrl.Init(relKnob, 0, 1, Parameter::LINEAR);
    }

    void Process(DacHandle::Channel chn, GateIn envgate)
    {
        if(envgate.State())
            env_state = true;
        else
            env_state = false;
        
        env.SetAttackTime(attCtrl.Process());
        env.SetDecayTime(decayCtrl.Process());
        env.SetSustainLevel(susCtrl.Process());
        env.SetReleaseTime(relCtrl.Process());

        

        //write to the DAC
        hw.seed.dac.WriteValue(
            chn,
            uint16_t(env.Process(env_state) * 4095.f));

        
        
        
    }
};


bool        menuSelect;
int         lfoSelect;


lfoStruct lfos[2];
envStruct envs[2];



void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	Update_Controls();
    Update_Digital();

    switch (appMode[0])
    {
    case lfo:
        if (hw.TrigIn1()) {lfos[0].osc.Reset();}
        for(size_t i = 0; i < size; i++)
        {
            lfos[0].Process(DacHandle::Channel::ONE);
        }
        break;
    case env:
        for(size_t i = 0; i < size; i++)
        {
            envs[0].Process(DacHandle::Channel::ONE,hw.gate_in1);
        }
        break;
    case sh:
        /* code */
        break;
    }	

    switch (appMode[1])
    {
    case lfo:
        if (hw.TrigIn2()) {lfos[1].osc.Reset();}
        for(size_t i = 0; i < size; i++)
        {
            lfos[1].Process(DacHandle::Channel::TWO);
        }
        break;
    case env:
        for(size_t i = 0; i < size; i++)
        {
            envs[1].Process(DacHandle::Channel::TWO,hw.gate_in2);
        }
        break;
    case sh:
        /* code */
        break;
    }
    
}



int main(void)
{
    hw.Init();
    
    

    float samplerate = hw.AudioSampleRate();

    AppMode(samplerate);


    //init the lfos
    if(appMode[0] == lfo) {
        lfos[0].Init(samplerate, hw.knob[0], hw.knob[1]);
    }
    if(appMode[1] == lfo) {
        lfos[1].Init(samplerate, hw.knob[3], hw.knob[2]);
    }

    //init the lfos
    if(appMode[0] == env) {
        envs[0].Init(samplerate, hw.knob[0], hw.knob[1], hw.knob[4], hw.knob[5]);
    }
    if(appMode[1] == env) {
        envs[1].Init(samplerate, hw.knob[3], hw.knob[2], hw.knob[6], hw.knob[7]);
    }



    	
    hw.StartAdc();
	hw.StartAudio(audio_callback);

    

	while(1)
	{	
		hw.DelayMs(1);   
	}
}


void Update_Digital() {

    AppMode(hw.AudioSampleRate());

    hw.ClearLeds();

    if(appMode[0] == lfo) {
        if(hw.SwitchRisingEdge(S4)) {
            lfos[0].waveform++;
            if(lfos[0].waveform == MAX_WAVE){
                lfos[0].waveform = 0;
            }
        }
        hw.SetRGBColor(hw.RGB_LED_1,static_cast<DaisyWhite::Colors>(lfos[0].waveform));
        hw.SetGreenLeds(hw.GREEN_LED_1,lfos[0].ledvalue);
        if(lfos[0].eoc){
            hw.SetRGBColor(hw.RGB_LED_2,hw.yellow);
        }
        dsy_gpio_write(&hw.gate_out_1, lfos[0].eoc);
    }

    if(appMode[1] == lfo) {
        if(hw.SwitchRisingEdge(S5)) {
            lfos[1].waveform++;
            if(lfos[1].waveform == MAX_WAVE){
                lfos[1].waveform = 0;
            }
        }
        hw.SetRGBColor(hw.RGB_LED_4,static_cast<DaisyWhite::Colors>(lfos[1].waveform));
        hw.SetGreenLeds(hw.GREEN_LED_2,lfos[1].ledvalue);
        if(lfos[1].eoc){
            hw.SetRGBColor(hw.RGB_LED_3,hw.yellow);
        }
        dsy_gpio_write(&hw.gate_out_2, lfos[1].eoc);
    }
    
    
    hw.UpdateLeds();
}

void Update_Controls() {
    hw.ProcessAllControls();
}

void AppMode(float samplerate) {
    if(!hw.SwitchState(S0A)) {
        appMode[0] = lfo;
        lfos[0].Init(samplerate, hw.knob[0], hw.knob[1]);
    } else if(!hw.SwitchState(S0B)) {
        appMode[0] = sh;
    } else {
        appMode[0] = env;
        envs[0].Init(samplerate, hw.knob[0], hw.knob[1], hw.knob[4], hw.knob[5]);
    }

    if(!hw.SwitchState(S1A)) {
        appMode[1] = lfo;
        lfos[1].Init(samplerate, hw.knob[3], hw.knob[2]);
    } else if(!hw.SwitchState(S1B)) {
        appMode[1] = sh;
    } else {
        appMode[1] = env;
        envs[1].Init(samplerate, hw.knob[3], hw.knob[2], hw.knob[6], hw.knob[7]);
    }
}