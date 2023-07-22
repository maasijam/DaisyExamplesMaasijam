
#include "../daisy_saul.h"
#include "daisysp.h"

//#include "sr_595.h"
//#include "LedMatrix.h"
#include <math.h>
#include <string.h>
#include <atomic>




using namespace daisy;
using namespace daisysp;
using namespace saul;

//#define PIN_HC595_D1 14
//#define PIN_HC595_CS 12
//#define PIN_HC595_CLK 11



DaisySaul hw;

void UpdateLeds();

//srtest::ShiftRegister595 sr; 
 

//LedMatrixx ledmatrix;

void AudioCallback(AudioHandle::InterleavingInputBuffer in, AudioHandle::InterleavingOutputBuffer out, size_t size)
{   
    for(size_t i = 0; i < size; i += 2)
    {

       
    }
}



int main(void)
{
    //float samplerate;
    
    hw.Init();
    //hw.SetAudioBlockSize(4);
    //samplerate = hw.AudioSampleRate();
    //hw.SetAudioBlockSize(1);     //set blocksize.
    //dsy_gpio_pin allleds_cfg[3];
    //allleds_cfg[0]    = hw.seed.GetPin(PIN_HC595_CS);
    //allleds_cfg[1]    = hw.seed.GetPin(PIN_HC595_CLK);
    //allleds_cfg[2]  = hw.seed.GetPin(PIN_HC595_D1);
    //sr.Init(allleds_cfg,2);
    
    
    //ledmatrix.init(&sr);
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);


    while (1) {
    
        hw.ProcessAnalogControls(); // Normalize CV inputs
        //hw.UpdateExample();
        //ledmatrix.setLed(3,2,16,1);
        //sr.SetMatrix(18,true);

        //ledmatrix.process();
        UpdateLeds();
        
    }
}

int anode_decimal[8]={1, 2, 4, 8, 16, 32, 64, 128};
int cathode_decimal[8]={254, 253, 251, 247, 239, 223, 191, 127};

void UpdateLeds() {
    
    for (size_t j = 0; j < 8; j++)
    {
        
      hw.SetLed(j,true);
      /************To increase the ON time of LEDs five times more than 
      OFF time to increase the brightness of LEDs*************/
      
      for (size_t i = 0; i < 6; i++)
      {
        
        /*************************  TURN ON DIAGONAL LEDs ONLY  ***************************/  
   
        // take the latchPin low so the LEDs don't change while you're sending in bits:  
        hw.SetLed(i,false);
        
      }   
    
      
   }
}
    
