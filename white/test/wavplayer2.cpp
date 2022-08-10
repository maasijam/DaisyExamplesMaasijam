#include <cstring>
#include "wavplayer2.h"
#include "daisy_seed.h"

using namespace daisy;

int WavPlayer2::Init(const char *search_path,
                    int32_t    *buffer,
                    size_t      bufferSize,
                    size_t      numChannels)
{
    buff_        = buffer;
    bufferSize_  = bufferSize / numChannels;
    numChannels_ = numChannels;

    // First check for all .wav files, and add them to the list until its full or there are no more.
    // Only checks '/'
    FRESULT result = FR_OK;
    FILINFO fno;
    DIR     dir;
    char   *fn;
    file_sel_ = 0;
    file_cnt_ = 0;
    playing_  = true;
    looping_  = false;

    // Open Dir and scan for files.
    result = f_opendir(&dir, search_path);
    if(result != FR_OK)
    {
        return result;
    }

    do
    {
        result = f_readdir(&dir, &fno);
        // Exit if bad read or NULL fname
        if(result != FR_OK || fno.fname[0] == 0)
            break;
        // Skip if its a directory or a hidden file.
        if(fno.fattrib & (AM_HID | AM_DIR))
            continue;
        // Now we'll check if its .wav and add to the list.
        fn = fno.fname;
        if(file_cnt_ < kMaxFiles - 1)
        {
            if(strstr(fn, ".wav") || strstr(fn, ".WAV"))
            {
                strcpy(file_info_[file_cnt_].name, search_path);
                strcat(file_info_[file_cnt_].name, fn);
                file_cnt_++;
                // For now lets break anyway to test.
                //                break;
            }
        }
        else
        {
            break;
        }
    } while(result == FR_OK);
    f_closedir(&dir);
    // Now we'll go through each file and load the WavInfo.
    for(size_t i = 0; i < file_cnt_; i++)
    {
        size_t bytesread;
        if(f_open(&fil_, file_info_[i].name, (FA_OPEN_EXISTING | FA_READ))
           == FR_OK)
        {
            // Populate the WAV Info
            result = f_read(&fil_,
                            (void *)&file_info_[i].raw_data,
                            sizeof(WAV_FormatTypeDef),
                            &bytesread);
            if(result != FR_OK)
            {
                // Maybe add return type
                return result;
            }
            f_close(&fil_);
        }
    }

    // fill buffer with first file preemptively.
    buff_state_ = BUFFER_STATE_PREPARE_0;
    Open((size_t)0);
    // seek past .WAV header bytes
    Restart();
    // load 1st half of buffer
    Prepare();
    read_ptr_ = 0;

    return result;
}


int WavPlayer2::Open(size_t sel)
{
    if(sel != file_sel_)
    {
        f_close(&fil_);
        file_sel_ = sel < file_cnt_ ? sel : file_cnt_ - 1;
    }

    // Set Buffer Position
    return f_open(
        &fil_, file_info_[file_sel_].name, (FA_OPEN_EXISTING | FA_READ));
}

int WavPlayer2::Open(const char *filename)
{
    if(strcmp(filename, file_info_[file_sel_].name) != 0)
    {
        f_close(&fil_);
        file_sel_ = -1;
    }

    return f_open(&fil_, filename, (FA_OPEN_EXISTING | FA_READ));
}

int WavPlayer2::Close()
{
    return f_close(&fil_);
}

int32_t WavPlayer2::Stream()
{
    int16_t samp;
    if(playing_)
    {
        samp = buff_[read_ptr_];
        // Increment rpo
        read_ptr_ = (read_ptr_ + 1) % (bufferSize_ * numChannels_);
        if(read_ptr_ == 0)
            buff_state_ = BUFFER_STATE_PREPARE_1;
        else if(read_ptr_ == bufferSize_ * numChannels_ / 2)
            buff_state_ = BUFFER_STATE_PREPARE_0;
    }
    else
    {
        samp = 0;
        if(looping_)
            playing_ = true;
    }
    return samp;
}

int32_t *WavPlayer2::StreamStereo()
{
    int32_t samps[2] = {Stream(), Stream()};
    return samps;
}

int WavPlayer2::Prepare()
{
    FRESULT readres = FR_OK;

    if(buff_state_ != BUFFER_STATE_IDLE)
    {
        size_t offset, bytesread, rxsize;
        bytesread = 0;
        rxsize    = bufferSize_ * numChannels_ * sizeof(buff_[0]) / 2;
        offset    = buff_state_ == BUFFER_STATE_PREPARE_1
                        ? bufferSize_ * numChannels_ / 2
                        : 0;

        readres = f_read(&fil_, &buff_[offset], rxsize, &bytesread);

        if(bytesread < rxsize || f_eof(&fil_))
        {
            if(looping_)
            {
                Restart();
                readres = f_read(&fil_,
                                 &buff_[offset + (bytesread / 2)],
                                 rxsize - bytesread,
                                 &bytesread);
            }
            else
            {
                playing_ = false;
            }
        }
        buff_state_ = BUFFER_STATE_IDLE;
    }

    return readres;
}

void WavPlayer2::Restart()
{
    playing_ = true;
    f_lseek(&fil_,
            sizeof(WAV_FormatTypeDef)
                + file_info_[file_sel_].raw_data.SubChunk1Size);
}

WavPlayer2::BufferState WavPlayer2::GetNextBuffState()
{
    size_t next_samp;
    next_samp = (read_ptr_ + 1) % bufferSize_;
    if(next_samp < bufferSize_ / 2)
    {
        return BUFFER_STATE_PREPARE_1;
    }
    else
    {
        return BUFFER_STATE_PREPARE_0;
    }
}
