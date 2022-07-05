#include <array>

#include "../daisy_saul.h"
#include "daisysp.h"
#include "DroneOsc.h"



using namespace daisy;
using namespace daisysp;


DaisySaul hw;


constexpr int NUM_TONES(4);
#define NUM_WAVEFORMS 4

static CrossFade WidthXfade;

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

static int waveformState = 0;
static int intervalSetState = 0;

Parameter  drone_width, drone_detune;

uint8_t waveforms[NUM_WAVEFORMS] = {
    Oscillator::WAVE_SIN,
    Oscillator::WAVE_TRI,
    Oscillator::WAVE_POLYBLEP_SAW,
    Oscillator::WAVE_POLYBLEP_SQUARE,
};

void Update_Buttons();
void Update_Leds();

constexpr int NUM_INTERVALS(4);
constexpr int NUM_INTERVAL_SETS(2);
const int intervals[NUM_INTERVAL_SETS][NUM_INTERVALS] = {
    { 12, 7, 4, 3 },
    { 3, 4, 5 ,9}
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


        float osc_out_left_SUM = WidthXfade.Process(osc_out_left,osc_out_right);    //mix to mono if width 0.0
        float osc_out_right_SUM = WidthXfade.Process(osc_out_right,osc_out_left);

		

        ins_left[i] = osc_out_left_SUM;
        ins_right[i]= osc_out_right_SUM;
    		
		out[0][i] = ins_left[i] * gain;
        out[1][i] = ins_right[i] * gain;
        
	}
}

void set_tones(float base_frequency,float cv_voct)
{
	//hw.seed.PrintLine("BaseFreq: %f", base_frequency  );
	//onstexpr int NUM_INTERVALS(4);
	//const int intervals[NUM_INTERVALS] = { 12, 3, 4, 5 };	// ocatave, 3rd, 7th, octave
	
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
    drone_detune.Init(hw.knob[8], 0.0f, 0.5f, Parameter::LINEAR);

	
	// NOTE: AGND and DGND must be connected for audio and ADC to work
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


		//const float pot1_val = hw.adc.GetFloat(NUM_TONES);
		gain = hw.knob[1].Value();

        WidthXfade.SetPos(width);

		

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


void Update_Buttons() {
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

void Update_Leds() {
    

	switch (waveformState)
    {
    case 1:
        hw.SetRGBLed(1, hw.blue);
        
        break;
    case 2:
        hw.SetRGBLed(1, hw.yellow);
       
        break;
    case 3:
        hw.SetRGBLed(1, hw.purple);
       
        break;
    default:
        hw.SetRGBLed(1, hw.green);
       
        break;
    }

    switch (intervalSetState)
    {
    case 1:
        hw.SetRGBLed(4, hw.blue);
        
        break;
    default:
        hw.SetRGBLed(4, hw.green);
       
        break;
    }

	
}