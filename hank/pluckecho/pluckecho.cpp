#include "../daisy_hank.h"
#include "daisysp.h"
#include "arp_notes.h"
#include <string>

using namespace daisy;
using namespace daisysp;
using namespace arps;

#define NUM_VOICES 32
#define MAX_DELAY ((size_t)(10.0f * 48000.0f))


// Hardware
DaisyHank hw;

// Synthesis
PolyPluck<NUM_VOICES> synth;
// 10 second delay line on the external SDRAM
DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delay;
ReverbSc                                  verb;

// Persistent filtered Value for smooth delay time changes.
float smooth_time;

uint8_t     arp_idx;
uint8_t 	scale_idx;
uint8_t 	chord_idx;
uint8_t 	chord_slot_idx;

int chordSlot[4] = {0};

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

// select scale, and pass midi note as nn 
float chordSelect(float nn){
	float freq;
	freq = nn + chords[chordSlot[chord_slot_idx]].notes[arp_idx];

	/*switch (chord_idx)
	{
	case I:
		freq = nn + chords[chord_idx].notes[arp_idx];
		break;
	case IS2:
		freq = nn + chords[chord_idx].notes[arp_idx];
		break;
	case IS4:
		freq = nn + chords[chord_idx].notes[arp_idx];
		break;
	case I7:
		freq = nn + chords[chord_idx].notes[arp_idx];
		break;
	default:
		
		
		break;
	}*/
	
	return freq;

	}

void UpdateDigital() {
	hw.s1.Debounce();
	hw.ClearLeds();
	if(hw.s1.FallingEdge()){
		//chord_idx = (chord_idx + 1) % 12; // advance scale +1, else wrap back to 1? 
		//if(chord_idx == 0) {
		//	chord_idx = 0;
		//}
	}
	switch (chord_slot_idx)
	{
	case 0:
		hw.SetRGBColor(DaisyHank::RGB_LED_1,hw.BLUE);
		break;
	case 1:
		hw.SetRGBColor(DaisyHank::RGB_LED_2,hw.BLUE);
		break;
	case 2:
		hw.SetRGBColor(DaisyHank::RGB_LED_3,hw.BLUE);
		break;
	case 3:
		hw.SetRGBColor(DaisyHank::RGB_LED_4,hw.BLUE);
		break;
   
	default:
		
		break;
	}
	hw.UpdateLeds();
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    float sig, delsig;           // Mono Audio Vars
    float trig, nn, decay;       // Pluck Vars
    float deltime, delfb, kval;  // Delay Vars
    float dry, send, wetl, wetr; // Effects Vars
    float samplerate;

    // Assign Output Buffers
    float *out_left, *out_right;
    out_left  = out[0];
    out_right = out[1];

    samplerate = hw.AudioSampleRate();
    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();

    // Handle Triggering the Plucks
    trig = 0.0f;
    int chordLength = chords[chordSlot[chord_slot_idx]].num_notes;
    if(hw.gate_in1.Trig())
    {
        if(arp_idx == (chordLength - 1)) {
            chord_slot_idx = (chord_slot_idx + 1) % 4;
        }
        arp_idx = (arp_idx + 1) % chordLength; // advance the kArpeggio, wrapping at the end.
        trig = 1.0f;
    }
        

    // Set MIDI Note for new Pluck notes.
    nn = 24.0f + hw.GetKnobValue(DaisyHank::KNOB_1) * 60.0f;
    nn = static_cast<int32_t>(nn); // Quantize to semitones



    // Read knobs for decay;
    decay = 0.5f + (hw.GetKnobValue(DaisyHank::KNOB_2) * 0.5f);
    synth.SetDecay(decay);

    // Get Delay Parameters from knobs.
    kval    = hw.GetKnobValue(DaisyHank::KNOB_3);
    deltime = (0.001f + (kval * kval) * 5.0f) * samplerate;
    delfb   = hw.GetKnobValue(DaisyHank::KNOB_4);

    // Synthesis.
    for(size_t i = 0; i < size; i++)
    {
        // Smooth delaytime, and set.
        fonepole(smooth_time, deltime, 0.0005f);
        delay.SetDelay(smooth_time);

        float new_freq = chordSelect(nn);

        // Synthesize Plucks
        sig = synth.Process(trig, new_freq);

        //		// Handle Delay
        delsig = delay.Read();
        delay.Write(sig + (delsig * delfb));

        // Create Reverb Send
        dry  = sig + delsig;
        send = dry * 0.6f;
        verb.Process(send, send, &wetl, &wetr);

        // Output
        out_left[i]  = dry + wetl;
        out_right[i] = dry + wetr;
    }
}

int main(void)
{
    // Init everything.
    float samplerate;
    hw.Init();
    hw.SetAudioBlockSize(4);
    samplerate = hw.AudioSampleRate();

    hw.ClearLeds();
    hw.UpdateLeds();

    synth.Init(samplerate);

    delay.Init();
    delay.SetDelay(samplerate * 0.8f); // half second delay

    verb.Init(samplerate);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(2000.0f);

    scale_idx = 1;
    chord_idx = 0;
    chord_slot_idx = 0;

    chordSlot[0] = I;
    chordSlot[1] = I7;
    chordSlot[2] = IV;
    chordSlot[3] = V7;

    // Start the ADC and Audio Peripherals on the Hardware
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    for(;;)
    {
        UpdateDigital();
    }
}
