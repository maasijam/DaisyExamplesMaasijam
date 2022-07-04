#include <array>

#include "../daisy_saul.h"
#include "daisysp.h"
#include "fx/frame.h"
#include "fx/reverb.h"
#include "DroneOsc.h"



using namespace daisy;
using namespace daisysp;

using namespace torus;


DaisySaul hw;

Reverb  reverb_;
Phaser     phaser;
MoogLadder moogfilter;
WhiteNoise wnoise;
Svf hpfilter;

float ph_wet;

float ph_freqtarget, ph_freq;
float ph_lfotarget, ph_lfo;
int   ph_numstages;

constexpr int NUM_TONES(6);
constexpr int NUM_POTS(NUM_TONES+2);	// tones pots plus 2 additional parameter pots
constexpr int pot_pins[NUM_POTS] = { 17, 18, 19, 20, 21, 22, 16, 15 };
int led_tone_set;

Parameter  cutoff_ctrl;

int resState = 0;
int noiseState = 0;

void Update_Leds();
void Update_Buttons();

struct ToneSet
{
	float	m_base_frequency;
	char	m_note;
	bool	m_is_sharp;
};

constexpr int NUM_TONE_SETS(12);
ToneSet tones_sets[NUM_TONE_SETS] = {	{55.0f, 'A', false },
										{58.27f, 'A', true },
										{61.74f, 'B', false },
										{65.41f, 'C', false },
										{69.30f, 'C', true },
										{73.42f, 'D', false },
										{77.78f, 'D', true },
										{82.41f, 'E', false },
										{87.31f, 'F', false },
										{92.50f, 'F', true },
										{98.00f, 'G', false },
										{103.83, 'G', true } };

enum class WAVE_SUM_TYPE
{
	AVERAGE,
	SINE_WAVE_FOLD,
	TRIANGLE_WAVE_FOLD,
};


WAVE_SUM_TYPE sum_type = WAVE_SUM_TYPE::AVERAGE;



DroneOscillator oscillators[NUM_TONES];
float gain = 0.0f;


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
	moogfilter.SetFreq(cutoff);
	if(resState == 1) {
		moogfilter.SetRes(0.4f);
		gainboost = 2.0f;
		wnoise_boost = 0.6f;
	} else if(resState == 2){
		moogfilter.SetRes(0.85f);
		gainboost = 3.0f;
		wnoise_boost = 0.4f;
	} else {
		moogfilter.SetRes(0.0f);
		gainboost = 1.0f;
		wnoise_boost = 1.0f;
	}

	
	
    
    for (size_t i = 0; i < size; i++)
	{
		float osc_out = 0.0f;
		for( int o = 0; o < NUM_TONES; ++o )
		{
			osc_out += oscillators[o].process();
		}

		switch(sum_type)
		{
			case WAVE_SUM_TYPE::AVERAGE:
			{
				osc_out /= NUM_TONES;
				break;
			}
			case WAVE_SUM_TYPE::SINE_WAVE_FOLD:
			{
				osc_out = sin_wave_fold(osc_out);
                osc_out *= 0.5f;
				break;
			}
			case WAVE_SUM_TYPE::TRIANGLE_WAVE_FOLD:
			{
				osc_out = triangular_wave_fold(osc_out);
                osc_out *= 0.4f;
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

		osc_out += wnoise_sig;

		osc_out = moogfilter.Process(osc_out);
		osc_out *= gainboost;


		

		

        ins_left[i] = osc_out;
        ins_right[i]= osc_out;

        
        reverb_.Process(&ins_left[i], &ins_right[i], 1);

		
		out[0][i] = ins_left[i] * gain;
        out[1][i] = ins_right[i] * gain;
        //out[0][i] = osc_out;
		//out[1][i] = osc_out;

        
	}
}

void set_tones(float base_frequency,float cv_voct)
{
	//hw.seed.PrintLine("BaseFreq: %f", base_frequency  );
	//onstexpr int NUM_INTERVALS(4);
	//const int intervals[NUM_INTERVALS] = { 12, 3, 4, 5 };	// ocatave, 3rd, 7th, octave
	constexpr int NUM_INTERVALS(5);
	const int intervals[NUM_INTERVALS] = { 12, 7, 5, 4, 3 };	// ocatave, 3rd, 7th, octave
	int interval = 0;
	float semitone = 0.f;
	for( int t = 0; t < NUM_TONES; ++t )
	{
		oscillators[t].set_freq(base_frequency, cv_voct, semitone);

		semitone				+= intervals[interval];
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
		osc.initialise(sample_rate);
	}

	moogfilter.Init(sample_rate);
	wnoise.Init();
	wnoise.SetAmp(0.1f);

	hpfilter.Init(sample_rate);

	//const float base_frequency = 65.41f; // C2
	//const float base_frequency(440);
	//set_tones(base_frequency,0.f);

	// NOTE: AGND and DGND must be connected for audio and ADC to work
    hw.StartAdc();
	hw.StartAudio(audio_callback);

    //hw.seed.StartLog(false);

	//int current_tone_set = 0;
	//const ToneSet& tone_set = tones_sets[current_tone_set];
//hw.seed.PrintLine("ToneSetBase: %d", current_tone_set  );	
	//set_tones(tone_set.m_base_frequency,0.f);

    int pot_map[6] = {0,4,5,6,7,3};

	while(1)
	{	
		hw.ProcessAllControls();
		Update_Buttons();
        Update_Leds();

        for( int t = 0; t < NUM_TONES; ++t )
		{
			const float pot_val = hw.knob[pot_map[t]].Value();
			oscillators[t].set_amplitude( pot_val );
			const float detune_val = hw.knob[8].Value();
			oscillators[t].set_detune( detune_val );
		}

		//const float pot1_val = hw.adc.GetFloat(NUM_TONES);
		gain = hw.knob[1].Value();

        float reverb_amount = hw.knob[10].Value() * 0.95f;
        //reverb_amount += feedback * (2.0f - feedback) * freeze_lp_;
       //CONSTRAIN(reverb_amount, 0.0f, 1.0f);

        reverb_.set_amount(reverb_amount * 0.54f);
        reverb_.set_diffusion(0.7f);
        reverb_.set_time(0.35f + 0.63f * reverb_amount);
        reverb_.set_input_gain(0.2f);
        reverb_.set_lp(0.6f + 0.37f * 0.01f);

        ph_wet = hw.knob[8].Value();

        ph_numstages = 8;
        phaser.SetPoles(ph_numstages);

        float k = 0.3f;
        phaser.SetLfoFreq(k * k * 20.f);
        ph_lfo  = hw.knob[3].Process();
        k    = 0.2f;
        ph_freq = k * k * 7000; //0 - 10 kHz, square curve
        phaser.SetFeedback(0.8f);


		// Moog Ladder
		//float cutoff = hw.knob[9].Value() * 0.95f;
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

        float toneVal = hw.knob[2].Value();
        //current_tone_set = toneVal > 10.98f ? 11 : static_cast<int>(std::floor(toneVal));
        //hw.seed.PrintLine("ToneSet: %d", current_tone_set  );
        
		//const ToneSet& tone_set = tones_sets[current_tone_set];
		set_tones(toneVal,0.f);

		//led_tone_set = current_tone_set;

        //wait 1 ms
        System::Delay(1);		
	}
}

void Update_Leds() {
    switch (led_tone_set)
        {
        case 0:
            hw.SetRGBLed(1, hw.off);
            hw.SetRGBLed(2, hw.off);
            hw.SetRGBLed(3, hw.off);
            hw.SetRGBLed(4, hw.off);
            break;
        case 1:
            hw.SetRGBLed(1, hw.off);
            hw.SetRGBLed(2, hw.off);
            hw.SetRGBLed(3, hw.off);
            hw.SetRGBLed(4, hw.green);
            break;
        case 2:
            hw.SetRGBLed(1, hw.off);
            hw.SetRGBLed(2, hw.off);
            hw.SetRGBLed(3, hw.green);
            hw.SetRGBLed(4, hw.off);
            break;
        case 3:
           hw.SetRGBLed(1, hw.off);
            hw.SetRGBLed(2, hw.off);
            hw.SetRGBLed(3, hw.green);
            hw.SetRGBLed(4, hw.green);
            break;
        case 4:
            hw.SetRGBLed(1, hw.off);
            hw.SetRGBLed(2, hw.green);
            hw.SetRGBLed(3, hw.off);
            hw.SetRGBLed(4, hw.off);
            break;
        case 5:
            hw.SetRGBLed(1, hw.off);
            hw.SetRGBLed(2, hw.green);
            hw.SetRGBLed(3, hw.off);
            hw.SetRGBLed(4, hw.green);
            break;
        case 6:
            hw.SetRGBLed(1, hw.off);
            hw.SetRGBLed(2, hw.green);
            hw.SetRGBLed(3, hw.green);
            hw.SetRGBLed(4, hw.off);
            break;
        case 7:
           hw.SetRGBLed(1, hw.off);
            hw.SetRGBLed(2, hw.green);
            hw.SetRGBLed(3, hw.green);
            hw.SetRGBLed(4, hw.green);
            break;
        case 8:
            hw.SetRGBLed(1, hw.green);
            hw.SetRGBLed(2, hw.off);
            hw.SetRGBLed(3, hw.off);
            hw.SetRGBLed(4, hw.off);
            break;
        case 9:
           hw.SetRGBLed(1, hw.green);
            hw.SetRGBLed(2, hw.off);
            hw.SetRGBLed(3, hw.off);
            hw.SetRGBLed(4, hw.green);
            break;
        case 10:
            hw.SetRGBLed(1, hw.green);
            hw.SetRGBLed(2, hw.off);
            hw.SetRGBLed(3, hw.green);
            hw.SetRGBLed(4, hw.off);
            break;
        case 11:
            hw.SetRGBLed(1, hw.green);
            hw.SetRGBLed(2, hw.off);
            hw.SetRGBLed(3, hw.green);
            hw.SetRGBLed(4, hw.green);
            break;
        default:
            hw.SetRGBLed(1, hw.off);
            hw.SetRGBLed(2, hw.off);
            hw.SetRGBLed(3, hw.off);
            hw.SetRGBLed(4, hw.off);
            break;
        }

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
}