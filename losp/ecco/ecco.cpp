#include "../daisy_losp.h"
#include "daisysp.h"
#include "delayline_multitap.h" //modified delayline
#include "taptempo.h"
#include "LEDs.h"
#include "DelayMulti.h"
#include <string>



using namespace daisy;
using namespace daisysp;

DaisyLosp hw;



DelayLineMultiTap<float, static_cast<size_t>(48000 * 36.0f)> DSY_SDRAM_BSS delMemsL;

DelayMulti delayL;

//Tap tempo
Taptempo BaseTempo; 
TempoLED tempoLED_BASE;

float delayTimeL{};
float feedbackL{};
float drywet{0.5};
float encDryWet{25};
bool syncMode{false};
bool ClockInFlag{false};

void Update_DelayTimeL();
void Update_feedbackL();
void Update_drywet();
void Update_DelayBaseTempo();
void Update_BaseTempoLED();
void Update_DelayTempoLEDs();
float HardLimit(float input, float limit);
void Update_Buttons();
void UpdateClock();

// This runs at a fixed rate, to prepare audio samples
void callback(AudioHandle::InputBuffer  in,
                AudioHandle::OutputBuffer out,
                size_t                    size)
{
        hw.ProcessAnalogControls();
       
        Update_DelayTimeL();
        Update_feedbackL();
        Update_DelayTempoLEDs();
        
        // Audio is interleaved stereo by default
        for(size_t i = 0; i < size; i ++)
        {
            UpdateClock();
            Update_DelayBaseTempo();

            float Left_In = in[0][i];
            float Right_In = in[1][i];

            //Get combined output from all delay heads
            float delaySignal_L{delayL.GetOutput()};   
            
            //Update Base Tempo LED
            Update_BaseTempoLED();

            //hard limit
            delaySignal_L = HardLimit(delaySignal_L,AudioLimit);

            static float LNegFB{};

            //ensure we never get inverse feedback
            if(feedbackL < LNegFB) 
                LNegFB = 0.0f;

            float feedbackSignalL{ (feedbackL - LNegFB) * delaySignal_L };

            float combinedL{feedbackSignalL + Left_In};

            delayL.Write( combinedL );

            // floats for wet dry mix
            float mixL;
            

            if(drywet < 0.5f)
            {
                mixL = Left_In + (2.0f * drywet * delaySignal_L);
                
            }
            else if(drywet > 0.5f)
            {
                mixL = ((1 - drywet)* 2.0f * Left_In) + delaySignal_L;
                
            }
            else
            {
                mixL = Left_In + delaySignal_L;
                
            }

            
            out[0][i] = mixL;
            out[1][i] = mixL;
            
        }
}

void InitDelays(float samplerate)
{

    //Init fwd delays
    delMemsL.Init(2);    //2 heads
    

    //point del classes at SDRAM buffers
    delayL.del = &delMemsL; 
    

    delayL.init(hw.seed.GetPin(26),samplerate);
       

}


int main(void)
{
    // Initialize Versio hardware and start audio, ADC
    float samplerate;
    hw.Init();
    samplerate = hw.AudioSampleRate();

    hw.SetLed(0,0.f,0.f,0.f);
    hw.SetLed(1,0.f,0.f,0.f);
    hw.UpdateLeds();

    InitDelays(hw.AudioSampleRate());
    BaseTempo.init(mintap,maxtap,1.25f,1);  //max 6 second tap
    BaseTempo.setTempo(24000.0f);

    //setup tempo indicators
    //tempoLED_BASE.init(hw.GetPin(14),hw.AudioSampleRate());
    tempoLED_BASE.init(hw.seed.GetPin(28),hw.AudioSampleRate());
    tempoLED_BASE.setTempo(BaseTempo.getTapFreq());
    tempoLED_BASE.resetPhase();

    
    hw.StartAudio(callback);
    hw.StartAdc();

    //hw.seed.StartLog(false);
    while(1)
    {
        //hw.seed.PrintLine("drywet: %f", encDryWet);
         Update_Buttons();
         Update_drywet();
    }
}

float HardLimit(float input, float limit)
{
    float returnval{};
    if (input > limit)
        returnval = limit;
    else if(input < (limit * -1.0f))
        returnval = limit * -1.0f;
    else
        returnval = input;

    return returnval;   
}

void Update_DelayBaseTempo()
{
    delayL.SetBaseTempo(BaseTempo.getDelayLength());
}

void Update_DelayTimeL()
{
    static bool lastShift{};
    static bool delayTimeL_pickup{};
    
    
    static float delayTimeL_Last{};

    //update pot values
    float delayTimeL_Pot{hw.GetKnobValue(DaisyLosp::CONTROL_KNOB_TOP)};

    //counter used to limit how quickly delay time is changed, 
    //and to ensure L and R delay times don't change at the same time.
    static int counterL{};
    counterL = (counterL + 1) % (32 * 6);    

    
        static float delayTimeL_new{};
        
            if(!delayTimeL_pickup)  //not picked up
            {
                if(abs(delayTimeL_Pot - delayTimeL_new) > pickupTolerance)  //checked if changed from new value
                {
                    delayTimeL_pickup = true;   //set to picked up
                }
            }
        

        float delayTimeL{};

        if(delayTimeL_pickup)
        {
            delayTimeL = delayTimeL_Pot; //combine pot value and CV
            delayTimeL_Last = delayTimeL_Pot; //update last value
        }

        else
        {
            delayTimeL = delayTimeL_Last; //combine last pot value and CV
        }

        if(counterL == 0)
        {
            if(delayL.SetDelayTime(delayTimeL,syncMode))
            {
            };
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
    delayL.SetBasePhase( dividedPhase );
    
    
}

void Update_feedbackL()
{
    static bool lastShift{};
    static bool feedbackL_pickup{};
    static bool HPCutoff_pickup{};

    static float feedbackL_Last{};

    //get pot values:
    //float feedbackL_Pot{hw.adc.GetFloat(2)};
    float feedbackL_Pot{hw.GetKnobValue(DaisyLosp::CONTROL_KNOB_BOTTOM)};

      
        static float feedbackL_new{};
        //update pickup
        
            if(!feedbackL_pickup)  //not picked up
            {
                if(abs(feedbackL_Pot - feedbackL_new) > pickupTolerance)  //checked if changed from new value
                {
                    feedbackL_pickup = true;   //set to picked up
                }
            }
       

        float feedbackL_combo{};

        if(feedbackL_pickup)
        {
            feedbackL_combo = feedbackL_Pot;
            feedbackL_Last = feedbackL_Pot; //update last value
        }

        else
        {
            feedbackL_combo = feedbackL_Last;
        }
  
        float feedbackL_Target{scale(feedbackL_combo,0.0,maxFB,LINEAR)};  
        fonepole(feedbackL,feedbackL_Target,0.032f);

    
}

void Update_DelayTempoLEDs()
{    
    delayL.updateTempoLED(syncMode);
    
    //DELAYL_DEBUG = delayL.GetDelayTime();
    //DELAYR_DEBUG = delayR.GetDelayTime();
    //CLOCK_DEBUG = BaseTempo.getDelayLength();
}


void Update_Buttons()
{  
    static uint32_t shiftTime{};
    static uint32_t resetTime{};
    static int buttonstate{};

    
    hw.encoder.Debounce();
    
    //Sync.Debounce();
    
    if (hw.encoder.RisingEdge())    
    {
        if(BaseTempo.tap()) //if tempo changed
        {
            tempoLED_BASE.setTempo(BaseTempo.getTapFreq());
            //AltControls.tempo = BaseTempo.getTempo();
            //save_flag = true;
        }
        tempoLED_BASE.resetPhase();
        

        shiftTime = System::GetNow();   //reset shift timer
        buttonstate += 1;
        
        if (buttonstate == 3)
        {
            resetTime = System::GetNow();
        }

    }

    if (hw.encoder.FallingEdge())    //when button is let go shift is off
    {
        //shift = false;
        buttonstate -= 1;
        //mute = false;
    }

    

    syncMode = hw.sw[hw.SW_RIGHT].Read() == 2 ? true : false;
    //syncMode = Sync.Pressed() ? true : false;
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
                //AltControls.tempo = BaseTempo.getTempo();
                //save_flag = true;
            }
            ClockCounter = 0; //reset counter

    } 
}

void Update_drywet()
{
    

   

    float encInc = hw.encoder.Increment();
    encDryWet += encInc;

    if(encDryWet < 0) {
        encDryWet = 0;
    }
    if(encDryWet > 50) {
        encDryWet = 50;
    }        
        float drywetcombo{};

        drywetcombo = encDryWet / 50;

        float drywetTarget{};

        drywetTarget = scale(drywetcombo,0.0,1.0,LINEAR); 
        

        //fonepole(drywet,drywetTarget,0.032f); 
        fonepole(drywet,drywetTarget,0.016f); 
   
    
}