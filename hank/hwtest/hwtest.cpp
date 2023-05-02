#include <array>

#include "../daisy_hank.h"
#include "daisysp.h"


using namespace daisy;
using namespace daisysp;

enum Modes {
    NORMAL,
    PRESET,
    CV,
    LAST_MODE
};


DaisyHank hw;

static Oscillator osc;


void Update_Digital();
void Start_Led_Ani();
void Update_Controls();

int ledcount = 0;
bool longPress = false;
bool doubleSwOn = false;
int   myenc;
int myknob2;
int preset = 0;
int cvToCtrl = 0;
bool changePreset = false;
bool firstTimeMode = false;
bool cvMode = false;
int mode = NORMAL;



void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	float sig;
    Update_Controls();
    Update_Digital(); 
   
    for(size_t i = 0; i < size; i++)
    {
        sig = osc.Process();

        // left out
        //out[0][i] = sig;
        //out[1][i] = sig;

        out[0][i] = 0;
        out[1][i] = 0;

        

        
        
        
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
		 
         
         
	}
}


void Update_Digital() {

    hw.ClearLeds();

    switch (mode)
    {
        case NORMAL:
            if (hw.s1.TimeHeldMs() == 1000)
            {
                firstTimeMode = true;   
                mode = PRESET;
                
            }
        
        
            if(hw.s1.DoubleClick()) {
                mode = CV;
            }

            hw.SetLed(DaisyHank::RGB_LED_1, 0,( preset == 0 ? 1.0f : 0),0);
            hw.SetLed(DaisyHank::RGB_LED_2, 0,( preset == 1 ? 1.0f : 0),0);
            hw.SetLed(DaisyHank::RGB_LED_3, 0,( preset == 2 ? 1.0f : 0),0);
            hw.SetLed(DaisyHank::RGB_LED_4, 0,( preset == 3 ? 1.0f : 0),0);

            break;
        case PRESET:
            if(hw.s1.RisingEdge()) {
                firstTimeMode = false;
            }
            
            if(hw.s1.FallingEdge() && !firstTimeMode){
                
                preset++;
                if(preset > 3) 
                    preset = 0;
            }
            if (hw.s1.TimeHeldMs() == 1000)
            {
                firstTimeMode = true;   
                mode = NORMAL;
                
            }
            hw.SetLed(DaisyHank::RGB_LED_1, preset == 0 ? 1 : 0,0,0 );
            hw.SetLed(DaisyHank::RGB_LED_2, preset == 1 ? 1: 0,0,0);
            hw.SetLed(DaisyHank::RGB_LED_3, preset == 2 ? 1 : 0,0,0);
            hw.SetLed(DaisyHank::RGB_LED_4, preset == 3 ? 1 : 0,0,0);
            break;
        case CV:
            if(hw.s1.RisingEdge()) {
                firstTimeMode = false;
            }
            
            if(hw.s1.FallingEdge() && !firstTimeMode){
                
                cvToCtrl++;
                if(cvToCtrl > 3) 
                    cvToCtrl = 0;
            }
            if (hw.s1.TimeHeldMs() == 1000)
            {
                firstTimeMode = true;   
                mode = NORMAL;
                
            }
            hw.SetLed(DaisyHank::RGB_LED_1, 0,0,cvToCtrl == 0 ? 1 : 0 );
            hw.SetLed(DaisyHank::RGB_LED_2, 0,0,cvToCtrl == 1 ? 1 : 0);
            hw.SetLed(DaisyHank::RGB_LED_3, 0,0,cvToCtrl == 2 ? 1 : 0);
            hw.SetLed(DaisyHank::RGB_LED_4, 0,0,cvToCtrl == 3 ? 1 : 0);
            break;
    }
    
        
   
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