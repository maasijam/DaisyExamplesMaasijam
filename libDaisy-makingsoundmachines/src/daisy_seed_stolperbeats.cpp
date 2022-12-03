#include "daisy_seed_stolperbeats.h"

extern "C"
{
#include "dev/codec_ak4556.h"
}

using namespace daisy;

#define SEED_LED_PORT DSY_GPIOC
#define SEED_LED_PIN 7

#define SEED_TEST_POINT_PORT DSY_GPIOG
#define SEED_TEST_POINT_PIN 14

const dsy_gpio_pin seedgpio[31] = {
    // GPIO 1-8
    //{DSY_GPIOA, 8}, // removed on Rev4
    {DSY_GPIOB, 12},
    {DSY_GPIOC, 11},
    {DSY_GPIOC, 10},
    {DSY_GPIOC, 9},
    {DSY_GPIOC, 8},
    {DSY_GPIOD, 2},
    {DSY_GPIOC, 12},
    // GPIO 9-16
    {DSY_GPIOG, 10},
    {DSY_GPIOG, 11},
    {DSY_GPIOB, 4},
    {DSY_GPIOB, 5},
    {DSY_GPIOB, 8},
    {DSY_GPIOB, 9},
    {DSY_GPIOB, 6},
    {DSY_GPIOB, 7},
    // GPIO 17-24
    {DSY_GPIOC, 0},
    {DSY_GPIOA, 3},
    {DSY_GPIOB, 1},
    {DSY_GPIOA, 7},
    {DSY_GPIOA, 6},
    {DSY_GPIOC, 1},
    {DSY_GPIOC, 4},
    {DSY_GPIOA, 5},
    // GPIO 17-24
    {DSY_GPIOA, 4},
    {DSY_GPIOA, 1},
    {DSY_GPIOA, 0},
    {DSY_GPIOD, 11},
    {DSY_GPIOG, 9},
    {DSY_GPIOA, 2},
    {DSY_GPIOB, 14},
    {DSY_GPIOB, 15},
};

// Public Initialization

/** Vestigial function body for old function
 *  This is no longer in use.
 */
void DaisySeedStolperbeats::Configure() {}

void DaisySeedStolperbeats::Init(bool boost)
{
    //dsy_system_init();
    System::Config syscfg;
    boost ? syscfg.Boost() : syscfg.Defaults();

    /* auto memory = System::GetProgramMemoryRegion();

    if(memory != System::MemoryRegion::INTERNAL_FLASH)
        syscfg.skip_clocks = true; */

    system.Init(syscfg);

    /* if(memory != System::MemoryRegion::QSPI)
        qspi.Init(qspi_config);

    if(memory == System::MemoryRegion::INTERNAL_FLASH)
    {
        dsy_gpio_init(&led);
        dsy_gpio_init(&testpoint);
        sdram_handle.Init();
    } */

    ConfigureAudio();

    callback_rate_ = AudioSampleRate() / AudioBlockSize();
    // Due to the added 16kB+ of flash usage,
    // and the fact that certain breakouts use
    // both; USB won't be initialized by the
    // SEED file.
    //usb_handle.Init(UsbHandle::FS_INTERNAL);
}

void DaisySeedStolperbeats::DeInit()
{
    // This is intended to be used by the bootloader, but
    // we don't want to reinitialize pretty much anything in the
    // target application, so...
    // qspi.DeInit();
    // sdram_handle.DeInit();
    // dsy_gpio_deinit(&led);
    // dsy_gpio_deinit(&testpoint);

    // dsy_gpio_pin codec_reset_pin;
    // codec_reset_pin = {DSY_GPIOB, 11};
    // // Perhaps a bit unnecessary, but maybe we'll make
    // // this non-static at some point
    // Ak4556::DeInit(codec_reset_pin);
    // audio_handle.DeInit();

    system.DeInit();
}

dsy_gpio_pin DaisySeedStolperbeats::GetPin(uint8_t pin_idx)
{
    dsy_gpio_pin p;
    pin_idx = pin_idx < 32 ? pin_idx : 0;
#ifndef SEED_REV2
    p = seedgpio[pin_idx];
#else
    p = {seed_ports[pin_idx], seed_pins[pin_idx]};
#endif
    return p;
}

void DaisySeedStolperbeats::DelayMs(size_t del)
{
    system.Delay(del);
}

void DaisySeedStolperbeats::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    audio_handle.Start(cb);
}

void DaisySeedStolperbeats::StartAudio(AudioHandle::AudioCallback cb)
{
    audio_handle.Start(cb);
}

void DaisySeedStolperbeats::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    audio_handle.ChangeCallback(cb);
}

void DaisySeedStolperbeats::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    audio_handle.ChangeCallback(cb);
}

void DaisySeedStolperbeats::StopAudio()
{
    audio_handle.Stop();
}

void DaisySeedStolperbeats::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    audio_handle.SetSampleRate(samplerate);
    callback_rate_ = AudioSampleRate() / AudioBlockSize();
}

float DaisySeedStolperbeats::AudioSampleRate()
{
    return audio_handle.GetSampleRate();
}

void DaisySeedStolperbeats::SetAudioBlockSize(size_t blocksize)
{
    audio_handle.SetBlockSize(blocksize);
    callback_rate_ = AudioSampleRate() / AudioBlockSize();
}

size_t DaisySeedStolperbeats::AudioBlockSize()
{
    return audio_handle.GetConfig().blocksize;
}

float DaisySeedStolperbeats::AudioCallbackRate() const
{
    return callback_rate_;
}

void DaisySeedStolperbeats::SetLed(bool state)
{
    dsy_gpio_write(&led, state);
}

void DaisySeedStolperbeats::SetTestPoint(bool state)
{
    dsy_gpio_write(&testpoint, state);
}

// Private Implementation

void DaisySeedStolperbeats::ConfigureQspi()
{
    qspi_config.device = QSPIHandle::Config::Device::IS25LP064A;
    qspi_config.mode   = QSPIHandle::Config::Mode::MEMORY_MAPPED;

    qspi_config.pin_config.io0 = dsy_pin(DSY_GPIOF, 8);
    qspi_config.pin_config.io1 = dsy_pin(DSY_GPIOF, 9);
    qspi_config.pin_config.io2 = dsy_pin(DSY_GPIOF, 7);
    qspi_config.pin_config.io3 = dsy_pin(DSY_GPIOF, 6);
    qspi_config.pin_config.clk = dsy_pin(DSY_GPIOF, 10);
    qspi_config.pin_config.ncs = dsy_pin(DSY_GPIOG, 6);
}
void DaisySeedStolperbeats::ConfigureAudio()
{
    // SAI1 -- Peripheral
    // Configure
    SaiHandle::Config sai_config;
    sai_config.periph          = SaiHandle::Config::Peripheral::SAI_1;
    sai_config.sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config.bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config.a_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config.b_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config.pin_config.fs   = {DSY_GPIOE, 4};
    sai_config.pin_config.mclk = {DSY_GPIOE, 2};
    sai_config.pin_config.sck  = {DSY_GPIOE, 5};

    // Device-based Init
    switch(CheckBoardVersion())
    {
        case BoardVersion::DAISY_SEED_1_1:
        {
            // Data Line Directions
            sai_config.a_dir         = SaiHandle::Config::Direction::RECEIVE;
            sai_config.pin_config.sa = {DSY_GPIOE, 6};
            sai_config.b_dir         = SaiHandle::Config::Direction::TRANSMIT;
            sai_config.pin_config.sb = {DSY_GPIOE, 3};
            I2CHandle::Config i2c_config;
            i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
            i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_2;
            i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
            i2c_config.pin_config.scl = {DSY_GPIOH, 4};
            i2c_config.pin_config.sda = {DSY_GPIOB, 11};
            I2CHandle i2c_handle;
            i2c_handle.Init(i2c_config);
            Wm8731::Config codec_cfg;
            codec_cfg.Defaults();
            Wm8731 codec;
            codec.Init(codec_cfg, i2c_handle);
        }
        break;
        case BoardVersion::DAISY_SEED:
        default:
        {
            // Data Line Directions
            sai_config.a_dir         = SaiHandle::Config::Direction::TRANSMIT;
            sai_config.pin_config.sa = {DSY_GPIOE, 6};
            sai_config.b_dir         = SaiHandle::Config::Direction::RECEIVE;
            sai_config.pin_config.sb = {DSY_GPIOE, 3};
            dsy_gpio_pin codec_reset_pin;
            codec_reset_pin = {DSY_GPIOB, 11};
            Ak4556::Init(codec_reset_pin);
        }
        break;
    }

    // Then Initialize
    SaiHandle sai_1_handle;
    sai_1_handle.Init(sai_config);

    // Audio
    AudioHandle::Config audio_config;
    audio_config.blocksize  = 48;
    audio_config.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    audio_config.postgain   = 1.f;
    audio_handle.Init(audio_config, sai_1_handle);
}
void DaisySeedStolperbeats::ConfigureDac()
{
    // This would be the equivalent initialization as previously existed.
    // However, not all platforms have the DAC, and many use those pins
    // for other things.
    //    DacHandle::Config cfg;
    //    cfg.bitdepth   = DacHandle::Config::BitDepth::BITS_12;
    //    cfg.buff_state = DacHandle::Config::BufferState::ENABLED;
    //    cfg.mode       = DacHandle::Config::Mode::POLLING;
    //    cfg.chn        = DacHandle::Config::Channel::BOTH;
    //    dac.Init(cfg);
}

DaisySeedStolperbeats::BoardVersion DaisySeedStolperbeats::CheckBoardVersion()
{
    /** Version Checks:
     *  * Fall through is Daisy Seed v1 (aka Daisy Seed rev4)
     *  * PD3 tied to gnd is Daisy Seed v1.1 (aka Daisy Seed rev5)
     *  * PD4 tied to gnd reserved for future hardware
     */
        return BoardVersion::DAISY_SEED;
}

