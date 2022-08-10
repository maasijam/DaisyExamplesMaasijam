#include <array>

#include "../daisy_white.h"
#include "daisysp.h"
//#include "wavplayer2.h"



using namespace daisy;
using namespace daisysp;

//int16_t DSY_SDRAM_BSS my_sample[1024*1024*32];



DaisyWhite hw;
//SdmmcHandler   sdh;
//FatFSInterface fsi;
//FIL            file;
//WavPlayer2      sampler;


void Update_Buttons();
void sdcard_init();

enum
{
    S1      = 15,
    S2      = 14,
    S3      = 13,
    S4      = 12,
    S5      = 11,
    S6      = 10,
    S7      = 9,
    S8      = 8,
    S0A     = 7,
    S0B     = 6,
    S1A     = 5,
    S1B     = 4,
};


struct lfoStruct
{
    Oscillator osc;
    Parameter  freqCtrl;
    Parameter  ampCtrl;
    float      amp_;
    float      freq_;
    int        waveform;
    float      value;

    void Init(float samplerate, float freq, float amp)
    {
        osc.Init(samplerate);
        osc.SetAmp(1);
        waveform = 0;
        freq_ = freq;
        amp_ = amp;
    }

    void Process(DacHandle::Channel chn)
    {
        //read the knobs and set params
        osc.SetFreq(freq_);
        osc.SetWaveform(waveform);

        //write to the DAC
        hw.seed.dac.WriteValue(
            chn,
            uint16_t((osc.Process() + 1.f) * .5f * amp_ * 4095.f));
    }
};

lfoStruct lfos[2];

bool time_r = false;


void audio_callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                                size)
{
	
    //int32_t* samples_out;
        
    for(size_t i = 0; i < size; i ++)
    {
        lfos[0].Process(DacHandle::Channel::ONE);
        lfos[1].Process(DacHandle::Channel::TWO);

        out[0][i] = in[0][i]; 
		out[1][i] = in[0][i];
        //samples_out = sampler.StreamStereo();
        //out[i] = s162f(samples_out[0]);
        //out[i+1] = s162f(samples_out[1]);

        //out[i] = out[i + 1] = s162f(my_sample[i]);
    }
}


int main(void)
{
	size_t blocksize = 48;
    hw.Init();
    //sdcard_init();
    //sampler.Open(0);
	//sampler.SetLooping(true);
    //size_t curfile;
    //curfile = sampler.GetCurrentFile();
    //sampler.Open(curfile);
    
    //const char *fname  = "genent.wav";
    //FRESULT     fres = FR_DENIED; /**< Unlikely to actually experience this */
    /*
    if(f_mount(&fsi.GetSDFileSystem(), "/", 0) == FR_OK)
    {
        
            //genent.wav
            
            if(f_open(&file, fname, (FA_OPEN_EXISTING | FA_READ)) == FR_OK)
            {
                UINT   br = 0;
                //char   readbuff[32];
                //size_t len = strlen(test_string);
                fres       = f_read(&file, my_sample, 1024*1024*32 * sizeof(my_sample[0]), &br);
            }
            f_close(&file);
       
    }
    */


    float samplerate = hw.AudioSampleRate();

    //init the lfos
    lfos[0].Init(samplerate, 0.5f, 0.5f);
    lfos[1].Init(samplerate, 10.f, 0.8f);
    
    

    	
    hw.StartAdc();
    hw.SetAudioBlockSize(blocksize);
	hw.StartAudio(audio_callback);

    

	while(1)
	{	
		hw.ProcessAllControls();
		Update_Buttons();
        //Update_Leds();
        //sampler.Prepare();

        //wait 1 ms
        //System::Delay(1);	

        	
	}
}


void Update_Buttons() {

    hw.ClearLeds();


	if(!hw.SwitchState(S1)){
       hw.SetGreenLeds(DaisyWhite::GREEN_LED_1, 1.f);
    } 
    if(!hw.SwitchState(S2)){
        hw.SetGreenLeds(DaisyWhite::GREEN_LED_3, 1.f);
    } 
    if(!hw.SwitchState(S3)){
       hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_6, 1.f);
    } 
    if(!hw.SwitchState(S4)){
        hw.SetGreenLeds(DaisyWhite::GREEN_LED_4, 1.f);
        
    } 
    if(!hw.SwitchState(S5)){
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_5, 1.f);
        
    } 
    if(!hw.SwitchState(S6)){
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_2, 1.f);
    } 
    if(!hw.SwitchState(S7)){
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_1, 1.f);
    } 
    if(hw.SwitchRisingEdge(S8)){
        time_r = !time_r;
    } 
    hw.SetGreenLeds(DaisyWhite::GREEN_LED_2, (time_r ? 1.f : 0.f));
    
    if(!hw.SwitchState(S1A)){
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_4, 1.f);
    } 
    if(!hw.SwitchState(S1B)){
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_3, 1.f);
    } 
   
    if(!hw.SwitchState(S0A)){
        hw.SetRgbLeds(DaisyWhite::RGB_LED_1, 0.f,0.f,1.f * hw.CVKnobCombo(hw.GetCvValue(0),hw.GetKnobValue(0)));
        hw.SetRgbLeds(DaisyWhite::RGB_LED_2, 0.f,0.f,1.f * hw.CVKnobCombo(hw.GetCvValue(1),hw.GetKnobValue(1)));
        hw.SetRgbLeds(DaisyWhite::RGB_LED_3, 0.f,0.f,1.f * hw.CVKnobCombo(hw.GetCvValue(2),hw.GetKnobValue(2)));
        hw.SetRgbLeds(DaisyWhite::RGB_LED_4, 0.f,0.f,1.f * hw.CVKnobCombo(hw.GetCvValue(3),hw.GetKnobValue(3)));
    } else if(!hw.SwitchState(S0B)) {
        hw.SetRgbLeds(DaisyWhite::RGB_LED_1, 0.f,1.f * hw.CVKnobCombo(hw.GetCvValue(4),hw.GetKnobValue(4)),0.f);
        hw.SetRgbLeds(DaisyWhite::RGB_LED_2, 0.f,1.f * hw.CVKnobCombo(hw.GetCvValue(5),hw.GetKnobValue(5)),0.f);
        hw.SetRgbLeds(DaisyWhite::RGB_LED_3, 0.f,1.f * hw.CVKnobCombo(hw.GetCvValue(6),hw.GetKnobValue(6)),0.f);
        hw.SetRgbLeds(DaisyWhite::RGB_LED_4, 0.f,1.f * hw.CVKnobCombo(hw.GetCvValue(7),hw.GetKnobValue(7)),0.f);
    } else {
        hw.SetRgbLeds(DaisyWhite::RGB_LED_1, 1.f * hw.GetKnobValue(8),0.f,0.f);
        hw.SetRgbLeds(DaisyWhite::RGB_LED_2, 1.f * hw.GetKnobValue(9),0.f,0.f);
        hw.SetRgbLeds(DaisyWhite::RGB_LED_3, 1.f * hw.GetKnobValue(10),0.f,0.f);
        hw.SetRgbLeds(DaisyWhite::RGB_LED_4, 1.f * hw.GetKnobValue(11),0.f,0.f);
    }
    

    if(!hw.GateIn1()) {
        hw.SetGreenLeds(DaisyWhite::GREEN_LED_3, 1.f);
        hw.SetGreenLeds(DaisyWhite::GREEN_LED_4, 1.f);
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_6, 1.f);
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_5, 1.f);
        dsy_gpio_write(&hw.gate_out_1, true);
    } else {
        dsy_gpio_write(&hw.gate_out_1, false);
    }
    if(!hw.GateIn2()) {
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_4, 1.f);
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_2, 1.f);
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_3, 1.f);
        hw.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_1, 1.f);
        dsy_gpio_write(&hw.gate_out_2, true);
    } else {
        dsy_gpio_write(&hw.gate_out_2, false);
    }
    
      hw.UpdateLeds();
}
/*
void sdcard_init() {
	daisy::SdmmcHandler::Config sdconfig;
	sdconfig.Defaults(); 
	sdconfig.speed           = daisy::SdmmcHandler::Speed::MEDIUM_SLOW; 
    sdconfig.width           = daisy::SdmmcHandler::BusWidth::BITS_4;
	sdh.Init(sdconfig);
	fsi.Init(FatFSInterface::Config::MEDIA_SD);
	f_mount(&fsi.GetSDFileSystem(), "/", 0);
    //f_mount(&fsi.GetSDFileSystem(), fsi.GetSDPath(), 1);
	//sampler.Init(fsi.GetSDPath(),buffer,BUFSIZE,2);

}

int sdcard_load_wav(const char * filename, Data& gendata) {
			float * buffer = gendata.mData;
			size_t buffer_frames = gendata.dim;
			size_t buffer_channels = gendata.channels;
			size_t bytesread = 0;
			WavFormatChunk format;
			uint32_t header[3];
			uint32_t marker, frames, chunksize, frames_per_read, frames_to_read, frames_read, total_frames_to_read;
			uint32_t buffer_index = 0;
			size_t bytespersample;
			if(f_open(&file, filename, (FA_OPEN_EXISTING | FA_READ)) != FR_OK) {
				log("no %s", filename);
				return -1;
			}
			if (f_eof(&file) 
				|| f_read(&file, (void *)&header, sizeof(header), &bytesread) != FR_OK
				|| header[0] != daisy::kWavFileChunkId 
				|| header[2] != daisy::kWavFileWaveId) goto badwav;
			// find the format chunk:
			do {
				if (f_eof(&file) || f_read(&file, (void *)&marker, sizeof(marker), &bytesread) != FR_OK) break;
			} while (marker != daisy::kWavFileSubChunk1Id);
			if (f_eof(&file) 
				|| f_read(&file, (void *)&format, sizeof(format), &bytesread) != FR_OK
				|| format.chans == 0 
				|| format.samplerate == 0 
				|| format.bitspersample == 0) goto badwav;
			// find the data chunk:
			do {
				if (f_eof(&file) || f_read(&file, (void *)&marker, sizeof(marker), &bytesread) != FR_OK) break;
			} while (marker != daisy::kWavFileSubChunk2Id);
			bytespersample = format.bytesperframe / format.chans;
			if (f_eof(&file) 
				|| f_read(&file, (void *)&chunksize, sizeof(chunksize), &bytesread) != FR_OK
				|| format.format != 1 
				|| bytespersample < 2 
				|| bytespersample > 4) goto badwav; // only 16/24/32-bit PCM, sorry
			// make sure we read in (multiples of) whole frames
			frames = chunksize / format.bytesperframe;
			frames_per_read = OOPSY_WAV_WORKSPACE_BYTES / format.bytesperframe;
			frames_to_read = frames_per_read;
			total_frames_to_read = buffer_frames;
			// log("b=%u c=%u t=%u", buffer_frames, buffer_channels, total_frames_to_read);
			// log("f=%u c=%u p=%u", frames, format.chans, frames_per_read);
			// log("bp=%u c=%u p=%u", frames_to_read * format.bytesperframe);
			do {
				if (frames_to_read > total_frames_to_read) frames_to_read = total_frames_to_read;
				f_read(&file, workspace, frames_to_read * format.bytesperframe, &bytesread);
				frames_read = bytesread / format.bytesperframe;
				//log("_r=%u t=%u", frames_read, frames_to_read);
				switch (bytespersample) {
					case 2:  { // 16 bit
						for (size_t f=0; f<frames_read; f++) {
							for (size_t c=0; c<buffer_channels; c++) {
								uint8_t * frame = workspace + f*format.bytesperframe + (c % format.chans)*bytespersample;
								buffer[(buffer_index+f)*buffer_channels + c] = ((int16_t *)frame)[0] * 0.000030517578125f;
							}
						}
					} break;
					case 3: { // 24 bit
						for (size_t f=0; f<frames_read; f++) {
							for (size_t c=0; c<buffer_channels; c++) {
								uint8_t * frame = workspace + f*format.bytesperframe + (c % format.chans)*bytespersample;
								int32_t b = (int32_t)(
									((uint32_t)(frame[0]) <<  8) | 
									((uint32_t)(frame[1]) << 16) | 
									((uint32_t)(frame[2]) << 24)
								) >> 8;
								buffer[(buffer_index+f)*buffer_channels + c] = (float)(((double)b) * 0.00000011920928955078125);
							}
						}
					} break;
					case 4: { // 32 bit
						for (size_t f=0; f<frames_read; f++) {
							for (size_t c=0; c<buffer_channels; c++) {
								uint8_t * frame = workspace + f*format.bytesperframe + (c % format.chans)*bytespersample;
								buffer[(buffer_index+f)*buffer_channels + c] = ((int32_t *)frame)[0] / 2147483648.f;
							}
						}
					} break;
				}
				total_frames_to_read -= frames_read;
				buffer_index += frames_read;
			} while (!f_eof(&file) && bytesread > 0 && total_frames_to_read > 0);
			f_close(&file);
			log("read %s", filename);
			return buffer_index;
		badwav:
			f_close(&file);
			log("bad %s", filename);
			return -1;
		}
*/