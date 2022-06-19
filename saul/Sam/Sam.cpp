// # WavPlayer
// ## Description
// Fairly simply sample player.
// Loads 16
//
// Play .wav file from the SD Card.
//
#include <stdio.h>
#include <string.h>
#include "../daisy_saul.h"
//#include "daisy_pod.h"

using namespace daisy;

//DaisyPod  hw;
DaisySaul      hw;
SdmmcHandler   sdh;
FatFSInterface fsi;
FIL            file;
WavPlayer      sampler;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    

    //    if(hw.button1.RisingEdge())
    //    {
    //        sampler.Restart();
    //    }
    //
    //    if(hw.button2.RisingEdge())
    //    {
    //        sampler.SetLooping(!sampler.GetLooping());
    //        //hw.SetLed(DaisyPatch::LED_2_B, sampler.GetLooping());
    //        //dsy_gpio_write(&hw.leds[DaisyPatch::LED_2_B],
    //        //               static_cast<uint8_t>(!sampler.GetLooping()));
    //    }

    for(size_t i = 0; i < size; i += 2)
    {
        out[i] = out[i + 1] = s162f(sampler.Stream()) * 0.5f;
    }
}

void sdcard_init() {
	daisy::SdmmcHandler::Config sdconfig;
	sdconfig.Defaults(); 
	sdconfig.speed           = daisy::SdmmcHandler::Speed::SLOW; 
    sdconfig.width           = daisy::SdmmcHandler::BusWidth::BITS_4;
	sdh.Init(sdconfig);
	fsi.Init(FatFSInterface::Config::MEDIA_SD);
	f_mount(&fsi.GetSDFileSystem(), "/", 1);
	sampler.Init(fsi.GetSDPath());

}


int main(void)
{
    // Init hardware
    size_t blocksize = 4;
    hw.Init();
    sdcard_init();

    sampler.Open(0);
	sampler.SetLooping(true);



    // SET LED to indicate Looping status.
    hw.SetLed(DaisySaul::LED_18, !sampler.GetLooping());

    // Init Audio
    hw.SetAudioBlockSize(blocksize);
    hw.StartAudio(AudioCallback);
    // Loop forever...
    for(;;)
    {
        // Prepare buffers for sampler as needed
        sampler.Prepare();
    }
}
