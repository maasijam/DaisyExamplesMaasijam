#include <array>

#include "../daisy_hank.h"
#include "daisysp.h"


using namespace daisy;
using namespace daisysp;


DaisyHank hw;

static Oscillator osc;
static Metro      clock;

void Update_Digital();
void Start_Led_Ani();
void OledTest();

int ledcount = 0;
int   myenc;
int myknob2;

struct lfoStruct
{
    Oscillator osc;
    Parameter  freqCtrl;
    Parameter  ampCtrl;
    float      amp;
    float      freq;
    int        waveform;
    float      value;

    void Init(float samplerate, AnalogControl freqKnob, AnalogControl ampKnob)
    {
        osc.Init(samplerate);
        osc.SetAmp(1);
        waveform = 0;
        freqCtrl.Init(freqKnob, .1, 35, Parameter::LOGARITHMIC);
        ampCtrl.Init(ampKnob, 0, 1, Parameter::LINEAR);
    }

    void Process(DacHandle::Channel chn)
    {
        //read the knobs and set params
        osc.SetFreq(freqCtrl.Process());
        osc.SetWaveform(waveform);

        //write to the DAC
        hw.seed.dac.WriteValue(
            chn,
            uint16_t((osc.Process() + 1.f) * .5f * ampCtrl.Process() * 4095.f));
    }
};

lfoStruct lfo;

void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	float sig;
    uint8_t tic;
    for(size_t i = 0; i < size; i++)
    {
        sig = osc.Process();

        // left out
        out[0][i] = sig;

        lfo.Process(DacHandle::Channel::ONE);

        tic = clock.Process();
        
        if(tic)
        {
            dsy_gpio_toggle(&hw.gate_out_1);
        } 

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
    osc.SetWaveform(osc.WAVE_SAW);
    osc.SetFreq(600);
    osc.SetAmp(0.75);

    lfo.Init(samplerate, hw.knob[0], hw.knob[1]);

    clock.Init(2, samplerate);
    myenc = 0;


	while(1)
	{	
		 hw.ProcessAllControls();
         Update_Digital(); 
         myenc =  myenc + hw.encoder.Increment();
         myknob2 = static_cast<int>(hw.GetKnobValue(0)*100.0);
         OledTest();
	}
}


void Update_Digital() {

    hw.ClearLeds();
    if(hw.s1.RisingEdge() || hw.gate_in1.Trig()) {
        ledcount++;
        if(ledcount == hw.DIRECT_LEDS_LAST) {
            ledcount = 0;
        }
    }
    if(hw.sw.Read() == hw.sw.POS_UP) {
        hw.SetGreenDirectLeds(DaisyHank::DirectLeds::GREEN_D_LED_9, hw.knob[0].Value());
    }
    if(hw.sw.Read() == hw.sw.POS_DOWN) {
        hw.SetGreenDirectLeds(DaisyHank::DirectLeds::GREEN_D_LED_10, hw.knob[1].Value());
    }
    hw.SetGreenDirectLeds(static_cast<DaisyHank::DirectLeds>(ledcount), 1.0f);

    hw.SetGreenDirectLeds(DaisyHank::DirectLeds::GREEN_D_LED_8,hw.cv[0].Value() > 0.2f ? 1.0f : 0.0f);
    hw.SetGreenDirectLeds(DaisyHank::DirectLeds::GREEN_D_LED_7,hw.cv[1].Value() > 0.2f ? 1.0f : 0.0f);
    hw.UpdateLeds();
}

void Update_Controls() {
    hw.ProcessAllControls();
    
}

void Start_Led_Ani() {
    
    for(size_t i = 0; i < hw.DIRECT_LEDS_LAST; i++)
    {
        hw.SetGreenDirectLeds(static_cast<DaisyHank::DirectLeds>(i), 1.0f);
        hw.UpdateLeds();
        hw.DelayMs(80);
    }
    
}

void OledTest()
{
    
    
    //encodervalue = hw.encoder.Increment();
    
    

    //str = "FREQ:" + std::to_string(static_cast<uint32_t>(osc.GetFreq()));
    
      
    
    //if(hw.seed.system.GetNow() - hw.screen_update_last_ > hw.screen_update_period_)
    //{
        hw.display.Fill(false);
        
        
        

        hw.display.SetCursor(0, 0);
        hw.display.WriteString("HANK", Font_11x18, true);
        hw.display.SetCursor(0, 20);
        hw.display.WriteString("maasijam", Font_11x18, true);
        std::string encodervalue;
        char*       encstr = &encodervalue[0];
        encodervalue =   "Encoder: " + std::to_string(myenc);
        hw.display.SetCursor(0, 40);
        hw.display.WriteString(encstr, Font_7x10, true);
        encodervalue =   "Pot1: " + std::to_string(myknob2);
        hw.display.SetCursor(0, 52);
        hw.display.WriteString(encstr, Font_7x10, true);

        hw.display.Update();
        
    //}
}