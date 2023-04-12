#include <array>

#include "../daisy_hank.h"
#include "daisysp.h"


using namespace daisy;
using namespace daisysp;


DaisyHank hw;

static Oscillator osc;


void Update_Digital();
void Start_Led_Ani();


int ledcount = 0;
int   myenc;
int myknob2;



void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	float sig;
   
    for(size_t i = 0; i < size; i++)
    {
        sig = osc.Process();

        // left out
        out[0][i] = sig;
        out[1][i] = sig;

        

        
        
        
    }

    
}



int main(void)
{
    hw.Init(); 

    float samplerate = hw.AudioSampleRate();

            	
    hw.StartAdc();
	hw.StartAudio(audio_callback);

    Start_Led_Ani();

    osc.Init(samplerate);

    // Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(100);
    osc.SetAmp(0.75);

    

	while(1)
	{	
		 hw.ProcessAllControls();
         Update_Digital(); 
         
	}
}


void Update_Digital() {

    hw.ClearLeds();
    if(hw.s1.RisingEdge() || hw.gate_in1.Trig()) {
        ledcount++;
        if(ledcount == hw.RGB_LED_LAST) {
            ledcount = 0;
        }
    }
    
    hw.SetLed(DaisyHank::RGB_LED_1, 1.0f,0,0);

    hw.SetLed(DaisyHank::RGB_LED_2,hw.knob[0].Value() > 0.2f ? 1.0f : 0.0f,0,0);
    hw.SetLed(DaisyHank::RGB_LED_3,hw.knob[1].Value() > 0.2f ? 1.0f : 0.0f,0,0);
    hw.SetLed(DaisyHank::RGB_LED_4,ledcount == 1 ? 1.0f : 0.0f, ledcount == 2 ? 1.0f : 0.0f, ledcount == 3 ? 1.0f : 0.0f);
    hw.UpdateLeds();
}

void Update_Controls() {
    hw.ProcessAllControls();
    
}

void Start_Led_Ani() {
    
    for(size_t i = 0; i < hw.RGB_LED_LAST; i++)
    {
        hw.SetLed(static_cast<DaisyHank::Rgbs>(i), 1.0f,0.6f,0.3f);
        hw.UpdateLeds();
        hw.DelayMs(80);
    }
    
}