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
DelayLineMultiTap<float, static_cast<size_t>(48000 * 36.0f)> DSY_SDRAM_BSS delMemsR;
DelayLineMultiTap<float, static_cast<size_t>(48000 * 36.0f)> DSY_SDRAM_BSS delMemsC;
DelayLineMultiTap<float, static_cast<size_t>(48000 * 36.0f)> DSY_SDRAM_BSS delMemsC2;

DelayMulti delayL, delayR, delayC, delayC2;

//Tap tempo
Taptempo BaseTempo; 
TempoLED tempoLED_BASE;

PitchShifter ps,ps2;

float delayTime{};
float feedback{};
float drywet{0.5};
float encDryWet{25};
bool syncMode{false};
bool ClockInFlag{false};

void Update_DelayTime();
void Update_feedback();
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
       
        Update_DelayTime();
        Update_feedback();
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
            float delaySignal_R{delayR.GetOutput()};
            float delaySignal_C{delayC.GetOutput()};
            float delaySignal_C2{delayC2.GetOutput()};
            
            //Update Base Tempo LED
            Update_BaseTempoLED();

            //hard limit
            delaySignal_L = HardLimit(delaySignal_L,AudioLimit);
            delaySignal_R = HardLimit(delaySignal_R,AudioLimit);
            delaySignal_C = HardLimit(delaySignal_C,AudioLimit);
            delaySignal_C2 = HardLimit(delaySignal_C2,AudioLimit);

            static float NegFB{};

            //ensure we never get inverse feedback
            if(feedback < NegFB) 
                NegFB = 0.0f;

            float feedbackSignalL{ (feedback - NegFB) * delaySignal_L };
            float combinedL{feedbackSignalL + Left_In};
            delayL.Write( combinedL );

            float feedbackSignalR{ (feedback - NegFB) * delaySignal_R };
            float combinedR{feedbackSignalR + Right_In};
            delayR.Write( combinedR );

            float feedbackSignalC{ (feedback - NegFB) * delaySignal_C };
            float combinedC{feedbackSignalC + Left_In};
            delayC.Write( combinedC );

            float feedbackSignalC2{ (feedback - NegFB) * delaySignal_C2 };
            float combinedC2{feedbackSignalC2 + Right_In};
            delayC2.Write( combinedC2 );

            // floats for wet dry mix
            float mixL;
            float mixR;

            float shifted = ps.Process(combinedC);
             shifted *= 0.2;

            float shifted2 = ps2.Process(combinedC2);
             shifted2 *= 0.15;

            if(drywet < 0.5f)
            {
                mixL = Left_In + (2.0f * drywet * (delaySignal_L + shifted));
                mixR = Right_In + (2.0f * drywet * (delaySignal_R + shifted));
                //mixL = Left_In + (2.0f * drywet * (delaySignal_L));
                //mixR = Right_In + (2.0f * drywet * (delaySignal_R));
            }
            else if(drywet > 0.5f)
            {
                mixL = ((1 - drywet)* 2.0f * Left_In) + delaySignal_L + shifted;
                mixR = ((1 - drywet)* 2.0f * Right_In) + delaySignal_R + shifted;
                //mixL = ((1 - drywet)* 2.0f * Left_In) + delaySignal_L;
                //mixR = ((1 - drywet)* 2.0f * Right_In) + delaySignal_R;
                
            }
            else
            {
                mixL = Left_In + delaySignal_L + shifted;
                mixR = Right_In + delaySignal_R + shifted;
                //mixL = Left_In + delaySignal_L;
                //mixR = Right_In + delaySignal_R;
                
            }

            
            out[0][i] = mixL;
            out[1][i] = mixR;
            
        }
}

void InitDelays(float samplerate)
{

    //Init fwd delays
    delMemsL.Init(2);    //2 heads
    delMemsR.Init(2);    //2 heads
    delMemsC.Init(2);    //2 heads
    delMemsC2.Init(2);    //2 heads

    //point del classes at SDRAM buffers
    delayL.del = &delMemsL; 
    delayR.del = &delMemsR; 
    delayC.del = &delMemsC; 
    delayC2.del = &delMemsC2; 
    

    delayL.init(hw.seed.GetPin(26),samplerate);
    delayR.init(hw.seed.GetPin(7),samplerate);
    delayC.init(hw.seed.GetPin(7),samplerate);
    delayC2.init(hw.seed.GetPin(7),samplerate);
       

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

    ps.Init(samplerate);
    ps.SetTransposition(12.0f);
    ps2.Init(samplerate);
    ps2.SetTransposition(24.0f);

    
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
    delayR.SetBaseTempo(BaseTempo.getDelayLength());
    delayC.SetBaseTempo(BaseTempo.getDelayLength());
    delayC2.SetBaseTempo(BaseTempo.getDelayLength());
}

void Update_DelayTime()
{
    static bool lastShift{};
    static bool delayTime_pickup{};
    
    
    static float delayTime_Last{};

    //update pot values
    float delayTime_Pot{hw.GetKnobValue(DaisyLosp::CONTROL_KNOB_TOP)};

    //counter used to limit how quickly delay time is changed, 
    //and to ensure L and R delay times don't change at the same time.
    static int counter{};
    counter = (counter + 1) % (32 * 6);    

    
        static float delayTime_new{};
        
            if(!delayTime_pickup)  //not picked up
            {
                if(abs(delayTime_Pot - delayTime_new) > pickupTolerance)  //checked if changed from new value
                {
                    delayTime_pickup = true;   //set to picked up
                }
            }
        

        float delayTime{};

        if(delayTime_pickup)
        {
            delayTime = delayTime_Pot; //combine pot value and CV
            delayTime_Last = delayTime_Pot; //update last value
        }

        else
        {
            delayTime = delayTime_Last; //combine last pot value and CV
        }

        if(counter == 0)
        {
            if(delayL.SetDelayTime(delayTime,syncMode))
            {
            };
            if(delayR.SetDelayTime((delayTime * 0.25),syncMode))
            {
            };
            if(delayC.SetDelayTime((delayTime * 0.1),syncMode))
            {
            };
            if(delayC2.SetDelayTime((delayTime),syncMode))
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

void Update_feedback()
{
    static bool lastShift{};
    static bool feedback_pickup{};
    
    static float feedback_Last{};

    //get pot values:
    //float feedbackL_Pot{hw.adc.GetFloat(2)};
    float feedback_Pot{hw.GetKnobValue(DaisyLosp::CONTROL_KNOB_BOTTOM)};

      
        static float feedback_new{};
        //update pickup
        
            if(!feedback_pickup)  //not picked up
            {
                if(abs(feedback_Pot - feedback_new) > pickupTolerance)  //checked if changed from new value
                {
                    feedback_pickup = true;   //set to picked up
                }
            }
       

        float feedback_combo{};

        if(feedback_pickup)
        {
            feedback_combo = feedback_Pot;
            feedback_Last = feedback_Pot; //update last value
        }

        else
        {
            feedback_combo = feedback_Last;
        }
  
        float feedback_Target{scale(feedback_combo,0.0,maxFB,LINEAR)};  
        fonepole(feedback,feedback_Target,0.032f);

    
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