#include "../daisy_hank.h"
#include "daisysp.h"
#include "polypluckarp.h"
#include "arp.h"
#include "settings.h"
#include "ui.h"
#include <string>

using namespace daisy;
using namespace daisysp;
using namespace arp;


#define NUM_VOICES 32

// Hardware
DaisyHank hw;

using namespace plaits;
using namespace stmlib;

// Synthesis
PolyPluckArp<NUM_VOICES> synth;
ReverbSc                                  verb;

Settings settings;
Ui ui;

Arp arpeggiator;
Synthparams synthparams;


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

    if(hw.gate_in1.Trig())
    {
        trig = 1.0f;
        arpeggiator.Trig();
    }
        

    // Set MIDI Note for new Pluck notes.
    nn = 24.0f + synthparams.freq * 60.0f;
    nn = static_cast<int32_t>(nn); // Quantize to semitones



    // Read knobs for decay;
    damp = 0.5f + (synthparams.damp * 0.5f);
    synth.SetDamp(damp);

    

    verb.SetFeedback(synthparams.revfdb);
    verblpf = 2000.0f + (synthparams.revlpf * 6000.0f);
    verb.SetLpFreq(verblpf);

    
    

    // Synthesis.
    for(size_t i = 0; i < size; i++)
    {
              

        float new_freq = arpeggiator.GetArpNote(nn);
        
        // Synthesize Plucks
        sig = synth.Process(trig, new_freq);

        // Create Reverb Send
        dry  = sig;
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

    verb.Init(samplerate);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(2000.0f);

    arpeggiator.Init();
    


    settings.Init(&hw);
  
    ui.Init(&synthparams, &arpeggiator, &settings, &hw);

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
