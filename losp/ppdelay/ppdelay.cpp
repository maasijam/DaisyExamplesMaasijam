#include "../daisy_losp.h"
#include "daisysp.h"
#include <string>
#include "taptempo.h"
#include "LEDs.h"
#include "constants.h"


using namespace daisy;
using namespace daisysp;

DaisyLosp hw;

#define MAX_DELAY ((size_t)(5.0f * 48000.0f))

static DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS dell;
static DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delr;

//Tap tempo
Taptempo BaseTempo; 
TempoLED tempoLED_BASE;



float smooth_time;
float tempo;

void UpdateClock();
void Update_Buttons();
void Update_BaseTempoLED();


// This runs at a fixed rate, to prepare audio samples
void callback(AudioHandle::InputBuffer  in,
                AudioHandle::OutputBuffer out,
                size_t                    size)
{
        float dry_sig, dellsig, delrsig;
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

        // Get Delay Parameters from knobs.
        kval    = hw.GetKnobValue(DaisyLosp::CONTROL_KNOB_TOP);
        deltime = (0.001f + (kval * kval) * 5.0f) * samplerate;
        delfb   = hw.GetKnobValue(DaisyLosp::CONTROL_KNOB_BOTTOM);
    
        // Audio is interleaved stereo by default
        for(size_t i = 0; i < size; i ++)
        {
            UpdateClock(); 
            Update_BaseTempoLED();
            
            float in_left = in[0][i];
            float in_right = in[1][i];
            
            // Smooth delaytime, and set.
            fonepole(smooth_time, (tempo), 0.0005f);
            dell.SetDelay(smooth_time);
            delr.SetDelay(smooth_time);

            dry_sig = in_left + in_right;

            dellsig = dell.Read();
            delrsig = delr.Read();

            dell.Write(dry_sig + (delrsig * delfb));           
            delr.Write((dellsig * delfb));

            // Output
            out_left[i]  = dellsig;
            out_right[i] = delrsig;
        }
}

int main(void)
{
    // Initialize Versio hardware and start audio, ADC
    float samplerate;
    hw.Init();
    samplerate = hw.AudioSampleRate();

    dell.Init();
    delr.Init();

    dell.SetDelay(samplerate * 0.8f); // half second delay
    delr.SetDelay(samplerate * 0.8f);

    BaseTempo.setTempo(24000.0f);
        

    //setup tempo indicators
    //tempoLED_BASE.init(hw.GetPin(14),hw.AudioSampleRate());
    tempoLED_BASE.init(hw.seed.GetPin(PIN_LED0_G),hw.AudioSampleRate());
    tempoLED_BASE.setTempo(BaseTempo.getTapFreq());
    tempoLED_BASE.resetPhase();


    hw.StartAudio(callback);
    hw.StartAdc();

hw.seed.StartLog(false);
    while(1)
    {
        //UpdateClock();
        Update_Buttons();
        hw.seed.PrintLine("Tempo: %f", tempo);
    }
}

void UpdateClock()
{
    static uint32_t ClockCounter{};

    ClockCounter += 1; //increment by one
    //if clock in pulse received
    if (hw.gate.Trig())     
    {   
        
        //tempoLED_BASE.resetPhase();
            if(BaseTempo.clock(ClockCounter)) //if valid tap resistered
            {
                tempoLED_BASE.setTempo(BaseTempo.getTapFreq()); //set new base freq
                tempo = BaseTempo.getTempo();
                //save_flag = true;
            }
            ClockCounter = 0; //reset counter

    } 
}

void Update_Buttons()
{      
    //hw.encoder.update();
    
    if (hw.encoder.RisingEdge())    
    {
        if(BaseTempo.tap()) //if tempo changed
        {
            tempoLED_BASE.setTempo(BaseTempo.getTapFreq());
            tempo = BaseTempo.getTempo();
            
        }
        //tempoLED_BASE.resetPhase();

    }

    if (hw.encoder.FallingEdge())    //when button is let go shift is off
    {
       
    }

}

void Update_BaseTempoLED()
{
    tempoLED_BASE.update();

    static int phaseCounter{};

    if(tempoLED_BASE.isEOC())
    {
        phaseCounter = (phaseCounter + 1) % 6;
    }

    float dividedPhase{(tempoLED_BASE.GetPhase() / 6) + ( (TWOPI_F / 6) * phaseCounter ) };
    //update base phase for both delay lines
    //delayL.SetBasePhase( dividedPhase );
    //delayR.SetBasePhase( dividedPhase );
    //PHASE_DEBUG = dividedPhase;
}