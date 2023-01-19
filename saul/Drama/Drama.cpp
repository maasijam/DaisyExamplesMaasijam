#include <array>

#include "../daisy_saul.h"
#include "daisysp.h"
#include "fx/frame.h"
#include "fx/reverb.h"
#include "DroneOsc.h"



using namespace daisy;
using namespace daisysp;
using namespace clouds_reverb;
using namespace saul;

DaisySaul hw;

Reverb  reverb_;
MoogLadder moogfilter_l, moogfilter_r;
WhiteNoise wnoise;
Svf hpfilter;
static CrossFade WidthXfade;

#define NUM_WAVEFORMS 4

constexpr int NUM_TONES(4);

Parameter  cutoff_ctrl, drone_width, drone_detune;

static int resState = 0;
static int noiseState = 0;
static int waveformState = 0;
static int intervalSetState = 0;

void Update_Leds();
void Update_Buttons();


enum class WAVE_SUM_TYPE
{
	AVERAGE,
	SINE_WAVE_FOLD,
	TRIANGLE_WAVE_FOLD,
};

uint8_t waveforms[NUM_WAVEFORMS] = {
    Oscillator::WAVE_SIN,
    Oscillator::WAVE_POLYBLEP_TRI,
    Oscillator::WAVE_POLYBLEP_SAW,
    Oscillator::WAVE_POLYBLEP_SQUARE,
};


WAVE_SUM_TYPE sum_type = WAVE_SUM_TYPE::AVERAGE;


DroneOscillator oscillators[NUM_TONES];
float gain = 0.0f;

constexpr int NUM_INTERVALS(4);
constexpr int NUM_INTERVAL_SETS(2);
const int intervals[NUM_INTERVAL_SETS][NUM_INTERVALS] = {
    { 12, 7, 4, 3 },
    { 12, 8, 9 ,10}
};	// ocatave, 3rd, 7th, octave


// https://www.desmos.com/calculator/ge2wvg2wgj
float triangular_wave_fold( float in )
{
	const float q_in = in * 0.25f;
	return 4 * (abs(q_in + 0.25f - roundf(q_in+0.25f))-0.25f);
}

float sin_wave_fold( float in )
{
	return sinf(in);
}

void audio_callback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float ins_left[48];
    float ins_right[48];
	float cutoff = cutoff_ctrl.Process();
	float hpcutoff = 2000.0f;
	float gainboost, wnoise_sig, wnoise_boost;
	moogfilter_l.SetFreq(cutoff);
    moogfilter_r.SetFreq(cutoff);
	if(resState == 1) {
		moogfilter_l.SetRes(0.4f);
        moogfilter_r.SetRes(0.4f);
		gainboost = 2.0f;
		wnoise_boost = 0.6f;
	} else if(resState == 2){
		moogfilter_l.SetRes(0.85f);
        moogfilter_r.SetRes(0.85f);
		gainboost = 3.0f;
		wnoise_boost = 0.4f;
	} else {
		moogfilter_l.SetRes(0.0f);
        moogfilter_r.SetRes(0.0f);
		gainboost = 1.0f;
		wnoise_boost = 1.0f;
	}

	
	
    
    for (size_t i = 0; i < size; i++)
	{
		float osc_out_left = 0.0f;
        float osc_out_right = 0.0f;
		for( int o = 0; o < NUM_TONES; ++o )
		{
			oscillators[o].set_waveform(waveforms[waveformState]);
            osc_out_left += oscillators[o].process_left();
            osc_out_right += oscillators[o].process_right();
		}

		switch(sum_type)
		{
			case WAVE_SUM_TYPE::AVERAGE:
			{
				osc_out_left /= NUM_TONES;
                osc_out_right /= NUM_TONES;
				break;
			}
			case WAVE_SUM_TYPE::SINE_WAVE_FOLD:
			{
				osc_out_left = sin_wave_fold(osc_out_left);
                osc_out_left *= 0.5f;
                osc_out_right = sin_wave_fold(osc_out_right);
                osc_out_right *= 0.5f;
				break;
			}
			case WAVE_SUM_TYPE::TRIANGLE_WAVE_FOLD:
			{
				osc_out_left = triangular_wave_fold(osc_out_left);
                osc_out_left *= 0.4f;
                osc_out_right = triangular_wave_fold(osc_out_right);
                osc_out_right *= 0.4f;
                break;
			}
		}

		if(noiseState == 1) {
			wnoise_sig = wnoise.Process();
		} else {
			wnoise_sig = 0.0f;
		}
		hpfilter.SetFreq(hpcutoff);
		hpfilter.Process(wnoise_sig);
		wnoise_sig = hpfilter.High();
		wnoise_sig *= wnoise_boost;

		osc_out_left += wnoise_sig;
        osc_out_right += wnoise_sig;

		osc_out_left = moogfilter_l.Process(osc_out_left);
		osc_out_left *= gainboost;
        osc_out_right = moogfilter_r.Process(osc_out_right);
		osc_out_right *= gainboost;


		float osc_out_left_SUM = WidthXfade.Process(osc_out_left,osc_out_right);    //mix to mono if width 0.0
        float osc_out_right_SUM = WidthXfade.Process(osc_out_right,osc_out_left);

		

        ins_left[i] = osc_out_left_SUM;
        ins_right[i]= osc_out_right_SUM;

        
        reverb_.Process(&ins_left[i], &ins_right[i], 1);

		
		out[0][i] = ins_left[i] * gain;
        out[1][i] = ins_right[i] * gain;
        
	}
}

void set_tones(float base_frequency,float cv_voct)
{
	
	int interval = 0;
	float semitone = 0.f;
	for( int t = 0; t < NUM_TONES; ++t )
	{
		float rdm = hw.GetRandomFloat(0.f, 0.05f);
        oscillators[t].set_freq(base_frequency, cv_voct, semitone, rdm);

		semitone				+= intervals[intervalSetState][interval];
		interval				= ( interval + 1 ) % NUM_INTERVALS;
	}
}

int main(void)
{
	hw.Init();

    //Set up oscillators
	const float sample_rate = hw.AudioSampleRate();
	for( DroneOscillator& osc : oscillators )
	{
		osc.init(sample_rate);
	}

    WidthXfade.Init();
    WidthXfade.SetCurve(CROSSFADE_CPOW);

    drone_width.Init(hw.knob[10], 0.5f, 0.0f, Parameter::LINEAR);
    drone_detune.Init(hw.knob[0], 0.0f, 0.5f, Parameter::LINEAR);

	moogfilter_l.Init(sample_rate);
    moogfilter_r.Init(sample_rate);
	wnoise.Init();
	wnoise.SetAmp(0.1f);

	hpfilter.Init(sample_rate);

    hw.StartAdc();
	hw.StartAudio(audio_callback);

    int pot_map[4] = {4,5,6,7};

	while(1)
	{	
		hw.ProcessAllControls();
		Update_Buttons();
        Update_Leds();

        for( int t = 0; t < NUM_TONES; ++t )
		{
			const float pot_val = hw.knob[pot_map[t]].Value();
			oscillators[t].set_amplitude( pot_val );
			const float detune_val = drone_detune.Process();
			oscillators[t].set_detune( detune_val );
		}

        float width = drone_width.Process();
        WidthXfade.SetPos(width);
		gain = hw.knob[1].Value();

        float reverb_amount = hw.knob[3].Value() * 0.95f;

        reverb_.set_amount(reverb_amount * 0.54f);
        reverb_.set_diffusion(0.7f);
        reverb_.set_time(0.35f + 0.63f * reverb_amount);
        reverb_.set_input_gain(0.2f);
        reverb_.set_lp(0.6f + 0.37f * 0.01f);

		// Moog Ladder
		cutoff_ctrl.Init(hw.knob[9], 100, 20000, Parameter::LOGARITHMIC);


		if( hw.sw[0].Read() == 0)
		{
			sum_type = WAVE_SUM_TYPE::AVERAGE;
		}
		else if( hw.sw[0].Read() == 1 )
		{
			sum_type = WAVE_SUM_TYPE::SINE_WAVE_FOLD;
		}
		else if( hw.sw[0].Read() == 2 )
		{
			sum_type = WAVE_SUM_TYPE::TRIANGLE_WAVE_FOLD;
		}

        float toneVal = hw.knob[8].Value();
        
		set_tones(toneVal,0.f);

        //wait 1 ms
        System::Delay(1);		
	}
}

void Update_Leds() {
   

	switch (resState)
    {
    case 1:
        hw.SetLed(4,true);
        hw.SetLed(6,false);
        break;
    case 2:
        hw.SetLed(4,false);
        hw.SetLed(6,true);
        break;
    default:
        hw.SetLed(4,true);
        hw.SetLed(6,true);
        break;
    }

	switch (noiseState)
    {
    case 1:
        hw.SetLed(0,false);
        hw.SetLed(2,true);
        break;
    case 2:
        hw.SetLed(0,true);
        hw.SetLed(2,true);
        break;
    default:
        hw.SetLed(0,true);
        hw.SetLed(2,true);
        break;
    }

    switch (waveformState)
    {
    case 1:
        hw.SetRGBLed(1, BLUE);
        
        break;
    case 2:
        hw.SetRGBLed(1, YELLOW);
       
        break;
    case 3:
        hw.SetRGBLed(1, PURPLE);
       
        break;
    default:
        hw.SetRGBLed(1, GREEN);
       
        break;
    }

    switch (intervalSetState)
    {
    case 1:
        hw.SetRGBLed(4, BLUE);
        
        break;
    default:
        hw.SetRGBLed(4, GREEN);
       
        break;
    }
}

void Update_Buttons() {
	if(hw.s[4].RisingEdge()){
        resState += 1;
        if(resState > 2) {
           resState = 0;     
        } 
    }
	if(hw.s[0].RisingEdge()){
        noiseState += 1;
        if(noiseState > 1) {
           noiseState = 0;     
        } 
    }
    if(hw.s[3].RisingEdge()){
        waveformState += 1;
        if(waveformState > 3) {
           waveformState = 0;     
        } 
    }
    if(hw.s[2].RisingEdge()){
        intervalSetState += 1;
        if(intervalSetState > 1) {
           intervalSetState = 0;     
        } 
    }
}