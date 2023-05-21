#include "../daisy_hank.h"
#include "daisysp.h"
#include "arp.h"
#include "settings.h"
#include "ui.h"
#include <string>

using namespace daisy;
using namespace daisysp;
using namespace arp;




// Hardware
DaisyHank hw;

using namespace plaits;
using namespace stmlib;

// Synthesis

ReverbSc verb;
Fm2 synth;
AdEnv      env;

Settings settings;
Ui ui;

Arp arpeggiator;
Synthparams synthparams;

float envVal;


void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    float sig;           // Mono Audio Vars
    float trig, nn, damp;       // Pluck Vars
    //float deltime, delfb, kval;  // Delay Vars
    float dry, send, wetl, wetr, verblpf; // Effects Vars
    float synthfreq;
    

    // Assign Output Buffers
    float *out_left, *out_right;
    out_left  = out[0];
    out_right = out[1];

    
    //hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();

    ui.Poll();

    // Handle Triggering the Plucks
    trig = 0.0f;

    //env.SetTime(ADENV_SEG_ATTACK, synthparams.revfdb);
    env.SetTime(ADENV_SEG_ATTACK, 0.005);
    env.SetTime(ADENV_SEG_DECAY, synthparams.revlpf);

    if(hw.gate_in1.Trig())
    {
        trig = 1.0f;
        arpeggiator.Trig();
        env.Trigger();
    }

    float knob_coarse = synthparams.freq;
    float coarse_tune = fmap(knob_coarse, 12, 84); 
    float new_freq = arpeggiator.GetArpNote(coarse_tune);
    float midi_nn = fclamp(new_freq, 0.f, 127.f);  

    // Set MIDI Note for new Pluck notes.
    //nn = 24.0f + synthparams.freq * 60.0f;
    //nn = static_cast<int32_t>(nn); // Quantize to semitones

     //synth.SetFrequency(synthparams.damp);  
    synthfreq = mtof(midi_nn);
    synth.SetFrequency(synthfreq);

    synth.SetIndex(synthparams.damp);
    synth.SetRatio(synthparams.revfdb);
    // Synthesis.
    for(size_t i = 0; i < size; i++)
    {
              

        //float new_freq = arpeggiator.GetArpNote(nn);

        envVal   = env.Process();
        
        // Synthesize Plucks
        sig = synth.Process();

        // Create Reverb Send
        dry  = sig;
        //send = dry * 0.6f;
        //verb.Process(send, send, &wetl, &wetr);

        // Output
        out_left[i]  = dry * envVal;
        out_right[i] = dry * envVal;
        
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
    env.Init(samplerate);
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
