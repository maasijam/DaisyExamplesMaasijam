#include <array>

#include "../daisy_pinkman.h"
#include "daisysp.h"
#include "settings.h"
#include "ui.h"






using namespace daisy;
using namespace daisysp;
using namespace pinkman;

DaisyPinkman hw;
Settings settings;
Ui ui;


FatFSInterface fsi;
FIL            file;

static Oscillator osc;
Svf        svf;



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
        
        hw.WriteCvOut(chn,(osc_lfo.Process() + 1.f) * levelCtrl.Process() * 5.f);
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
    int   num_waves = Oscillator::WAVE_LAST - 1;

    float cutoff_ = fmap(hw.CVKnobCombo(hw.controls[CV_5].Value(),hw.controls[POT_5].Value()), 20.f, 20000.f,Mapping::LOG);
    float freqctrl_ = fmap(hw.CVKnobCombo(hw.controls[CV_1].Value(),hw.controls[POT_1].Value()), 10.f, 110.f,Mapping::LINEAR);
    float finectrl_ = fmap(hw.CVKnobCombo(hw.controls[CV_2].Value(),hw.controls[POT_2].Value()), 0.f, 7.f,Mapping::LINEAR);
    int wavectrl_ =  fmap(hw.CVKnobCombo(hw.controls[CV_3].Value(),hw.controls[POT_3].Value()), 0.f, num_waves,Mapping::LINEAR);
    float ampctrl_ = fmap(hw.CVKnobCombo(hw.controls[CV_4].Value(),hw.controls[POT_4].Value()), 0.f, 0.5f,Mapping::LINEAR);
    float res_ = fmap(hw.CVKnobCombo(hw.controls[CV_6].Value(),hw.controls[POT_6].Value()), .3f, 1.f,Mapping::LINEAR);
    //float drive_ =  fmap(hw.CVKnobCombo(hw.controls[CV_7].Value(),hw.controls[POT_7].Value()), .3f, 1.f,Mapping::LINEAR);
    float drive_ = 0.5f;
    
    
    
    float sig, freq, amp;
    size_t wave;

    //float cutoff_ = cutoff.Process();
    //float res_    = res.Process();
    //float drive_  = drive.Process();
    

    svf.SetFreq(cutoff_);
    svf.SetRes(res_);
    svf.SetDrive(drive_);
    
    for(size_t i = 0; i < size; i++)
    {
        freq = mtof(freqctrl_ + finectrl_);
        wave = wavectrl_;
        amp  = ampctrl_;

        osc.SetFreq(freq);
        osc.SetWaveform(wave);
        osc.SetAmp(amp);
        
        sig = osc.Process();

        svf.Process(sig);

        // left out
        out[0][i] = svf.Low();
        out[1][i] = svf.Low();

        lfo.Process(CV_OUT_2);

        

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
    
            	
    
    
    Start_Led_Ani();

    osc.Init(samplerate);
    svf.Init(samplerate);


    // Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SAW);
    osc.SetFreq(30);
    osc.SetAmp(0.75);

    lfo.Init(samplerate, hw.controls[POT_7], hw.controls[POT_8]);

     

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
       //hw.led[LED_2].Set(0.f);
       // hw.led[LED_2].Update();
    } else {
       // hw.led[LED_2].Set(1.f);
       // hw.led[LED_2].Update();
    }

    
        

    /** 5 second delay before starting streaming test. */
    System::Delay(1000);
    
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
        hw.s[i].Debounce();
    }
    hw.toggle.Debounce();
   

    //led[LED_2].Set(s[S_1].Pressed() ? 1.f : 0.f);
    //led[LED_2].Update();
    hw.SetLed(LED_1,hw.s[S_BIG].Pressed());
    hw.SetLed(LED_2,hw.s[S_TOP].Pressed());
    hw.SetLed(LED_3,hw.s[S_RED].Pressed());
    hw.SetLed(LED_4,hw.toggle.Pressed());
    hw.SetLed(LED_5,!hw.toggle.Pressed());
    
    
    hw.UpdateLeds();
    
    
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
