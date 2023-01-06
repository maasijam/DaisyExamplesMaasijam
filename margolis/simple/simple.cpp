#include <array>

#include "../daisy_margolis.h"
#include "daisysp.h"


using namespace daisy;
using namespace daisysp;

DaisyMargolis hw;



void Start_Led_Ani();



void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	
    
    for(size_t i = 0; i < size; i++)
    {
        

        

    }

    
}



int main(void)
{
    hw.Init(); 
    

    float samplerate = hw.AudioSampleRate();
       	
    
    
    Start_Led_Ani();

    
    
    hw.StartAdc();
	hw.StartAudio(audio_callback);

	while(1)
	{	
		        
	}
}



void Start_Led_Ani() {
    
    hw.ClearLeds();
    for(size_t i = 0; i < hw.LEDDRIVER_LAST; i++)
    {
        if(i < 8) {
            hw.SetRGBColor(static_cast<DaisyMargolis::LeddriverLeds>(i),hw.red);
        }
        
        hw.UpdateLeds();
        hw.DelayMs(50);
    }

    for(size_t i = 0; i < hw.LEDDRIVER_LAST; i++)
    {
        if(i < 8) {
            hw.SetRGBColor(static_cast<DaisyMargolis::LeddriverLeds>(i),hw.green);
        }
        
        hw.UpdateLeds();
        hw.DelayMs(50);
    }

    for(size_t i = 0; i < hw.LEDDRIVER_LAST; i++)
    {
        if(i < 8) {
            hw.SetRGBColor(static_cast<DaisyMargolis::LeddriverLeds>(i),hw.blue);
        }
        
        hw.UpdateLeds();
        hw.DelayMs(50);
    }

    for(size_t i = 0; i < 3; i++)
    {
        hw.SetGreenLeds(static_cast<DaisyMargolis::LeddriverLeds>(hw.LEDDRIVER_LAST - (i + 1)),1.0f);
                
        hw.UpdateLeds();
        hw.DelayMs(100);
    }
    
}