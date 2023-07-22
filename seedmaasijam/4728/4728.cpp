#include "daisy_seed.h"
#include "daisysp.h"


using namespace daisy; 
using namespace daisysp; 

// uint8_t output_buffer[8];
#define BUFF_SIZE 8
static uint8_t DMA_BUFFER_MEM_SECTION output_buffer[BUFF_SIZE];



DaisySeed hw;

enum dacChannel {
    ch1,
    ch2,
    ch3,
    ch4
};

I2CHandle::Config _i2c_config;
I2CHandle _i2c;

void setDacChannelValue(dacChannel channel,uint16_t channel_value);

struct lfoStruct
{
    Oscillator osc;
    Parameter  freqCtrl;
    Parameter  ampCtrl;
    float      amp;
    float      freq;
    int        waveform;
    float      value;

    void Init(float samplerate, float freq, int wf, AnalogControl freqKnob)
    {
        osc.Init(samplerate);
        osc.SetAmp(1);
        osc.SetFreq(freq);
        osc.SetWaveform(wf);
        freqCtrl.Init(freqKnob, .1, 35, Parameter::LOGARITHMIC);
        //ampCtrl.Init(ampKnob, 0, 1, Parameter::LINEAR);
    }

    void Process(dacChannel chn)
    {
        //read the knobs and set params
        osc.SetFreq(freqCtrl.Process());
        //osc.SetWaveform(waveform);

        //write to the DAC
        setDacChannelValue(
            chn,
            uint16_t((osc.Process() + 1.f) * .5f * 1 * 4095.f));
    }
};

lfoStruct lfos[4];
    



static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        lfos[0].Process(dacChannel::ch1);
        lfos[1].Process(dacChannel::ch2);
        lfos[2].Process(dacChannel::ch3);
        lfos[3].Process(dacChannel::ch4);
    }
}


int main(void)
{
    hw.Configure();
    hw.Init();
    hw.StartLog(false);
    System::Delay(1000);
    float samplerate = hw.AudioSampleRate();

     lfos[0].Init(samplerate, 0.1f,Oscillator::WAVE_TRI);
     lfos[1].Init(samplerate, 0.85f,Oscillator::WAVE_POLYBLEP_SQUARE);
     lfos[2].Init(samplerate, 0.25f,Oscillator::WAVE_RAMP);
     lfos[3].Init(samplerate, 2.f,Oscillator::WAVE_POLYBLEP_SQUARE);
    

    _i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1;
    _i2c_config.speed  = I2CHandle::Config::Speed::I2C_400KHZ;
    _i2c_config.mode   = I2CHandle::Config::Mode::I2C_MASTER;
    _i2c_config.pin_config.scl  = {DSY_GPIOB, 8};
    _i2c_config.pin_config.sda  = {DSY_GPIOB, 9};
    
    // initialise the peripheral
    
    _i2c.Init(_i2c_config);

    
/*
    int      nDevices = 0;
    for(unsigned char address = 1; address < 127; address++)
    {
        uint8_t           testData = 0;
        I2CHandle::Result i2cResult
            = _i2c.TransmitBlocking(address, &testData, 1, 500);

        if(i2cResult == I2CHandle::Result::OK)
        {
            int prAddress = (address < 16) ? 0 : address;
            hw.PrintLine("I2C device found at address %x !", prAddress);
            nDevices++;
        }
    }
    if(nDevices == 0)
        hw.PrintLine("No I2C devices found");
    else
        hw.PrintLine("done");
*/
    
    hw.StartAudio(AudioCallback);

    while(1) {  
       

        //System::Delay(500);

        //uint16_t channel_a_value {2000};
        //uint16_t channel_b_value {2500};
        //uint16_t channel_c_value {3000};
        //uint16_t channel_d_value {4095};
        //setDacChannelValue(ch1,4000);
        //setDacChannelValue(ch4,2500);
        //for (int i = 0; i <= 4095; i+=2){ 
        //    setDacChannelValue(ch2,i); 
        //    System::Delay(1);
        //}

        //output_buffer[0] = static_cast<uint8_t>(channel_a_value >> 8);
        //output_buffer[1] = static_cast<uint8_t>(channel_a_value & 0xFF);
        //output_buffer[2] = static_cast<uint8_t>(channel_b_value >> 8);
        //output_buffer[3] = static_cast<uint8_t>(channel_b_value & 0xFF);
        //output_buffer[4] = static_cast<uint8_t>(channel_c_value >> 8);
        //output_buffer[5] = static_cast<uint8_t>(channel_c_value & 0xFF);
        //output_buffer[6] = static_cast<uint8_t>(channel_d_value >> 8);
        //output_buffer[7] = static_cast<uint8_t>(channel_d_value & 0xFF);
        //I2CHandle::Result i2cResult= _i2c.TransmitDma(0x60, &output_buffer[0], 8, NULL, NULL);
        //if(i2cResult == I2CHandle::Result::OK) {
        //    hw.PrintLine("OK TRANSMISSION 1");
        //}
       /* I2CHandle::Result i2cResult_2= _i2c.TransmitDma(0x60, &output_buffer[0], 8, NULL, NULL);
        if(i2cResult_2 == I2CHandle::Result::OK) {
            hw.PrintLine("OK TRANSMISSION 2");
        }*/

        //System::Delay(500);

        //uint16_t channel_a_value_2 {0};
        //uint16_t channel_b_value_2 {0};
        //uint16_t channel_c_value_2 {0};
        //uint16_t channel_d_value_2 {0};
        //setDacChannelValue(ch1,0);
        //setDacChannelValue(ch4,0);
        //for (int i = 4095; i > 0 ; i-=2){ 
        //    setDacChannelValue(ch2,i); 
        //    System::Delay(1);
        //}

        //output_buffer[0] = static_cast<uint8_t>(channel_a_value_2 >> 8);
        //output_buffer[1] = static_cast<uint8_t>(channel_a_value_2 & 0xFF);
        //output_buffer[2] = static_cast<uint8_t>(channel_b_value_2 >> 8);
        //output_buffer[3] = static_cast<uint8_t>(channel_b_value_2 & 0xFF);
        //output_buffer[4] = static_cast<uint8_t>(channel_c_value_2 >> 8);
        //output_buffer[5] = static_cast<uint8_t>(channel_c_value_2 & 0xFF);
        //output_buffer[6] = static_cast<uint8_t>(channel_d_value_2 >> 8);
        //output_buffer[7] = static_cast<uint8_t>(channel_d_value_2 & 0xFF);
        //I2CHandle::Result i2cResult_3= _i2c.TransmitDma(0x60, &output_buffer[0], 8, NULL, NULL);
        //if(i2cResult_3 == I2CHandle::Result::OK) {
        //    hw.PrintLine("OK TRANSMISSION 3");
        //}
        /*I2CHandle::Result i2cResult_4= _i2c.TransmitDma(0x60, &output_buffer[0], 8, NULL, NULL);
        if(i2cResult_4 == I2CHandle::Result::OK) {
            hw.PrintLine("OK TRANSMISSION 4");
        }*/
        
        
    }
}

void setDacChannelValue(
    dacChannel channel,
    uint16_t channel_value
)
{
        int buffIdx = 0;
        switch (channel)
        {
        case ch1:
            buffIdx = channel;
            break;
        case ch2:
            buffIdx = channel+1;
            break;
        case ch3:
            buffIdx = channel+2;
            break;
        case ch4:
            buffIdx = channel+3;
            break;
        default:
            buffIdx = 0;
            break;
        }
        
        output_buffer[buffIdx] = static_cast<uint8_t>(channel_value >> 8);
        output_buffer[buffIdx+1] = static_cast<uint8_t>(channel_value & 0xFF);

        I2CHandle::Result i2cResult= _i2c.TransmitDma(0x60, &output_buffer[0], 8, NULL, NULL);
        if(i2cResult == I2CHandle::Result::OK) {
            hw.PrintLine("OK TRANSMISSION");
        }
}