#include <array>

#include "../daisy_pinkman.h"
#include "daisysp.h"
#include "settings.h"
#include "ui.h"



enum Switches {
  S_1,
  S_2,
  S_3,
  S_LAST
};

enum Leds {
  LED_1,
  LED_2,
  LED_3,
  LED_4,
  LED_5,
  LED_LAST
};


using namespace daisy;
using namespace daisysp;
using namespace pinkman;

DaisyPinkman hw;
Settings settings;
Ui ui;
Switch s[S_LAST];
Switch toggle;
Led led[LED_LAST];

FatFSInterface fsi;
FIL            file;

static Oscillator osc;
Parameter  freqctrl, wavectrl, ampctrl, finectrl;

void Update_Digital();
void Start_Led_Ani();
void LoadSettings();
void LoadState();
void SaveSettings();
void SaveState();


bool readyToSave = false;
bool readyToRestore = false;


struct lfoStruct
{
    Oscillator osc_lfo;
    Parameter  rateCtrl;
    Parameter  levelCtrl;
    float      amp;
    float      freq;
    int        waveform;
    float      value;

    void Init(float samplerate, AnalogControl rateKnob, AnalogControl levelKnob)
    {
        osc_lfo.Init(samplerate);
        osc_lfo.SetAmp(1);
        waveform = osc_lfo.WAVE_SIN;
        rateCtrl.Init(rateKnob, .1, 35, Parameter::LOGARITHMIC);
        levelCtrl.Init(levelKnob, 0, 1, Parameter::LINEAR);
    }

    void Process(int chn)
    {
        //read the knobs and set params
        osc_lfo.SetFreq(rateCtrl.Process());
        osc_lfo.SetWaveform(waveform);

        //write to the DAC
        
        hw.WriteCvOut(chn,(osc_lfo.Process() + 1.f) * .5f * levelCtrl.Process());
    }
};

lfoStruct lfo;




uint8_t decay;

void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	//ui.Poll();
    hw.ProcessAllControls();
    
    
    
    float sig, freq, amp;
    size_t wave;
    
    for(size_t i = 0; i < size; i++)
    {
        freq = mtof(freqctrl.Process() + finectrl.Process());
        wave = wavectrl.Process();
        amp  = ampctrl.Process();

        osc.SetFreq(freq);
        osc.SetWaveform(wave);
        osc.SetAmp(amp);
        
        sig = osc.Process();

        // left out
        out[0][i] = sig;
        out[1][i] = sig;

        lfo.Process(CV_OUT_1);

        

    }

    
}



int main(void)
{
    hw.Init();
    settings.Init(&hw);
  
    ui.Init(&settings, &hw);

    LoadSettings();
    LoadState();

    float samplerate = hw.AudioSampleRate();
    int   num_waves = Oscillator::WAVE_LAST - 1;
            	
    
    
    Start_Led_Ani();

    osc.Init(samplerate);


    // Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SAW);
    osc.SetFreq(30);
    osc.SetAmp(0.75);

    lfo.Init(samplerate, hw.controls[POT_7], hw.controls[POT_8]);

    freqctrl.Init(hw.controls[POT_1], 10.0, 110.0f, Parameter::LINEAR);
    finectrl.Init(hw.controls[POT_2], 0.f, 7.f, Parameter::LINEAR);
    wavectrl.Init(hw.controls[POT_3], 0.0, num_waves, Parameter::LINEAR);
    ampctrl.Init(hw.controls[POT_4], 0.0, 0.5f, Parameter::LINEAR);

    dsy_gpio_pin s_pins[S_LAST] = {hw.B7,hw.B8,hw.D10};
    for (size_t i = 0; i < S_LAST; i++)
    {
        s[i].Init(s_pins[i]);
    }


    dsy_gpio_pin led_pins[LED_LAST] = {hw.C10,hw.D8,hw.D9,hw.B5,hw.B6};
    for (size_t i = 0; i < LED_LAST; i++)
    {
        led[i].Init(led_pins[i],false);
    }

    toggle.Init(hw.A3);

    /** SD card next */
    SdmmcHandler::Config sd_config;
    SdmmcHandler         sdcard;
    sd_config.Defaults();
    sd_config.speed = SdmmcHandler::Speed::SLOW;
    sdcard.Init(sd_config);

    fsi.Init(FatFSInterface::Config::MEDIA_SD);


    /** Write/Read text file */
    const char *test_string = "Testing Daisy Pinkman";
    const char *test_fname  = "DaisyPinkman-Test.txt";
    FRESULT     fres = FR_DENIED; /**< Unlikely to actually experience this */
    if(f_mount(&fsi.GetSDFileSystem(), "/", 0) == FR_OK)
    {
        /** Write Test */
        if(f_open(&file, test_fname, (FA_CREATE_ALWAYS | FA_WRITE)) == FR_OK)
        {
            UINT   bw  = 0;
            size_t len = strlen(test_string);
            fres       = f_write(&file, test_string, len, &bw);
        }
        f_close(&file);
        if(fres == FR_OK)
        {
            /** Read Test only if Write passed */
            if(f_open(&file, test_fname, (FA_OPEN_EXISTING | FA_READ)) == FR_OK)
            {
                UINT   br = 0;
                char   readbuff[32];
                size_t len = strlen(test_string);
                fres       = f_read(&file, readbuff, len, &br);
            }
            f_close(&file);
        }
    }
    bool sdmmc_pass = fres == FR_OK;
    // If what was read does not match
    // what was written execution will stop.
    if(!sdmmc_pass)
    {
       led[LED_2].Set(0.f);
        led[LED_2].Update();
    } else {
        led[LED_2].Set(1.f);
        led[LED_2].Update();
    }

    
        

    /** 5 second delay before starting streaming test. */
    System::Delay(2000);
    
    //hw.StartAdc();
	hw.StartAudio(audio_callback);

	while(1)
	{	
		 //hw.ProcessAllControls();
        Update_Digital(); 
         if (readyToSave) {
            /** Collect the data from where ever in the application it is */
            SaveSettings();
            SaveState();

            /** And trigger the save */
            
            readyToSave = false;
        }
        if (readyToRestore) {
            /** Collect the data from where ever in the application it is */
            settings.RestoreSettings();

            /** And trigger the save */
            
            readyToRestore = false;
        }
        
	}
}


void Update_Digital() {

    for (size_t i = 0; i < S_LAST; i++)
    {
        s[i].Debounce();
    }
    toggle.Debounce();
   

    //led[LED_2].Set(s[S_1].Pressed() ? 1.f : 0.f);
    //led[LED_2].Update();

    led[LED_3].Set(s[S_2].Pressed() ? 1.f : 0.f);
    led[LED_3].Update();

    if(s[S_3].Pressed()){
        dsy_gpio_write(&hw.gate_out_1, true);
        //led[LED_4].Set(1.f);
        //led[LED_4].Update();
    } else {
        dsy_gpio_write(&hw.gate_out_1, false);
        //led[LED_4].Set(s[S_3].Pressed() ? 1.f : 0.f);
        //led[LED_4].Update();
    }

    if(toggle.Pressed()){
        dsy_gpio_write(&hw.gate_out_2, true);
        //led[LED_4].Set(1.f);
        //led[LED_4].Update();
        hw.WriteCvOut(CV_OUT_1,0.f);
    } else {
        dsy_gpio_write(&hw.gate_out_2, false);
        //led[LED_4].Set(s[S_3].Pressed() ? 1.f : 0.f);
        //led[LED_4].Update();
        hw.WriteCvOut(CV_OUT_1,5.f);
    }

    
    
    
    
}

void Update_Controls() {
    hw.ProcessAllControls();
    
}

void Start_Led_Ani() {
    size_t deltime = 100;
    //hw.ClearLeds();
    
    
    

    //hw.UpdateLeds();
}

void LoadSettings(){
    const HwtestSettings& hwtestsettings = settings.hwtestsettings();
    
    
    for(int i = 0; i < 14; i++)
    {
        //ledstate[i] = hwtestsettings.ledstate[i];
    }
}

void LoadState(){
    const StateSettings& statesettings = settings.statesettings();
    decay = statesettings.decay;
}

void SaveSettings(){
    HwtestSettings* hwtestsettings = settings.margolis_hwtestsettings();
   
    
    for(int i = 0; i < 14; i++)
    {
        //hwtestsettings->ledstate[i] = ledstate[i];
    }
    
    settings.SaveSettings();
}

void SaveState(){
    StateSettings* statesettings = settings.margolis_statesettings();
    statesettings->decay = decay;
    
    settings.SaveStateSettings();
}
