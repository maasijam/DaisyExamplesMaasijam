#include <stdio.h>
#include <string.h>
#include "../daisy_16rgb.h"
#include "daisysp.h"


#define KNOB_ID "KNOB_"
#define SW_ID "SWITCH_"
#define ENC_ID "ENCODER_"


using namespace daisy;
using namespace daisysp;


void UpdateLeds();

// Declare a local daisy_petal for hardware access
Daisy16rgb hw;

// Variable for tracking encoder increments since there is it's not a continuous control like the rest.
int32_t enc_tracker;
bool vmode = false;

// This runs at a fixed rate to prepare audio samples
void callback(AudioHandle::InterleavingInputBuffer  in,
              AudioHandle::InterleavingOutputBuffer out,
              size_t                                size)
{
    int32_t inc;
    hw.ProcessDigitalControls();
    
    if(hw.encoder.RisingEdge()){
        enc_tracker = 0;
        vmode = !vmode;
    }
    
    
    // Handle Enc
    inc = hw.encoder.Increment();
    if(inc > 0)
        enc_tracker += 1;
    else if(inc < 0)
        enc_tracker -= 1;
    // Clip
    if(enc_tracker > 15)
        enc_tracker = 15;
    else if(enc_tracker < 0)
        enc_tracker = 0;

    

    // Audio Rate Loop
    for(size_t i = 0; i < size; i += 2)
    {
       
    }
}

int main(void)
{
    
    //uint32_t led_period, now;
    //uint32_t last_led_update;

    // LED Freq = 60Hz
    // USB Freq= 10Hz
    //led_period = 5;
    

    hw.Init();
    hw.SetAudioBlockSize(4);

    //last_led_update  = now = System::GetNow();
    //UpdateLeds();


    //hw.StartAdc();
    hw.StartAudio(callback);


    while(1)
    {
        // Do Stuff InfInitely Here
        //now = System::GetNow();

        // Update LEDs (Vegas Mode)
        //if(now - last_led_update > led_period)
        //{
        //    last_led_update = now;
           //UpdateLeds();

        //}
        //hw.VegasMode();
        if(vmode) {
            hw.VegasMode();
        } else {
            hw.ClearLeds();
            hw.SetRingLed(static_cast<Daisy16rgb::RingLed>(enc_tracker),1,0,0);
            hw.UpdateLeds();
        }
        
       
    }
}


void UpdateLeds()
{
    uint32_t now;
    now = System::GetNow();
    hw.ClearLeds();
    
    // And now the ring
    for(size_t i = 0; i < hw.RING_LED_LAST; i++)
    {
        float    rb, gb, bb;
        uint32_t total, base;
        uint32_t col;
        col = (now >> 10) % 6;
        //        total = 8191;
        //        base  = total / (hw.RING_LED_LAST);
        total        = 1023;
        base         = total / hw.RING_LED_LAST;
        float bright = (float)((now + (i * base)) & total) / (float)total;
        bright       = 1.0f - bright;
        switch(col)
        {
            case 0:
                rb = bright;
                gb = 0.0f;
                bb = 0.0f;
                break;
            case 1:
                rb = 0.6f * bright;
                gb = 0.0f;
                bb = 0.7f * bright;
                break;
            case 2:
                rb = 0.0f;
                gb = bright;
                bb = 0.0f;
                break;
            case 3:
                rb = 0.0f;
                gb = 0.0f;
                bb = bright;
                break;
            case 4:
                rb = 0.75f * bright;
                gb = 0.75f * bright;
                bb = 0.0f;
                break;
            case 5:
                rb = 0.0f;
                bb = 0.85f * bright;
                gb = 0.85f * bright;
                break;

            default: rb = gb = bb = bright; break;
        }

        hw.SetRingLed(static_cast<Daisy16rgb::RingLed>(i), rb, gb, bb);
    }
    hw.UpdateLeds();
}
