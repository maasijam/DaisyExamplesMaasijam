////// Rhythm Delay v2.1 - 20220206 - Written by Sonic Explorer //////
#include "daisysp.h"
#include "../daisy_vertigo.h"
#include "taptempo.h"

#include <string>
#include <cmath>

// 48kHz sample rate (48,000 samples per second)
//   a max delay of 1 second would be 48000*1.f, 2 seconds would be 48000*2.f, etc.
#define MAX_DELAY static_cast<size_t>(48000 * 3.f)

using namespace daisy;
using namespace daisysp;

DaisyVertigo vertigo;

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delayMems[4];
//DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS swellMems[4];

//Tap tempo
Taptempo BaseTempo;



constexpr int mintap{20000};    //in us (0.02s)
constexpr int maxtap{6000000};  //in us (6s)

//Switch switch_r,switch_l;

static Tone toneLP_L,toneLP_R; // this is for a Tone object, Tone is just a low-pass filter
static ATone toneHP_L,toneHP_R; // this is for a ATone object, ATone is just a high-pass filter
static Balance bal; // this is for a Balance object, which will correct for volume drop from the filters
static CrossFade cfade_L,cfade_R,widthXfade,revMixL,revMixR; //this is for a CrossFade object, which we will to blend the wet/dry and maintain a constant volume
static Oscillator osc,osc2; //These are for the 'age' or modulation on the delays
static Oscillator osc_delay[4]; //This is for LED2 to show the delay time
//static Compressor compL,compR;
static ReverbSc   DSY_SDRAM_BSS verb;
static DcBlock    blk[2];
static PitchShifter DSY_SDRAM_BSS ps,ps1;



// Declare some global variable so all functions can see them
float feedback = 0;
float maxFeedback = 1.2f; // Max value of feedback knob, maxFeedback=1 -> forever repeats but no selfoscillation, values over 1 allow runaway feedback fun
/////////// Change the secondaryFeedback to 0.95f if you want a 'Sound on Sound' feel, try setting it to 0.0f if you want to use it as a kind of erase button to minimize repeats, I like it at 1.5f for a runaway feedback when pressed.
float secondaryFeedback = 0.95f; // This is what the feedback will go to when the right/second footswitch is pressed
bool feedbackSecondary = false;
float drywet_ratio = 0.5f; // drywet_ratio=0.0 is effect off
float tone_val =0.0f, swell_val=0.0f;
float samplerate, osc_delay_val[4]={0,0,0,0};
float mod_osc,mod_osc2;
float width_val =0.0f;

float dadj = 0.046;

static float      drylevel;


float rand_now=0,rand_val=0;
bool R_ON,L_ON;

int swLeftState = 3;
int swRightState = 3;

struct Delay
{
    DelayLine<float, MAX_DELAY> *delay;
    float                        currentDelay;
    float                        delayTarget;
    float                        feedback;

    float Process(float in)
    {
        //set delay times
        fonepole(currentDelay, delayTarget, .0002f); // This smoothes out the delay when you turn the delay control?
        delay->SetDelay(currentDelay);
        float read = delay->Read();
        delay->Write((feedback * read) + in);
        return read;
    }
};

Delay delays[4];// This creates a delay structure to store delay parameters
//Delay swells[4];// This creates a delay structure for the 'swell' delays

// All these Params will be controlled by Pots

Parameter feedbackParam;
Parameter mixParam;
Parameter toneParam;
//Parameter swellParam;
Parameter ageParam;
Parameter stereoWidthParam;
Parameter verbMixParam;

// Each delay head will be turned on/off independently
bool delayOn[4];

// Use the LED object
//Led led1, led2;

bool  passThruOn; // When this is true 'bypass' is on

void ProcessControls(int part);//this has to be declared above the AudioCallback so it knows it is a function since it is defined below the AudioCallback.
void UpdateSwitches();
bool DelaySwCheck(int idx);
void UpdateClock();

int call_counter = 0;//This counts how many times audiocallback has been ran.
int process_counter = 0;//this keeps track of how many times the same part of procescontrol is ran. 
static void AudioCallback(AudioHandle::InputBuffer in,
			  AudioHandle::OutputBuffer out,
			  //AudioHandle::InterleavingInputBuffer  in,
                          //AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
                          //float **in, float **out, size_t size)
{
    call_counter+=1;
    int n_cases = 4;//the total amount of 'parts' ProcessControl is split into
    const int m_val = 2; //this is how many audiocallbacks between case calls
    int n_calls = n_cases*m_val;

    switch (call_counter%(n_calls))
	{//ProcessControls makes sure all the moveable parameters are up to date (knobs, switches, leds, etc)
	    // ProcessControls is split into 4 sub-parts to spread out the processing over different AudioCallbacks runs
	    //  I do not know if it was necessary to split ProcessControls into 4, but I got some artifacts when calling the full ProcessControls every callback
	case 0*m_val:
	    ProcessControls(1);
	    process_counter+=1;
	    break;
	case 1*m_val: //ProcessControls(2);
	    break;
	case 2*m_val: //ProcessControls(3);
	    break;
	case 3*m_val: ProcessControls(4);
	    break;
	}

    
    
	
    for(size_t i = 0; i < size; i++)
	{
	    UpdateClock();
        
        
        //float final_mix = 0;
        float final_mix_L = 0;
        float final_mix_R = 0;
        
	    //float all_delay_signals = 0;
        float all_delay_signals_L = 0;
        float all_delay_signals_R = 0;

        //float  all_delay_signals_L_W = 0;
        //float  all_delay_signals_R_W = 0;
	    //float swell_signals = 0;
        //float swell_signals_L = 0;
        //float swell_signals_R = 0;
	    //float pre_filter_delay_signals = 0;
        float pre_filter_delay_signals_L = 0;
        float pre_filter_delay_signals_R = 0;

        float wetL, wetR;
        float delSig;

        for(int c = 0; c < 4; c++){
	        osc_delay_val[c] = osc_delay[c].Process();
        }
	    if(passThruOn){ // This if statement makes the Daisy only process the input through the delays when the pedal is turned 'on'
		    // Do below if the pedal is in 'bypass' mode
		    out[0][i] = in[0][i]; // in[0][i] is the input audio, out[0][i] is the output audio
		    out[1][i] = in[0][i]; // Something is odd with the audio_Out channels on the the Seedv1.1, so we send the audio in to both audio outs. 
		    // The for statment below takes the feedback to 0 when the pedal is off, which *almost* clears the repeats, but not if the pedal is turned off and on again quickly. 
		    for(int d = 0; d < 4; d++){
		        delays[d].feedback = 0;
		        delays[d].Process(0);
		        //swells[d].feedback = feedback;
		        //swells[d].Process(0);
		    }
	    }
	    else{
		    // calculate the oscillators for the Age control
		    mod_osc = osc.Process();//This is the base sine wave
		    mod_osc2 = osc2.Process();//This is the square wave for glitches

            

		    // create the delayed signal for each of the 4 delays
		    for(int d = 0; d < 4; d++)
		        if(delayOn[d]){
			        delays[d].feedback = feedback;
                    delSig = delays[d].Process(in[0][i]);
                    
                           
                    if((d == 0 || d == 3) && vertigo.sw[0].Read() == 1) {
                        all_delay_signals_L += delSig;
                    } else if((d == 0 || d == 2) && vertigo.sw[0].Read() == 0) {
                        all_delay_signals_L += delSig;
                    } else if((d == 0 || d == 1) && vertigo.sw[0].Read() == 2) {
                        all_delay_signals_L += delSig;
                    } else if((d == 1 || d == 2) && vertigo.sw[0].Read() == 1) {
                        all_delay_signals_R += delSig;
                    } else if((d == 1 || d == 3) && vertigo.sw[0].Read() == 0) {
                        all_delay_signals_R += delSig;
                    } else if((d == 2 || d == 3) && vertigo.sw[0].Read() == 2) {
                        all_delay_signals_R += delSig;
                    }
                    // Up: Left:1|4, Right:2|3
                    // Center: Left:1|3, Right:2|4
                    // Down: Left:1|2, Right:3|4
			
		        }
		// sends the delays signal through 4 delay heads that are always on for a faux reverb 'swell' effect
        /*
		for(int d = 0; d < 4; d++){
		    if(feedback<0.2f) /// This will make the swells adhere to the feedback knob at low feedback values
			swells[d].feedback = feedback;
		    else
			swells[d].feedback = 0.2f;
		    if(d==0) {
			swell_signals_L += swells[d].Process(all_delay_signals_L)*0.5;
            swell_signals_R += swells[d].Process(all_delay_signals_R)*0.5;
            } else {
			swell_signals_L += swells[d].Process(swell_signals_L)*0.5;
            swell_signals_R += swells[d].Process(swell_signals_R)*0.5;
            }
		}
        */
		//// Apply a bit of compression to the swells -- there is lots of room for the compression settings to be made better
		//swell_signals_L = compL.Process(swell_signals_L);
        //swell_signals_R = compR.Process(swell_signals_R);
		//swell_signals = comp.Process(swell_signals,all_delay_signals);//maybe sidechain this to the all_delay_singals? - I tried this but it sounded less natural to me. 

		// Add the swells to the normal delay head signals
		//all_delay_signals_L += swell_signals_L*(swell_val*swell_val)/2;//two factor of swell_val and the 1/2 here just seemed to feel right for the amount
		//all_delay_signals_R += swell_signals_R*(swell_val*swell_val)/2;
		// Save the pre-filtered delay signals as a reference volume
        //float wp = 0;
       
        
        float all_delay_signals_L_W = widthXfade.Process(all_delay_signals_L,all_delay_signals_R);    //mix to mono if width 0.0
        float all_delay_signals_R_W = widthXfade.Process(all_delay_signals_R,all_delay_signals_L);
        //all_delay_signals_L_W = all_delay_signals_L;
        //all_delay_signals_R_W = all_delay_signals_R;

        //float shiftedl = ps.Process(all_delay_signals_L_W);
        //float shiftedr = ps1.Process(all_delay_signals_R_W);

        //all_delay_signals_L_W = all_delay_signals_L_W + (shiftedl*0.5);
        //all_delay_signals_R_W = all_delay_signals_R_W + (shiftedr*0.5);


		pre_filter_delay_signals_L = all_delay_signals_L_W;
        pre_filter_delay_signals_R = all_delay_signals_R_W;

		// Filter the delay signals with a LP and HP based on the tone knob
		all_delay_signals_L_W = toneHP_L.Process(all_delay_signals_L_W);
		all_delay_signals_L_W = toneLP_L.Process(all_delay_signals_L_W);
		// This 'balances' (I think compresses) the pre & post filter delayed signal to make up for volume loss due to the filter
		all_delay_signals_L_W = bal.Process(all_delay_signals_L_W, pre_filter_delay_signals_L*(1.f-tone_val/1.5f));

        all_delay_signals_R_W = toneHP_R.Process(all_delay_signals_R_W);
		all_delay_signals_R_W = toneLP_R.Process(all_delay_signals_R_W);
		// This 'balances' (I think compresses) the pre & post filter delayed signal to make up for volume loss due to the filter
		all_delay_signals_R_W = bal.Process(all_delay_signals_R_W, pre_filter_delay_signals_R*(1.f-tone_val/1.5f));
		//** at extreme HP levels this introduces noise for the lower notes since it has to raise the volume a lot to make up for the low frew loss
		//     I added the tone_val factor here to account for this noise a bit and to lower the repeat volume at the extremes of the tone pot

        
		
		// Use a crossfade object to maintain a constant power while creating the delayed/raw audio mix
		cfade_L.SetPos(drywet_ratio);
        cfade_R.SetPos(drywet_ratio);
		//final_mix = cfade.Process(in[i], all_delay_signals);
		float orig = in[0][i]; // I don't exactly know why I have to do this, but something with the pointing is not behaving with cfade
		final_mix_L = cfade_L.Process(orig, all_delay_signals_L_W);
        final_mix_R = cfade_R.Process(orig, all_delay_signals_R_W);

        //if(vertigo.sw[1].Read() == 2) {

        revMixL.SetPos(drylevel);
        revMixR.SetPos(drylevel);
        verb.Process(final_mix_L*0.5f ,final_mix_R*0.5f, &wetL, &wetR);
        // Dc Block
        wetL = blk[0].Process(wetL);
        wetR = blk[1].Process(wetR);
        out[0][i] = revMixL.Process(final_mix_L,wetL); // this sends 'final_mix' to the (left) audio
		out[1][i] = revMixR.Process(final_mix_R,wetR); // Something is odd with the audio_Out channels on the the Seedv1.1, so we send the effected audio to both audio outs. 
        //} else {
        //    out[0][i] = blk[0].Process(final_mix_L); // this sends 'final_mix' to the (left) audio
		//    out[1][i] = blk[1].Process(final_mix_R); // Something is odd with the audio_Out channels on the the Seedv1.1, so we send the effected audio to both audio outs. 
        //}

		
	    }
	    
	}
}

void InitDelays(float samplerate)
{ //Initialize all the delays & swells
    for(int i = 0; i < 4; i++)
    { 
        delayMems[i].Init();
        delays[i].delay = &delayMems[i];
	    delays[i].feedback = 0;
	    //swellMems[i].Init();
        //swells[i].delay = &swellMems[i];
	    //swells[i].feedback = 0; 
       //delay times: - just one knob controls all delay times
       //    (delay times for 1-3 are set as fractions of this knob value in ProcessControls below)
	
    }
    //delayParam.Init(petal.knob[Terrarium::KNOB_1], samplerate * .1, MAX_DELAY * 1.0,Parameter::LINEAR);
    //delayParam.Init(vertigo.knobs[DaisyVertigo::KNOB_0], -1, 1.0,Parameter::LINEAR);
    mixParam.Init(vertigo.knobs[DaisyVertigo::KNOB_0], 0.0, 1.0, Parameter::LINEAR);  // mix is knob 2
    feedbackParam.Init(vertigo.knobs[DaisyVertigo::KNOB_4], 0.0, maxFeedback, Parameter::LINEAR);  // feedback is knob 3
}

void InitSwell(float samplerate)
{ // Initilize the compression and knob for the swell
    //swellParam.Init(vertigo.knobs[DaisyVertigo::KNOB_1], 0.0, 1, Parameter::LINEAR);  // swell is knob 6
    //compL.Init(samplerate);
    //compL.SetThreshold(-40.0f);
    //compL.SetRatio(10.0f);
    //compL.SetAttack(0.5f);
    //compL.SetRelease(0.5f);

    //compR.Init(samplerate);
    //compR.SetThreshold(-40.0f);
    //compR.SetRatio(10.0f);
    //compR.SetAttack(0.5f);
    //compR.SetRelease(0.5f);
}

void InitAge(float samplerate)
{ // Initilize all the things for the Age control
    ageParam.Init(vertigo.knobs[DaisyVertigo::KNOB_5], 0.0, 1, Parameter::LINEAR);//LOGARITHMIC);//LINEAR);  // age is knob 5
    osc.Init(samplerate);
    osc.SetWaveform(osc.WAVE_SIN);//TRI);
    osc.SetFreq(2.0f);
    osc.SetAmp(1.0f);
    osc2.Init(samplerate);
    osc2.SetWaveform(osc2.WAVE_SQUARE);    //osc2.SetWaveform(osc.WAVE_SAW);
    osc2.SetFreq(2.f);
    osc2.SetAmp(1.0f);
}

void InitTone(float samplerate)
{   // Initialize the Tone object
    toneHP_L.Init(samplerate);
    toneLP_L.Init(samplerate);
    toneHP_R.Init(samplerate);
    toneLP_R.Init(samplerate);
    toneParam.Init(vertigo.knobs[DaisyVertigo::KNOB_2], -1.0f, 1.0f, Parameter::LINEAR); // This knob value will be converted later in ProcessControls to a frequency for the High/Low Pass filter
}

void InitWaveOut()
{    // This uses the default pins of 29 & 30 for the DAC (GPIO22 & GPIO23) to output the waveforms
    //   These are the same pins as the Terrarium leds...so you cannot use both at the same time.
    DacHandle::Config cfg;
    cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    cfg.buff_state = DacHandle::BufferState::ENABLED;
    cfg.mode       = DacHandle::Mode::POLLING;
    cfg.chn        = DacHandle::Channel::BOTH;
    vertigo.seed.dac.Init(cfg);
}

void InitVerb(float samplerate) {
    verb.Init(samplerate);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(18000.0f);
    verbMixParam.Init(vertigo.knobs[DaisyVertigo::KNOB_6], 0.0f, 1.0f, Parameter::LINEAR);
    blk[0].Init(samplerate);
    blk[1].Init(samplerate);
}

void InitPs(float samplerate) {
    ps.Init(samplerate);
    ps.SetTransposition(21.0f);
    ps1.Init(samplerate);
    ps1.SetTransposition(24.0f);
    
}



void InitLeds(void)
{
    //Initialize the leds - these are using LED objects
    //led1.Init(petal.seed.GetPin(Terrarium::LED_1),false);
    //led2.Init(petal.seed.GetPin(Terrarium::LED_2),false);
    // The 'Terrarium::LED_1' (and similar for the knobs) references the terrarium.h which defines which GPIO pins
    //     are associated with which knobs, switches, & LEDs
    // The oscilator is being initiated here because it shows the delay time. 
    for (size_t i = 0; i < 4; i++)
    {
        osc_delay[i].Init(samplerate);
        osc_delay[i].SetWaveform(osc.WAVE_SIN);
        osc_delay[i].SetFreq(2.0f);
        osc_delay[i].SetAmp(1.0f);
    }
    
    
}

void InitExSwitches(void)// Setup the extra top switches
{/// I have to do this to use the extra switches I added to the top of my Terrarium build
    //switch_l.Init(petal.seed.GetPin(27),petal.seed.AudioCallbackRate(),Switch::TYPE_TOGGLE,Switch::POLARITY_NORMAL,Switch::PULL_UP);
    //switch_r.Init(petal.seed.GetPin(28),petal.seed.AudioCallbackRate(),Switch::TYPE_TOGGLE,Switch::POLARITY_NORMAL,Switch::PULL_UP);
    // a reminder you have to use switch_l.Debounch() & switch_r.Debounch() for the extra switches to actually work
}

float get_rand(void)//returns a random value between 0 and 1.0
{
    return rand()/(float)RAND_MAX -0.5;
}

int main(void)
{
    vertigo.Init(); // Initialize hardware (daisy seed, and petal)
    samplerate = vertigo.AudioSampleRate();
    vertigo.SetAudioBlockSize(1);////////Adjust the blocksize 

    InitLeds();
    
    // Initialize the delay lines
    InitDelays(samplerate);
    
    // Initialize the Tone object
    InitTone(samplerate);
    
    // Initializes the Balance object
    bal.Init(samplerate);
    
    // Initializes the age functions/params
    InitAge(samplerate);

    // Initializes the swell functions/params
    InitSwell(samplerate);

    InitVerb(samplerate);

    InitPs(samplerate);
    
    
    stereoWidthParam.Init(vertigo.knobs[DaisyVertigo::KNOB_3], 0.5f, 0.0f, Parameter::LINEAR);//LOGARITHMIC);//LINEAR);  
    widthXfade.Init();
    widthXfade.SetCurve(CROSSFADE_CPOW);
    //widthXfade.SetPos(0.f);

    // Initialize & set params for CrossFade object
    cfade_L.Init();
    cfade_L.SetCurve(CROSSFADE_CPOW);
    cfade_R.Init();
    cfade_R.SetCurve(CROSSFADE_CPOW);

    revMixL.Init();
    revMixL.SetCurve(CROSSFADE_CPOW);
    revMixR.Init();
    revMixR.SetCurve(CROSSFADE_CPOW);
    //This sets to crossfade to maintain constant power, which will maintain a constant volume as we go from full dry to full wet on the mix knob

    //This is an example of what is required to add extra switches
    if(false)
	InitExSwitches();
//WidthXfade.SetPos(0.0f);
    BaseTempo.init(mintap,maxtap,1.25f,1);  //max 6 second tap
    
    passThruOn = false;// This starts the pedal in the 'off' (or delay bypassed) position

    vertigo.StartAdc();
    vertigo.StartAudio(AudioCallback);

    //vertigo.seed.StartLog(false);

    while(1)
    {
	//   dsy_system_delay(6);
        float width = stereoWidthParam.Process();
        widthXfade.SetPos(width);

        ProcessControls(2);
        ProcessControls(3);
        ProcessControls(4);
        
        vertigo.UpdateLeds();

        UpdateSwitches();
	    for(int i=0; i<4; i++)
	        delayOn[i] = DelaySwCheck(i);
	
            vertigo.sw_led[DaisyVertigo::S_Led1].Set(delayOn[0] ? 1.f : 0.f);
            vertigo.sw_led[DaisyVertigo::S_Led2].Set(delayOn[1] ? 1.f : 0.f);
            vertigo.sw_led[DaisyVertigo::S_Led4].Set(delayOn[2] ? 1.f : 0.f);
            vertigo.sw_led[DaisyVertigo::S_Led3].Set(delayOn[3] ? 1.f : 0.f);
        }
}

void ProcessControls(int part)
{

    if(part==1){
	    vertigo.ProcessAnalogControls();
	   
	
	//Bring the feedback to be equal to the knob value, if FS_2 was not pressed this will aleady be the case
	fonepole(feedback, feedbackParam.Process(), 0.001f); //decrease the number for a faster ramp & vice-versa
 
    }

    if(part==2){
	    /////////////////////////////////////////
	    //knobs
	    float mod_total;
	    float time_long;
	    float age_val = ageParam.Process();

    
	    time_long = BaseTempo.getDelayLength();
        //time_long += 100.f;
    
        //time_long = delayParam.Process();//this is just the knob value from -1 to 1.
	    //time_long = MAX_DELAY*(0.1+0.9*(powf(time_long,3)+1)/2);//this now converts the knob value to the delay time in samples, 1 sample is 1/samplerate seconds.
	    // The funny function with the powf and 0.1+0.9*(etc) is to get a knob taper where it starts at 0.1*MAX_DELAY, goes to 1*MAX_DELAY and spends most of the knobs rotation on the middle delay values. 

	    osc_delay[0].SetFreq(samplerate/time_long);// this is set to the delaytime in Hz of the longest delay
	    //led2.Set(osc_delay_val>0.9); //led2 is set to show the delay time of head 4, the longest delay. ---- comment out this line if you want to turn off the delay time indicator
        if(osc_delay_val[0]>0.9 && delayOn[3]) {
            vertigo.SetLed(DaisyVertigo::LED_3,0.f,0.f,0.6f);
        } else {
            vertigo.SetLed(DaisyVertigo::LED_3,0.f,0.f,0.f);
        }
    
        osc_delay[1].SetFreq(samplerate/(time_long*0.75f));// this is set to the delaytime in Hz of the longest * 0.75 delay
	    //led2.Set(osc_delay_val>0.9); //led2 is set to show the delay time of head 4, the longest delay. ---- comment out this line if you want to turn off the delay time indicator
        if(osc_delay_val[1]>0.9 && delayOn[2]) {
            vertigo.SetLed(DaisyVertigo::LED_2,0.f,0.f,0.6f);
        } else {
            vertigo.SetLed(DaisyVertigo::LED_2,0.f,0.f,0.f);
        }

        osc_delay[2].SetFreq(samplerate/(time_long*0.50f));// this is set to the delaytime in Hz of the longest * 0.50 delay
	    //led2.Set(osc_delay_val>0.9); //led2 is set to show the delay time of head 4, the longest delay. ---- comment out this line if you want to turn off the delay time indicator
        if(osc_delay_val[2]>0.9 && delayOn[1]) {
            vertigo.SetLed(DaisyVertigo::LED_1,0.f,0.f,0.6f);
        } else {
            vertigo.SetLed(DaisyVertigo::LED_1,0.f,0.f,0.f);
        }

        osc_delay[3].SetFreq(samplerate/(time_long*0.25f));// this is set to the delaytime in Hz of the longest * 0.25 delay
	    //led2.Set(osc_delay_val>0.9); //led2 is set to show the delay time of head 4, the longest delay. ---- comment out this line if you want to turn off the delay time indicator
        if(osc_delay_val[3]>0.9 && delayOn[0]) {
            vertigo.SetLed(DaisyVertigo::LED_0,0.f,0.f,0.6f);
        } else {
            vertigo.SetLed(DaisyVertigo::LED_0,0.f,0.f,0.f);
        }
	
	    /////// This next section all builds the Age modulation///////-------------------------------------------------
	    // get a random value that updates every roughly 0.1*delay time, but with some randomness in how often one is selected
	    if(process_counter%((int)(time_long/10))==(int)(get_rand()*time_long*0.05))
	        rand_val = get_rand();
	    // Smooth out the random values to make them less jarring
	    fonepole(rand_now,rand_val,1/(48000.0f*1/10.0f));//when the denominator is smaller the interpolation takes longer

	    osc.SetFreq(0.25*samplerate/time_long);// samplerate/time_long should be the delaytime in Hz of the longest delay (head 4)
	    osc2.SetFreq(0.17*samplerate/time_long);
	    float max_amp = 0.02;//Make this number bigger if you want more modulation in the Age control ---- this is the modulation/Age magic number.
	    float amp_val;
	    if(age_val<0.5)//This maxes out the age_val at noon on the knob, so >Noon only addes degregation
	        amp_val = age_val*max_amp;
	    else
	        amp_val = max_amp/2;
	    // We will slowly build up the shape of the modulation for the Age parameter, comment out any of the lines below the first one to remove that component of the modulation
	    mod_total = mod_osc;//This is the base sine wave
	    mod_total *=(1+0.2*(mod_osc2+0.5));//This adds a small sq wave for some abrupt changes
	    mod_total *= (1+(rand_now-0.5)*0.6);//This adds a slowly changing random value, pretty subtle right now
	    mod_total *= amp_val; // This makes the amplitude of the modulation obey the value of the age knob

	    // Add the degredation to the Age knob when it is above noon
	    // ---this is kind of 'code-bending', we are changing hte delay time really fast and allowing it to inject artifacts into the delayed sound on purpose.
	    if(age_val>0.5 && age_val <0.6)
	        mod_total += (powf(.6,3)/0.1)*(age_val-.5)*get_rand()/10; 
	    if(age_val >0.6)
	        mod_total += powf(age_val,3)*get_rand()/10;
	
	    //Below is for sending the mod signal to the dac_outputs to look at the waveform
	    
	    /////// End of building the Age (it gets added to the delay times just below)///////--------------------------------

	    //  Below each delay line has its own delay value
	    for(int i = 0; i < 4; i++)
	    //The (i+0.25-i*0.75) just sets the delay intervals to 1/4,1/2/,3/4,1.0 for i=0,1,2,3
	    //delays[i].delayTarget = (i+0.25-i*0.75)*time_long; //this is NO Age (modulation)
            if(vertigo.sw[1].Read() == 0) {
                delays[i].delayTarget = ((i+0.25-i*0.75)+dadj)*time_long*(1+mod_total); //Center: 1/4,2/4,3/4,4/4
            } else if (vertigo.sw[1].Read() == 1) {
                delays[i].delayTarget = ((i+0.375-i*0.625)+dadj)*time_long*(1+mod_total); //Up: 3/8,6/8,9/8,12/8
            } else if (vertigo.sw[1].Read() == 2) {
                delays[i].delayTarget = ((i+0.1875-i*0.8125)+dadj)*time_long*(1+mod_total); //Down: 3/16,6/16,9/16,12/16
            }
	    	
	    drywet_ratio = mixParam.Process();
   
	    //feedback is processed above, since it will depend on what footswitch 2

	    // Below are the delays for the 'swell' reverb like sound
	    //for(int i = 0; i < 4; i++)
	    //    swells[i].delayTarget = (i+0.25-i*0.75)*time_long;//*(1+get_rand()*0.01);
        
    }//ends part 2
    
    if(part==3){
	    // These are the cutoff freqs for the high and low pass filters
	    float tone_freqHP_L;
	    float tone_freqLP_L;
        float tone_freqHP_R;
	    float tone_freqLP_R;

	    // swell parameter updating
	    //swell_val = powf(swellParam.Process(),1/1.5);
	
	    // use a potentiameter for a tone control - toneParam sweeps from -1 to 1
	    tone_val = toneParam.Process();
    
	    if (tone_val<0.0f){ // left half of pot HP off, LP on
	        tone_freqHP_L = 0;
	        tone_freqLP_L = 5000.0f*(powf(10,2*tone_val))+100.f;//This is a more complex function just to make the taper nice and smooth, the filter turned on too fast when linear
            tone_freqHP_R = 0;
	        tone_freqLP_R = 5000.0f*(powf(10,2*tone_val))+100.f;
	    } else{// right half of pot HP on, LP off 
	        tone_freqHP_L = 5000.0f*powf(10,2.f*tone_val-2);//This is a more complex function just to make the taper nice and smooth, the filter turned on too fast when linear
	        tone_freqLP_L = 1000000.0f;// just something very high so the filter is not killing any actual guitar sound
            tone_freqHP_R = 5000.0f*powf(10,2.f*tone_val-2);//This is a more complex function just to make the taper nice and smooth, the filter turned on too fast when linear
	        tone_freqLP_R = 1000000.0f;// just something very high so the filter is not killing any actual guitar sound
	    }
	    toneHP_L.SetFreq(tone_freqHP_L);
	    toneLP_L.SetFreq(tone_freqLP_L);
        toneHP_R.SetFreq(tone_freqHP_R);
	    toneLP_R.SetFreq(tone_freqLP_R);

        //width_val = stereoWidthParam.Process();
    

    }
    if(part==4){
        drylevel = verbMixParam.Process();
    }
    
}


void UpdateSwitches() {

    vertigo.s[DaisyVertigo::S_Left].Debounce();
    vertigo.s[DaisyVertigo::S_Right].Debounce();
    vertigo.s[DaisyVertigo::S_Tap].Debounce();

    if(vertigo.s[DaisyVertigo::S_Left].RisingEdge()){
        swLeftState += 1;
        if(swLeftState > 3) {
           swLeftState = 0;     
        } 

    }

    if(vertigo.s[DaisyVertigo::S_Right].RisingEdge()){
        swRightState += 1;
        if(swRightState > 3) {
           swRightState = 0;     
        } 
    }

    if (vertigo.s[DaisyVertigo::S_Tap].RisingEdge())    
    {
        if(BaseTempo.tap()) //if tempo changed
        {
            
        }
        
    }

}

bool DelaySwCheck(int idx){
    switch (idx)
    {
    case 0:
        if(swLeftState == 1 || swLeftState == 3) {
            return true;
        } else {
            return false;
        }
        break;
     case 1:
        if(swLeftState == 2 || swLeftState == 3) {
            return true;
        } else {
            return false;
        }
        break;
     case 2:
        if(swRightState == 1 || swRightState == 3) {
            return true;
        } else {
            return false;
        }
        break;
     case 3:
        if(swRightState == 2 || swRightState == 3) {
            return true;
        } else {
            return false;
        }
        break;
    
    default:
    return true;
        break;
    }
}

void UpdateClock()
{
    static uint32_t ClockCounter{};

    ClockCounter += 1; //increment by one
    //if clock in pulse received
    if (vertigo.Gate())     
    {   
        
        //tempoLED_BASE.resetPhase();
            if(BaseTempo.clock(ClockCounter)) //if valid tap resistered
            {

            }
            ClockCounter = 0; //reset counter

    } 
}