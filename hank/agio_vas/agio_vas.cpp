#include "../daisy_hank.h"
#include "daisysp.h"
#include "vasynth.h"
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
float sysSampleRate;
float sysCallbackRate;
extern uint8_t preset_max;
extern VASynthSetting preset_setting[PRESET_MAX];

uint8_t param_;
float pitch_bend = 1.0f;
float master_tune = 0.0f;

VASynth vasynth;
ReverbSc                                  verb;

Settings settings;
Ui ui;

Arp arpeggiator;
Synthparams synthparams;

uint8_t gPlay = PLAY_ON;


void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{	
    float voice_left, voice_right;
    float nn;

    hw.ProcessAnalogControls();

    ui.Poll();
    //hw.ClearLeds();

    // Set MIDI Note for new Pluck notes.
    nn = 24.0f + 0.5 * 60.0f;
    nn = static_cast<int32_t>(nn); // Quantize to semitones

    // Handle Triggering the Plucks
    if(hw.gate_in1.Trig())
    {
        arpeggiator.Trig();
    }
    float new_freq = arpeggiator.GetArpNote(nn);
        if(!hw.GateIn1())
        {
        
            vasynth.NoteOn(static_cast<u_int8_t>(new_freq),127);
            //hw.SetRGBColor(DaisyHank::RGB_LED_1,DaisyHank::BLUE );
        } else {
            vasynth.NoteOff(static_cast<u_int8_t>(new_freq));
            //hw.SetRGBColor(DaisyHank::RGB_LED_1,DaisyHank::OFF );
        }
    
    
    
    //hw.UpdateLeds();
    for (size_t n = 0; n < size; n += 2)
	{	
		
        
        
        
        if (gPlay == PLAY_ON)
		{			
			
            
            vasynth.Process(&voice_left, &voice_right);
			
			out[n] = voice_left + in[n];;
			out[n + 1] = voice_right + in[n + 1];;	
		} 
		else 
		{
			out[n] = 0;
			out[n + 1] = 0;		
		}		
	}
        
}

int main(void)
{
    // Init everything.
    float samplerate;
    hw.Init(true);
    hw.SetAudioBlockSize(4);
    samplerate = hw.AudioSampleRate();
    sysSampleRate = hw.AudioSampleRate();
	sysCallbackRate = hw.AudioCallbackRate();

    hw.ClearLeds();
    hw.UpdateLeds();

    vasynth.First();

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
