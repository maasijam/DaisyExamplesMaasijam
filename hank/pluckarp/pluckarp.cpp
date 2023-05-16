#include "../daisy_hank.h"
#include "daisysp.h"
#include "polypluckarp.h"
#include "arp.h"
#include "settings.h"
#include "ui.h"
#include <string>

using namespace daisy;
using namespace daisysp;
using namespace arps;


#define NUM_VOICES 32
#define MAX_DELAY ((size_t)(10.0f * 48000.0f))


// Hardware
DaisyHank hw;



using namespace plaits;
using namespace stmlib;

// Synthesis
PolyPluckArp<NUM_VOICES> synth;
// 10 second delay line on the external SDRAM
//DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delay;
ReverbSc                                  verb;

Settings settings;
Ui ui;

// Persistent filtered Value for smooth delay time changes.
//float smooth_time;

inline int irand(int min, int max) {
    return rand()%(max-min + 1) + min;
}


bool firstrun = true;

ArpSettings arpsettings;

Synthparams synthparams;



int GetIdxByDirection(int idx, int chordlen)
{
    switch (static_cast<int>(arpsettings.chordDirection*5)) {
    case DOWN:
    {
        if((chordlen-1) == idx) {arpsettings.slotChange = true;} else {arpsettings.slotChange = false;}
        return (chordlen-1)-idx;
    }
    case UP_DOWN_IN:
    {
        arpsettings.slotChange = false;
        if(firstrun)
        {
            if((chordlen-1) == idx) {
                firstrun = false;
            }
            return idx;
        } else {
            if((chordlen-1) == idx) {
                firstrun = true;
                arpsettings.slotChange = true;
            } 
            return (chordlen-1)-idx;
        }
        
    }
    case UP_DOWN_EX:
    {
        arpsettings.slotChange = false;
        if(firstrun)
        {
            if((chordlen-1) == idx) {
                firstrun = false;
            }
            return idx;
        } else {
            if((chordlen-2) == idx) {
                firstrun = true;
                arpsettings.slotChange = true;
            }
            return (chordlen-2)-idx;
        }
    }
    case RAND:
    {
        //return irand(0, (chordlen-1));
        if((chordlen-1) == idx) {arpsettings.slotChange = true;} else {arpsettings.slotChange = false;}
        return rand() % chordlen;
    }
    default:
    {
        if((chordlen-1) == idx) {arpsettings.slotChange = true;} else {arpsettings.slotChange = false;}
        return idx;
    }
      
    } 
}

// select scale, and pass midi note as nn 
float chordSelect(float nn){
	float freq;
    int cLen = scaleChords[static_cast<int>(arpsettings.scaleIdx*3)][static_cast<int>(arpsettings.slotChordIdx[arpsettings.chord_slot_idx]*12)].num_notes;
    int cIdx = GetIdxByDirection(arpsettings.arp_idx,cLen);
	freq = nn + scaleChords[static_cast<int>(arpsettings.scaleIdx*3)][static_cast<int>(arpsettings.slotChordIdx[arpsettings.chord_slot_idx]*12)].notes[cIdx];	
	return freq;

}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    float sig;           // Mono Audio Vars
    float trig, nn, damp;       // Pluck Vars
    //float deltime, delfb, kval;  // Delay Vars
    float dry, send, wetl, wetr, verblpf; // Effects Vars
    

    

    // Assign Output Buffers
    float *out_left, *out_right;
    out_left  = out[0];
    out_right = out[1];

    
    //hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();

    ui.Poll();

    // Handle Triggering the Plucks
    trig = 0.0f;
    int chordLength = scaleChords[static_cast<int>(arpsettings.scaleIdx*3)][static_cast<int>(arpsettings.slotChordIdx[arpsettings.chord_slot_idx]*12)].num_notes;

    if(hw.gate_in1.Trig())
    {
        //if(arpsettings.arp_idx == (chordLength - 1)) {
        if(arpsettings.slotChange) {
            arpsettings.chord_slot_idx = (arpsettings.chord_slot_idx + 1) % 4;
        }
        arpsettings.arp_idx = (arpsettings.arp_idx + 1) % chordLength; // advance the kArpeggio, wrapping at the end.
        trig = 1.0f;
    }
        

    // Set MIDI Note for new Pluck notes.
    nn = 24.0f + synthparams.freq * 60.0f;
    nn = static_cast<int32_t>(nn); // Quantize to semitones



    // Read knobs for decay;
    damp = 0.5f + (synthparams.damp * 0.5f);
    synth.SetDamp(damp);

    // Read knobs for decay;
    //decay = 0.3f + (synthparams.damp * 0.7f);
    //synth.SetDecay(decay);

    // Get Delay Parameters from knobs.
    //kval    = synthparams.deltime;
    //deltime = (0.001f + (kval * kval) * 5.0f) * samplerate;
    //delfb   = synthparams.delfdbk;

    verb.SetFeedback(synthparams.revfdb);
    verblpf = 2000.0f + (synthparams.revlpf * 6000.0f);
    verb.SetLpFreq(verblpf);

    // Synthesis.
    for(size_t i = 0; i < size; i++)
    {
        // Smooth delaytime, and set.
        //fonepole(smooth_time, deltime, 0.0005f);
        //delay.SetDelay(smooth_time);
        //if(synthparams.revfdb >0.75f){
           //drylevel =  1 - synthparams.revfdb;
        //} else {
        //    drylevel = 1;
        //}

        float new_freq = chordSelect(nn);

        // Synthesize Plucks
        sig = synth.Process(trig, new_freq);

        //		// Handle Delay
        //delsig = delay.Read();
        //delay.Write(sig + (delsig * delfb));

        // Create Reverb Send
        dry  = sig;
        send = dry * 0.6f;
        verb.Process(send, send, &wetl, &wetr);

        // Output
        out_left[i]  = dry + wetl;
        out_right[i] = dry + wetr;
        //out_left[i]  = (dry * drylevel) + wetl;
        //out_right[i] = (dry * drylevel) + wetr;
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

    //delay.Init();
    //delay.SetDelay(samplerate * 0.8f); // half second delay

    verb.Init(samplerate);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(2000.0f);


    arpsettings.chord_slot_idx = 0;
    arpsettings.slotChange = false;


    settings.Init(&hw);
  
  ui.Init(&synthparams, &arpsettings, &settings, &hw);

    // Start the ADC and Audio Peripherals on the Hardware
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    uint32_t last_save_time = System::GetNow(); 

  while (1) {
    
        if (hw.ReadyToSaveCal()) {
            ui.SaveCalibrationData();
        }
        if (System::GetNow() - last_save_time > 100 && ui.readyToSaveState)
        {
          ui.SaveState();
          last_save_time = System::GetNow();
          ui.readyToSaveState = false;
        }
        if (ui.readyToRestore) {
            /** Collect the data from where ever in the application it is */
            settings.RestoreState();
            ui.readyToRestore = false;
        }
  }
}
