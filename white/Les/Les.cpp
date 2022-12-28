#include <array>

#include "../daisy_white.h"
#include "daisysp.h"
#include "oscillator_lfo.h"


using namespace daisy;
using namespace daisysp;


#define MAX_WAVE Oscillator_lfo::WAVE_POLYBLEP_TRI


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
    Oscillator_lfo osc;
    Wavefolder wf;
    Parameter  freqCtrl;
    Parameter  ampCtrl;
    Parameter  pwCtrl;
    Parameter  wfGainCtrl;
    Parameter  wfOffsetCtrl;
    float      amp;
    float      freq;
    int        waveform;
    float      value;
    float      ledvalue;
    bool       eoc;
    int        eocCounter;
    int        octave;
    float      oscOut;
    float      oscWfOut; 

    void Init(float samplerate, AnalogControl freqKnob, AnalogControl ampKnob, AnalogControl pwKnob, AnalogControl wfGainKnob, AnalogControl wfOffsetKnob)
    {
        osc.Init(samplerate);
        osc.SetAmp(1);
        wf.Init();
        waveform = 0;
        eoc = false;
        eocCounter = 0;
        octave = 0;
        freqCtrl.Init(freqKnob, .1, 20, Parameter::LOGARITHMIC);
        ampCtrl.Init(ampKnob, 0, 1, Parameter::LINEAR);
        pwCtrl.Init(pwKnob, 0.1, 0.87, Parameter::LINEAR);
        wfGainCtrl.Init(wfGainKnob, 0, 1, Parameter::LINEAR);
        wfOffsetCtrl.Init(wfOffsetKnob, 0, 1, Parameter::LINEAR);
    }

    void Process(DacHandle::Channel chn)
    {
        //read the knobs and set params
        osc.SetFreq(freqCtrl.Process()*(GetOctMult(octave)));
        osc.SetWaveform(waveform);
        if(waveform == osc.WAVE_SQUARE) {
            osc.SetPw(pwCtrl.Process());
        }
        if(osc.IsRising()) {
            eoc = true;
        } 
        if(osc.IsFalling()) {
            eoc = false;
        }
        //wf.SetGain(1.0f);
        //wf.SetOffset(wfOffsetCtrl.Process());

        oscOut = osc.Process();
        //oscWfOut = wf.Process(oscOut);

        //write to the DAC
        hw.seed.dac.WriteValue(
            chn,
            uint16_t((oscOut + 1.f) * .5f * ampCtrl.Process() * 4095.f));

        ledvalue = (osc.Process() + 1.f) * .5f * ampCtrl.Process();
        
        
    }

    float GetOctMult(int oct)
    {
        switch (oct)
        {
        case 0:
            return 1;
            break;
        case 1:
            return 2;
            break;
        case 2:
            return 4;
            break;
        default:
            return 1;
            break;
        }
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

struct sampHoldStruct
{
    SampleHold       sampHold;
    SampleHold::Mode mode;
    WhiteNoise noise;
    float            output;

    void Init()
    {
        noise.Init();
    }


    void Process(bool trigger, DacHandle::Channel chn)
    {
        output = sampHold.Process(trigger, noise.Process() * 500 + 500, mode);
        hw.seed.dac.WriteValue(
            chn,
            uint16_t(output * 4095.f));
    }
};



lfoStruct lfos[2];
envStruct envs[2];
sampHoldStruct sampHolds[2];



void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	Update_Controls();
    Update_Digital();

    for(size_t i = 0; i < size; i++)
        {
        switch (appMode[0])
        {
        case lfo:
            if (hw.TrigIn1()) {lfos[0].osc.Reset();}
            lfos[0].Process(DacHandle::Channel::ONE);
            break;
        case env:
            envs[0].Process(DacHandle::Channel::ONE,hw.gate_in1);
            break;
        case sh:
            sampHolds[0].Process(hw.gate_in1.State(),DacHandle::Channel::ONE);
            break;
        }	

        switch (appMode[1])
        {
        case lfo:
            if (hw.TrigIn2()) {lfos[1].osc.Reset();}
            lfos[1].Process(DacHandle::Channel::TWO);
            break;
        case env:
            envs[1].Process(DacHandle::Channel::TWO,hw.gate_in2);
            break;
        case sh:
            sampHolds[1].Process(hw.gate_in2.State(),DacHandle::Channel::TWO);
            break;
        }
    }
}



int main(void)
{
    hw.Init(); 

    float samplerate = hw.AudioSampleRate();

    lfos[0].Init(samplerate, hw.knob[0], hw.knob[1], hw.knob[9], hw.knob[4], hw.knob[5]);
    lfos[1].Init(samplerate, hw.knob[3], hw.knob[2], hw.knob[10], hw.knob[7], hw.knob[6]);

    envs[0].Init(samplerate, hw.knob[0], hw.knob[1], hw.knob[4], hw.knob[5]);
    envs[1].Init(samplerate, hw.knob[3], hw.knob[2], hw.knob[7], hw.knob[6]);

    sampHolds[0].Init();
    sampHolds[1].Init();

        	
    hw.StartAdc();
	hw.StartAudio(audio_callback);

	while(1)
	{	
		  
	}
}


void Update_Digital() {

    hw.ClearLeds();

    if(appMode[0] == lfo) {
        if(hw.SwitchRisingEdge(S4)) {
            lfos[0].waveform++;
            if(lfos[0].waveform == MAX_WAVE){
                lfos[0].waveform = 0;
            }
        }
        if(hw.SwitchRisingEdge(S2)) {
            lfos[0].octave++;
            if(lfos[0].octave == 3){
                lfos[0].octave = 0;
            }
        }
        switch (lfos[0].octave)
        {
        case 1:
            hw.SetGreenLeds(hw.GREEN_LED_3,1);
            break;
        case 2:
            hw.SetGreenLeds(hw.GREEN_LED_4,1);
            break;
        default:
            break;
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
        if(hw.SwitchRisingEdge(S6)) {
            lfos[1].octave++;
            if(lfos[1].octave == 3){
                lfos[1].octave = 0;
            }
        }
        switch (lfos[1].octave)
        {
        case 1:
            hw.SetGreenDirectLeds(hw.GREEN_D_LED_2,1);
            break;
        case 2:
            hw.SetGreenDirectLeds(hw.GREEN_D_LED_4,1);
            break;
        default:
            break;
        }
        hw.SetRGBColor(hw.RGB_LED_4,static_cast<DaisyWhite::Colors>(lfos[1].waveform));
        hw.SetGreenLeds(hw.GREEN_LED_2,lfos[1].ledvalue);
        if(lfos[1].eoc){
            hw.SetRGBColor(hw.RGB_LED_3,hw.yellow);
        }
        dsy_gpio_write(&hw.gate_out_2, lfos[1].eoc);
    }

    if(appMode[0] == env) {
         hw.SetRGBColor(hw.RGB_LED_1,hw.purple);
    }

    if(appMode[1] == env) {
         hw.SetRGBColor(hw.RGB_LED_4,hw.purple);
    }

    if(appMode[0] == sh) {
         hw.SetRGBColor(hw.RGB_LED_1,hw.orange);
    }

    if(appMode[1] == sh) {
         hw.SetRGBColor(hw.RGB_LED_4,hw.orange);
    }
    
    
    hw.UpdateLeds();
}

void Update_Controls() {
    hw.ProcessAllControls();
    AppMode(hw.AudioSampleRate());
}

void AppMode(float samplerate) {
    if(!hw.SwitchState(S0A)) {
        appMode[0] = lfo;
        
    } else if(!hw.SwitchState(S0B)) {
        appMode[0] = sh;
    } else {
        appMode[0] = env;
        
    }

    if(!hw.SwitchState(S1A)) {
        appMode[1] = lfo;
        
    } else if(!hw.SwitchState(S1B)) {
        appMode[1] = sh;
    } else {
        appMode[1] = env;
        
    }
}