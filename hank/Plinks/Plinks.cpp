#include "../daisy_hank.h"
#include "daisysp.h"
#include "arp_notes.h"

using namespace daisy;
using namespace daisysp;
using namespace arps;

DaisyHank		hw;
PolyPluck<32>		pp;
MoogLadder			ladder;
AdEnv      			env;

uint8_t     arp_idx;
uint8_t 	scale_idx;

// select scale, and pass midi note as nn 
float scaleSelect(float nn){
	float freq;
	

	switch (scale_idx)
	{
	case 1:
		freq = nn + noMajPen[arp_idx];
		break;
	case 2:
		freq = nn + noMinPen[arp_idx];
		break;
	case 3:
		freq = nn + noMajTri[arp_idx];
		break;
	case 4:
		freq = nn + noMinTri[arp_idx];
		break;
	default:
		
		
		break;
	}
	
	return freq;

	}

void UpdateDigital() {
	hw.s1.Debounce();
	hw.ClearLeds();
	if(hw.s1.FallingEdge()){
		scale_idx = (scale_idx + 1) % 5; // advance scale +1, else wrap back to 1? 
		if(scale_idx == 0) {
			scale_idx = 1;
		}
	}
	switch (scale_idx)
	{
	case 1:
		hw.SetRGBColor(DaisyHank::RGB_LED_1,hw.BLUE);
		break;
	case 2:
		hw.SetRGBColor(DaisyHank::RGB_LED_2,hw.BLUE);
		break;
	case 3:
		hw.SetRGBColor(DaisyHank::RGB_LED_3,hw.BLUE);
		break;
	case 4:
		hw.SetRGBColor(DaisyHank::RGB_LED_4,hw.BLUE);
		break;
	default:
		
		break;
	}
	hw.UpdateLeds();
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	
	float nn, trig, sig_out;
	trig = 0.f;

	float decay = hw.GetKnobValue(DaisyHank::KNOB_1);
	pp.SetDecay(decay);
	
	//float feedback = fmap(hw.GetAdcValue(CV_3), 0.f, 0.9f);
	//rv.SetFeedback(feedback);

	//float lpf = fmap(hw.GetAdcValue(CV_2), 1000.f, 32000.f, Mapping::LOG);
	//rv.SetLpFreq(lpf);

	float mlpf = fmap(hw.GetKnobValue(DaisyHank::KNOB_3), 1000.f, 32000.f, Mapping::LOG);
	ladder.SetFreq(mlpf);

	float attack = fmap(hw.GetKnobValue(DaisyHank::KNOB_2), 0.1f, 1.0f);
	float release = fmap(hw.GetKnobValue(DaisyHank::KNOB_4), 0.1f, 5.0f);
	env.SetTime(ADENV_SEG_ATTACK, attack);
	env.SetTime(ADENV_SEG_DECAY, release);

	// Set MIDI Note for new Pluck notes.
    nn = 24.0f + hw.GetCvValue(DaisyHank::CV_1) * 60.0f;
	//nn = 24.0f + 0.5f * 60.0f;
    nn = static_cast<int32_t>(nn); // Quantize to semitones

	for (size_t i = 0; i < size; i++)
	{
		if(hw.gate_in1.Trig()){
			arp_idx = (arp_idx + 1) % 5; // advance the kArpeggio, wrapping at the end.
			trig = 1.f;
			env.Trigger();
		}

		//if(hw.gate_in_2.Trig()){
		//	env.Trigger();
		//}

		float new_freq = scaleSelect(nn);

		sig_out = pp.Process(trig, new_freq);
		sig_out = ladder.Process(sig_out);
		float sig_out_env = sig_out * env.Process();
		
		//float wet;
		//rv.Process(sig_out, sig_out, &wet, &wet);
		// is this wet = softclip(wet) or just softclip(wet)?
		//wet = SoftClip(wet);

		OUT_L[i] = OUT_R[i] = sig_out_env * 1.f;

		//testing soft clip
		// also out = OUT never really made much sense anyway? 
		//out[0][i] = OUT_L[i] + (sig_out * .5f);
		//out[1][i] = OUT_R[i] + (sig_out_env * .5f);


	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	// init board
	//InitBed();
	
	// pluck init 
	pp.Init(sample_rate);

	// reverb init
	//rv.Init(sample_rate);

	// filter 
    ladder.Init(sample_rate);
    ladder.SetRes(0.4);

	// envelope
	env.Init(sample_rate);
	env.SetMin(0.0);
    env.SetMax(0.5);

	scale_idx = 1;

	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	while(1) {
		//bool state = hw.gate_in1.State();
		//gate_in_led.Write(state);
		UpdateDigital();
	}
}
