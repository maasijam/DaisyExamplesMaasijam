// GRNLTR test vehicle
//
// Check UpdateEncoder function for controls
//
#include <stdio.h>
#include <string.h>
#include "../daisy_saul.h"
#include "fatfs.h"
//#include "led_colours.h"
#include "util/wav_format.h"
#include "Effects/decimator.h"
#include "PagedParam.h"
#include "windows.h"
#include "granulator.h"

using namespace daisy;
using namespace daisysp;
using namespace saul;

static Granulator grnltr;
static Decimator crush;
static DaisySaul hw;
static Parameter pot1, pot2, pot3, pot4, pot5, pot6, pot7, pot8, pot9, pot10;

#define GRAIN_ENV_SIZE 1024
#define NUM_GRAIN_ENVS 6
float rect_env[GRAIN_ENV_SIZE];
float gauss_env[GRAIN_ENV_SIZE];
float hamming_env[GRAIN_ENV_SIZE];
float hann_env[GRAIN_ENV_SIZE];
float expo_env[GRAIN_ENV_SIZE];
float rexpo_env[GRAIN_ENV_SIZE];
float *grain_envs[] = {rect_env, gauss_env, hamming_env, hann_env, expo_env, rexpo_env};
size_t cur_grain_env = 2; 

#define MAX_WAVES 16

/**< Maximum LFN (set to same in FatFs (ffconf.h) */
#define WAV_FILENAME_MAX 256 

// 64 MB of memory - how many 16bit samples can we fit in there?
static int16_t DSY_SDRAM_BSS sm[(64 * 1024 * 1024) / sizeof(int16_t)];
size_t sm_size = sizeof(sm);
size_t cur_sm_bytes = 0;

// Buffer for copying wav files to SDRAM
#define BUF_SIZE 8192
char buf[BUF_SIZE];

WavFileInfo wav_file_names[MAX_WAVES];
uint8_t	    wav_file_count = 0;
size_t	    wav_start_pos[MAX_WAVES];

#define PARAM_THRESH 0.01f
static PagedParam pitch_p, rate_p, crush_p, downsample_p, grain_duration_p, \
		  grain_density_p, scatter_dist_p, pitch_dist_p, sample_start_p, sample_end_p;

#define NUM_PAGES 6
int8_t cur_page = 0;
int8_t cur_wave = 0;

float sr;


SdmmcHandler   sd;
FatFSInterface fsi;
FIL            SDFile;



void UpdateButtons()
{
  
      if(hw.s[S_2].RisingEdge()) {
	      cur_grain_env++;
	      if (cur_grain_env == NUM_GRAIN_ENVS) {
	        cur_grain_env = 0;
	      }
	      grnltr.ChangeEnv(grain_envs[cur_grain_env]);
      }
      if(hw.s[S_6].RisingEdge()) {
	      pitch_p.lock(1.0);
  	    rate_p.lock(1.0);
      }
      
      if(hw.s[S_4].RisingEdge()) {
	      grnltr.ToggleGrainReverse();
      }
      if(hw.s[S_5].RisingEdge()) {
	      grnltr.ToggleScanReverse();
      }
      
      if(hw.s[S_0].RisingEdge()) {
	      grnltr.ToggleScatter();
      }
      if(hw.s[S_1].RisingEdge()) {
	      grnltr.ToggleFreeze();
      }
      grnltr.ToggleRandomPitch((hw.sw[SW_1].Read() == 1 ? true : false));
      
      if(hw.s[S_3].RisingEdge()) {
	      cur_wave++;
	      if (cur_wave >= wav_file_count) cur_wave = 0;
	      grnltr.Stop();
	      grnltr.Reset( \
	        &sm[wav_start_pos[cur_wave]], \
	        wav_file_names[cur_wave].raw_data.SubCHunk2Size / sizeof(int16_t));
    	  grnltr.Dispatch(0);
      }
      grnltr.ToggleSampleLoop((hw.sw[SW_0].Read() == 1 ? true : false));
      
  
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
  float sample, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;

  // It should be possible to move the param handling to the main loop instead of the audio callback
  hw.ProcessDigitalControls();
  UpdateButtons();
 

  p1 = pot1.Process();
  p2 = pot2.Process();
  p3 = pot3.Process();
  p4 = pot4.Process();
  p5 = pot5.Process();
  p6 = pot6.Process();
  p7 = pot7.Process();
  p8 = pot8.Process();
  p9 = pot9.Process();
  p10 = pot10.Process();

  grnltr.SetGrainPitch(pitch_p.Process(p1));
  grnltr.SetScanRate(rate_p.Process(p2));
  grnltr.SetGrainDuration(grain_duration_p.Process(p3));
  grnltr.SetDensity((int32_t)grain_density_p.Process(p4));
  grnltr.SetScatterDist(scatter_dist_p.Process(p5));
  grnltr.SetPitchDist(pitch_dist_p.Process(p6));
  grnltr.SetSampleStart(sample_start_p.Process(p7));
  grnltr.SetSampleEnd(sample_end_p.Process(p8));
  crush.SetBitcrushFactor(crush_p.Process(p9));
  crush.SetDownsampleFactor(downsample_p.Process(p10));

  //audio
  for(size_t i = 0; i < size; i++)
  {
    sample = grnltr.Process();
    sample = crush.Process(sample);
    out[0][i] = sample;
    out[1][i] = sample;
  }
}

int ReadWavsFromDir(const char *dir_path)
{
  DIR dir;
  FILINFO fno;
  char *  fn;

  size_t bytesread;

  //hw.led1.Set(GREEN);
  hw.SetRGBLed(4,GREEN);
  //hw.UpdateLeds();

  // See /data/daisy/DaisyExamples-sm/libDaisy/src/hid/wavplayer.*
  // Open Dir and scan for files.
  if(f_opendir(&dir, dir_path) != FR_OK)
  {
      return -1;
  }
  while((f_readdir(&dir, &fno) == FR_OK) && (wav_file_count < MAX_WAVES)) {
    // Exit if NULL fname
    if(fno.fname[0] == 0)
        break;
    // Skip if its a directory or a hidden file.
    if(fno.fattrib & (AM_HID | AM_DIR))
        continue;
    // Now we'll check if its .wav and add to the list.
    fn = fno.fname;
    if(strstr(fn, ".wav") || strstr(fn, ".WAV"))
    {
      strcpy(wav_file_names[wav_file_count].name, dir_path);
      strcat(wav_file_names[wav_file_count].name, fn);
      wav_file_count++;
    }
  }
  f_closedir(&dir);
  
  // Now we'll go through each file and load the WavInfo.
  for(size_t i = 0; i < wav_file_count; i++)
  {
    hw.SetRGBLed(4,BLUE);
    // Read the test file from the SD Card.
    if(f_open(&SDFile, wav_file_names[i].name, FA_READ) == FR_OK)
    {
      // TODO: Add checks here to ensure we can deal with the wav file correctly
      f_read(&SDFile, (void *)&wav_file_names[i].raw_data, sizeof(WAV_FormatTypeDef), &bytesread);
      size_t wav_size = wav_file_names[i].raw_data.SubCHunk2Size;
      if ((cur_sm_bytes + wav_size) > sm_size) break;
      size_t this_wav_start_pos = (cur_sm_bytes / sizeof(int16_t)) + 1;
      wav_start_pos[i] = this_wav_start_pos;

      do {
	f_read(&SDFile, (void *)buf, BUF_SIZE, &bytesread);
	memcpy((void *)&sm[this_wav_start_pos], buf, bytesread);
      	cur_sm_bytes += bytesread;
	this_wav_start_pos = (cur_sm_bytes / sizeof(int16_t));
      } while (bytesread == BUF_SIZE);

      f_close(&SDFile);
    }
    hw.SetRGBLed(4,OFF);
    System::Delay(250);
  }
  return 0;
}

int main(void)
{
    rectangular_window(rect_env, GRAIN_ENV_SIZE);
    gaussian_window(gauss_env, GRAIN_ENV_SIZE, 0.5);
    hamming_window(hamming_env, GRAIN_ENV_SIZE, EQUIRIPPLE_HAMMING_COEF);
    hann_window(hann_env, GRAIN_ENV_SIZE);
    expodec_window(expo_env, rexpo_env, GRAIN_ENV_SIZE, TAU);

    // Init hardware
    hw.Init();
    sr = hw.AudioSampleRate();

    hw.SetRGBLed(4,RED);
    //hw.UpdateLeds();
    // Init SD Card
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sd.Init(sd_cfg);

    System::Delay(250);

    hw.SetRGBLed(4,PURPLE);

    // Links libdaisy i/o to fatfs driver.
    fsi.Init(FatFSInterface::Config::MEDIA_SD);

    System::Delay(250);

    hw.SetRGBLed(4,YELLOW);

    // Mount SD Card
    f_mount(&fsi.GetSDFileSystem(), "/", 1);

    System::Delay(250);

    ReadWavsFromDir("/");

    // unmount
    f_mount(0, "/", 0);

    System::Delay(250);

    hw.SetRGBLed(4,AQUA);

    grnltr.Init(sr, \
        &sm[wav_start_pos[cur_wave]], \
        wav_file_names[cur_wave].raw_data.SubCHunk2Size / sizeof(int16_t), \
        grain_envs[cur_grain_env], \
        GRAIN_ENV_SIZE);
    grnltr.Dispatch(0);

    pot1.Init(hw.knob[KNOB_0], 0.0f, 1.0f, Parameter::LINEAR);
    pot2.Init(hw.knob[KNOB_1], 0.0f, 1.0f, Parameter::LINEAR);
    pot3.Init(hw.knob[KNOB_2], 0.0f, 1.0f, Parameter::LINEAR);
    pot4.Init(hw.knob[KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);
    pot5.Init(hw.knob[KNOB_4], 0.0f, 1.0f, Parameter::LINEAR);
    pot6.Init(hw.knob[KNOB_5], 0.0f, 1.0f, Parameter::LINEAR);
    pot7.Init(hw.knob[KNOB_6], 0.0f, 1.0f, Parameter::LINEAR);
    pot8.Init(hw.knob[KNOB_7], 0.0f, 1.0f, Parameter::LINEAR);
    pot9.Init(hw.knob[KNOB_8], 0.0f, 1.0f, Parameter::LINEAR);
    pot10.Init(hw.knob[KNOB_9], 0.0f, 1.0f, Parameter::LINEAR);

    pitch_p.Init(           DEFAULT_GRAIN_PITCH,	  0.25f,  4.0f,	PARAM_THRESH);
    rate_p.Init(            DEFAULT_SCAN_RATE,        0.25f,  4.0f, PARAM_THRESH);
    grain_duration_p.Init(  DEFAULT_GRAIN_DUR,        0.01f,  0.2f, PARAM_THRESH);
    grain_density_p.Init(   sr/DEFAULT_GRAIN_DENSITY, sr/200, sr/2,	PARAM_THRESH);
    scatter_dist_p.Init(    DEFAULT_SCATTER_DIST,	  0.0f,   1.0f, PARAM_THRESH);
    pitch_dist_p.Init(      DEFAULT_PITCH_DIST,       0.0f,   1.0f, PARAM_THRESH);
    sample_start_p.Init(    0.0f,			  0.0f,   1.0f, PARAM_THRESH);
    sample_end_p.Init(	    1.0f,			  0.0f,   1.0f, PARAM_THRESH);
    crush_p.Init(           0.0f,                     0.0f,   1.0f, PARAM_THRESH);
    downsample_p.Init(      0.0f,                     0.0f,   1.0f, PARAM_THRESH);
    
    crush.Init();
    crush.SetDownsampleFactor(0.0f);

    // GO!
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    bool ledstate;
    ledstate = true;
    // The onboard LED will begin to blink.
    for(;;)
    {
        System::Delay(250);
        hw.seed.SetLed(ledstate);
        ledstate = !ledstate;
    }
}
