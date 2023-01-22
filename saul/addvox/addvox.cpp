#include "../daisy_saul.h"
#include "daisysp.h"

#define MIN_PARTIALS 1
#define STARTUP_PARTIALS 30;
#define STARTUP_AMPS 5
#define STARTUP_WAVE 0
#define STARTUP_POLY 1
#define STARTUP_MILD 7
#define TRANSPOSE_OCTAVES -1

#define MIN_SLOPE 1 
#define MAX_VOICES 1
#define MAXP 80


using namespace daisy;
using namespace daisysp;
using namespace saul;


DaisySaul hw;

Oscillator oscs[MAX_VOICES][MAXP];
float as[MAXP];
Parameter constantctrl, stretchctrl, ampctrl, wavectrl, partialsctrl, mildctrl, ampsctrl;
float dbg;
int partials = STARTUP_PARTIALS;
int amps = STARTUP_AMPS;
int wave = STARTUP_WAVE;
int poly = STARTUP_POLY;
int mild = STARTUP_MILD;
int num_waves = Oscillator::WAVE_LAST;
float normalizer = MAXP;
bool encoder_pressed_prev = false;
enum
{
    P_PARTIALS,
    P_AMPS,
    P_MILD,
    P_WAVE,
    P_POLY,
    P_LAST
};

enum
{
    A_EXP,
    A_LP,
    A_BP,
    A_HP,
    A_NOTCH,
    A_COMB,
    A_COMB_DIFFERENT,
    A_LAST
};

int max_partials[]     = { 40  , MAXP, MAXP, MAXP, MAXP, 40  , 40  , 40  };

float vtof(float voltage)
{
    float freq = powf(2.f, voltage) * 55;
    return freq;
}

float GetFundV(void)
{
    float in = hw.GetKnobValue(KNOB_8);
    float voltage = in * 5.0f + TRANSPOSE_OCTAVES ;
    return voltage;
}

float GetStretchV(void)
{
    float in = hw.GetKnobValue(KNOB_9);
    float voltage = in * 5.0f;
    return voltage;
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    float sum, con, stretch, amp, a, fundf, fundv;
    float samples[MAXP];

    hw.ProcessAnalogControls();

    stretch = GetStretchV();//stretchctrl.Process();
    con = constantctrl.Process();
    amp = ampctrl.Process();
    wave = wavectrl.Process();

    bool sync = hw.gate.Trig();

    for (int voice=0; voice < poly; voice++) {
        if(voice==0) {
            fundv = GetFundV();
	    fundf = vtof(fundv); 
        } else {
            continue;
        }
        for (int par=0; par < partials; par++) {
            
	    float freq = fundf + par*fundf*stretch + par*con;
	    freq += vtof(in[0][0] * 5.0f);
	    /*
	    if(par % 2 == 0){
		freq += in[1][0] * 100;
		freq += in[2][0] * 100;
	    } else {
		freq -= in[1][0] * 100;
	    }
	    if(par%3 == 2) {
		freq -= in[3][0] * 100;
	    }*/
	    if(sync) {
	        oscs[voice][par].Reset();
	        oscs[voice][par].PhaseAdd(in[3][0]*((float)par)/((float)partials));
	    }
            oscs[voice][par].SetFreq(freq);
            //oscs[j].SetFreq(fundf+(j*con));

            oscs[voice][par].SetWaveform(wave);
            a = 0;// (1 - abs(amp*2-1)) * 1; // middle: flat

            switch(amps) {
                case A_EXP:
                    if(amp <= 0.5) { // between 0 and 0.5
                        a += (1/(par*(2*(0.5-amp))+1)); // first half to middle: exponential 
                    } else { // between 0.5 and 1
                        a += (1/(1+(partials-par-1)*((amp-0.5)*2))); //middle to second half: exponential on reversed order
                    }
                    break;
                case A_LP:
                    if(par < amp*partials) {
                        a = 1;
                    } else if(par > amp*partials + mild){
                        a = 0;
                    } else {
                        a = 1 - (par - amp*partials)/mild;
                    }
                    break;
                case A_BP:
                    if(par < amp*partials - mild) {
                        a = 0;
                    } else if(par > amp*partials + mild){
                        a = 0;
                    } else if(par < amp*partials){
                        a = 1 - (amp*partials - par)/(mild);
                    } else if(par > amp*partials){
                        a = 1 - (par - amp*partials)/(mild);
                    }
                    break;
                case A_HP:
                    if(par < amp*partials) {
                        a = 0;
                    } else if(par > amp*partials + mild){
                        a = 1;
                    } else {
                        a = (par - amp*partials)/mild;
                    }
                    break;
                case A_NOTCH:
                    if(par < amp*partials - mild) {
                        a = 1;
                    } else if(par > amp*partials + mild){
                        a = 1;
                    } else if(par < amp*partials){
                        a = (amp*partials - par)/(mild);
                    } else if(par > amp*partials){
                        a = (par - amp*partials)/(mild);
                    }
                    break;
		case A_COMB:
		    {
			float offsetted = par + mild*2*amp;
			float tmp =fmod(offsetted,mild)/mild; 
			if( fmod((offsetted / mild), 2.f) >=1.f) {
			    a = tmp;
			} else {
			    a = 1 - tmp;
			}
		    }
		    break;
		case A_COMB_DIFFERENT:
		    {
			float curr_mild = ((float)mild) / (((float)par)/2.f + 1.f);
			float offsetted = ((float)par) + curr_mild*2*amp;
			float tmp =fmod(offsetted,curr_mild)/curr_mild;
			if( fmod((offsetted / curr_mild), 2.f) >=1.f) {
			    a = tmp;
			} else {
			    a = 1 - tmp;
			}
		    }
		    break;
            }
	    as[par] = a;
            oscs[voice][par].SetAmp(a);
        }
    }
    for (size_t i = 0; i < size; i ++)
    {
    	sum = 0;

        for (int par=0; par < partials; par++) {
            samples[par] = 0;
            for(int voice=0; voice<poly; voice++) {
                samples[par] += oscs[voice][par].Process();
            }
            sum += samples[par];
        }
        out[0][i] = sum/normalizer;
	    out[1][i] = samples[0];
	//out[2][i] = samples[partials-1];
	//out[3][i] = (sum - samples[0])/(normalizer-1);
    }
}

void UpdateNormalizer(void)
{
    switch(amps) {
	case A_EXP:
	case A_LP:
	case A_HP:
	    normalizer = poly*partials;
	    break;
	case A_BP:
	    normalizer = poly*mild;
	    break;
	case A_NOTCH:
	    normalizer = poly*(partials - mild);
	    break;
	case A_COMB:
	case A_COMB_DIFFERENT:
	    normalizer = poly * partials/2;
	    break;
    }
}

void UpdatePartialsAmount(int increment)
{
    partials = std::min(max_partials[wave],std::max(MIN_PARTIALS,(int)(partials + increment)));
}


int main(void)
{
    int screencycle = 0;
    float samplerate;
    hw.Init(); // Initialize hardware (daisy seed, and patch)
    samplerate = hw.AudioSampleRate();
   
    for(int voice=0; voice<MAX_VOICES; voice++){
	for(int par=0; par<MAXP; par++){
	    oscs[voice][par].Init(samplerate); // Init oscillator
        }
    }
    //fundctrl.Init(patch.controls[patch.CTRL_1], 10.0, 110.0f, Parameter::LINEAR);
    //stretchctrl.Init(patch.controls[patch.CTRL_2], 0.0, 5.0, Parameter::EXPONENTIAL);
    constantctrl.Init(hw.knob[KNOB_10], 0.0, 5000.0, Parameter::EXPONENTIAL);
    ampctrl.Init(hw.knob[KNOB_1], 0.0, 1.0f, Parameter::LINEAR);

    wavectrl.Init(hw.knob[KNOB_4], 0.0, num_waves, Parameter::LINEAR);
    partialsctrl.Init(hw.knob[KNOB_5], 0.0, 1.0f, Parameter::LINEAR);
    mildctrl.Init(hw.knob[KNOB_6], 0.0, 1.0f, Parameter::LINEAR);
    ampsctrl.Init(hw.knob[KNOB_7], 0.0, 1.0f, Parameter::LINEAR);
    
    UpdateNormalizer();
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);


    while (1) {
        
    }
}