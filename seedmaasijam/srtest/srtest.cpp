#include "../daisy_srtest.h"

#define PIN_CD4021_D1 26
#define PIN_CD4021_CS 27
#define PIN_CD4021_CLK 28

using namespace daisy; 

enum
    {
        KEY_1, /**< & */
        KEY_2, /**< & */
        KEY_3, /**< & */
        KEY_4, /**< & */
        KEY_5, /**< & */
        KEY_6, /**< & */
        KEY_7, /**< & */
        KEY_8, /**< & */
        KEY_9, /**< & */
        KEY_10, /**< & */
        KEY_11, /**< & */
        KEY_12, /**< & */
        SW0A, /**< & */
        SW0B, /**< & */
        SW1A, /**< & */
        SW1B, /**< & */
        KEY_SW_LAST    /**< & */
    };



DaisySrtest hw;



void UpdateButtons() {
    
    
    
    
}

void Update_Leds()
{
    hw.ClearLeds();
    hw.SetRingLed(hw.RING_LED_1,!hw.KeyboardState(hw.S1) ? 1 : 0,!hw.KeyboardState(hw.S2) ? 1 : 0,0);
    hw.SetRingLed(hw.RING_LED_2,!hw.KeyboardState(hw.S3) ? 1 : 0,!hw.KeyboardState(hw.S4) ? 1 : 0,0);
    hw.SetRingLed(hw.RING_LED_3,!hw.KeyboardState(hw.S5) ? 1 : 0,!hw.KeyboardState(hw.S7) ? 1 : 0,0);
    hw.SetRingLed(hw.RING_LED_4,!hw.KeyboardState(hw.S6) ? 1 : 0,!hw.KeyboardState(hw.S9) ? 1 : 0,0);
    hw.SetRingLed(hw.RING_LED_5,!hw.KeyboardState(hw.S8) ? 1 : 0,!hw.KeyboardState(hw.S11) ? 1 : 0,0);
    hw.SetRingLed(hw.RING_LED_6,!hw.KeyboardState(hw.S10) ? 1 : 0,0,0);
    hw.SetRingLed(hw.RING_LED_7,!hw.KeyboardState(hw.S12) ? 1 : 0,0,0);
    hw.SetRingLed(hw.RING_LED_8,hw.KeyboardState(hw.S0A) ? 1 : 0,hw.KeyboardState(hw.S0B) ? 1 : 0,0);
    hw.UpdateLeds();
    
}



int main(void)
{
   // Init
    float samplerate;
    hw.Init();

    hw.VegasMode();
    //hw.ClearLeds();

    samplerate = hw.AudioSampleRate();
   

    
    

   
    
    for(;;)
    {
        hw.ProcessDigitalControls();
        UpdateButtons();
        Update_Leds();
        
        
        
    }
}
