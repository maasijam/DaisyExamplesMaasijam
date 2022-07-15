#include <array>

#include "../daisy_white.h"
#include "daisysp.h"




using namespace daisy;
using namespace daisysp;



DaisyWhite hw;


void Update_Leds();
void Update_Buttons();


void audio_callback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	
}


int main(void)
{
	hw.Init();

    //Set up oscillators
	const float sample_rate = hw.AudioSampleRate();
    hw.led_driver.SetAllTo(0.f);
    hw.led_driver.SwapBuffersAndTransmit();

    hw.green_led[0].Init(hw.seed.GetPin(23),true);
    hw.green_led[1].Init(hw.seed.GetPin(22),true);
    hw.green_led[2].Init(hw.seed.GetPin(21),true);
    hw.green_led[3].Init(hw.seed.GetPin(20),true);
	
    hw.StartAdc();
	hw.StartAudio(audio_callback);

    

	while(1)
	{	
		hw.ProcessAllControls();
		Update_Buttons();
        //Update_Leds();

        //wait 1 ms
        System::Delay(1);		
	}
}

void Update_Leds() {
   
    // knob_vals is exactly 8 members
    size_t knob_leds[] = {
        DaisyWhite::LED_0,
        DaisyWhite::LED_1,
        DaisyWhite::LED_2,
        DaisyWhite::LED_3,
        DaisyWhite::LED_4,
        DaisyWhite::LED_5,
        DaisyWhite::LED_6,
        DaisyWhite::LED_7,
        DaisyWhite::LED_8,
        DaisyWhite::LED_9,
        DaisyWhite::LED_10,
        DaisyWhite::LED_11,
    };
    size_t switch_leds[] = {
        DaisyWhite::LED_12,
        DaisyWhite::LED_13,
        DaisyWhite::LED_14,
        DaisyWhite::LED_15,
    };
    for(size_t i = 0; i < 12; i++)
    {
        hw.led_driver.SetLed(knob_leds[i], hw.knob[i].Value());
    }
    for(size_t i = 0; i < 4; i++)
    {
        hw.led_driver.SetLed(switch_leds[i], 1.f);
    }
    hw.led_driver.SwapBuffersAndTransmit();
	
}

void Update_Buttons() {
	if(hw.SwitchState(0)){
        hw.green_led[0].Set(1.f);
    } else {
        hw.green_led[0].Set(0.f);
    }
    if(hw.SwitchState(1)){
        hw.green_led[1].Set(1.f);
    } else {
        hw.green_led[1].Set(0.f);
    }
    if(hw.SwitchState(2)){
        hw.green_led[2].Set(1.f);
    } else {
        hw.green_led[2].Set(0.f);
    }
    if(hw.SwitchState(3)){
        hw.green_led[3].Set(1.f);
    } else {
        hw.green_led[3].Set(0.f);
    }
     hw.green_led[0].Update();
      hw.green_led[1].Update();
       hw.green_led[2].Update();
        hw.green_led[3].Update();
     
}