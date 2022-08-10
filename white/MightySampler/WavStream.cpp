#include <string.h>
#include "WavStream.h"
#include "daisy.h"




DSY_SDRAM_BSS int16_t bigBuff[44100*120];

using namespace daisy;

FIL            file;
FatFSInterface fsi;

WavStream::WavStream() {
    
}

void WavStream::Init()
{
    // First check for all .wav files, and add them to the list until its full or there are no more.
    // Only checks '/'
    FRESULT result = FR_OK;
    FILINFO fno;
    DIR     dir;
    char *  fn;

    // Init Fatfs
    //dsy_fatfs_init();
    // Mount SD Card

    //display->Write("Loading SD ...");
   
    fn  = "genent.wav";

    fsi.Init(FatFSInterface::Config::MEDIA_SD);
    
	//f_mount(&fsi.GetSDFileSystem(), "/", 0);

    // Open Dir and scan for files.
    if(f_mount(&fsi.GetSDFileSystem(), "/", 1) != FR_OK)
    //if(f_opendir(&dir, "/") != FR_OK)
    {
        //display->Write("Cannot open Dir ...");
        return;
    }
    do
    {
        result = f_readdir(&dir, &fno);
        if(result != FR_OK || fno.fname[0] == 0)
            //break;
        // Skip if its a directory or a hidden file.
        if(fno.fattrib & (AM_HID | AM_DIR))
            continue;

        
        //f_open(&file, name, (FA_OPEN_EXISTING | FA_READ));
        f_open(&file, fn, (FA_OPEN_EXISTING | FA_READ));
            
    } while(result == FR_OK);
    f_closedir(&dir);

    // Now we'll go through each file and load the WavInfo.
    for(size_t i = 0; i < SPLR_COUNT; i++)
    {
       

        size_t bytesread;
        if(f_open(&file, fn, (FA_OPEN_EXISTING | FA_READ))
           == FR_OK)
        {
            strcpy(sample[i].fileInfo.name, fn);

            // Populate the WAV Info
            if(f_read(&file,
                      (void *)&sample[i].fileInfo.raw_data,
                      sizeof(WAV_FormatTypeDef),
                      &bytesread)
               != FR_OK)
            {
                // Maybe add return type
                return;
            }
            
            f_close(&file);

            Open(i);
        }

        
    }
    
    
    

    //display->Write("OK");
}

int WavStream::Open(size_t sel)
{
    

    f_open(&file, sample[sel].fileInfo.name, (FA_OPEN_EXISTING | FA_READ));
    
    f_lseek(&file,
            sizeof(WAV_FormatTypeDef)
                + sample[sel].fileInfo.raw_data.SubChunk1Size);

    UINT bytesread = 0;
    size_t fileSize = 0;

    size_t chanCount = sample[sel].fileInfo.raw_data.NbrChannels;
    size_t sampleRate = sample[sel].fileInfo.raw_data.SampleRate;

    while(f_eof(&file) == 0) {
        UINT sizeToRead = 44100 * 2 * sizeof(bigBuff[0]);
        f_read(&file, &bigBuff[fileSize + readHead], sizeToRead, &bytesread);
        fileSize += bytesread / 2;
    }

    sample[sel].sampleData = &bigBuff[readHead];

    readHead += fileSize;

    fileSize = fileSize / chanCount;
    
    sample[sel].sampleSize = fileSize;
    sample[sel].chanCount = chanCount;
    sample[sel].sampleRate = (double)sampleRate;
    sample[sel].Reset();

    f_close(&file);

    return 0;
}

int WavStream::Close()
{
    return f_close(&file);
}

void WavStream::Stream(double speed)
{
    data[0] = 0;
    data[1] = 0;
    
    for (size_t sampler = 0; sampler < SPLR_COUNT; sampler++) {

        sample[sampler].Stream(speed);

        for (size_t channel = 0; channel < 2; channel++) {        
            data[channel] += sample[sampler].data[channel];
        }
    }
    
}



