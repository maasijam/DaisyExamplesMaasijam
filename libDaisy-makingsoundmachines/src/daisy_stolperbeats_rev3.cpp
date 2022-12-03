
#include "daisy_stolperbeats_rev3.h"
#include "dev/codec_ak4556.h"

using namespace daisy;

/** MSM modified for use with Heart of Gold */

// Hardware Definitions

#define PIN_MIDI_OUT 13 // HoG MIDI Out PB6
#define PIN_MIDI_IN 14 // HoG MIDI In PB7

#define PIN_BUTTON_1 54 // PG13
#define PIN_BUTTON_2 55 // PH7
#define PIN_BUTTON_3 56 // PI8
#define PIN_BUTTON_4 57 // PI11

#define PIN_SAI_SCK_A 28 // HoG SAI2_SCK_B PA2
#define PIN_SAI2_FS_A 27 // HoG SAI2_FS_B PG9
#define PIN_SAI2_SD_A 26 // HoG SAI2_SD_A PD 11
#define PIN_SAI2_SD_B 25 // HoG SAI2_SD_B PA0
#define PIN_SAI2_MCLK 24 // HoG SAI2_MCLK_B PA1

#define PIN_AK4556_RESET 29 // HoG PB14

#define PIN_SR_DATA 51 // PD3
#define PIN_SR_CLOCK 52 // PD5
#define PIN_SR_LATCH 53 // PD4

#define PIN_DAC8568_SYNC 9 //PB4

void DaisyStolperbeatsRev3::Init(bool boost)
{
    // Configure Seed first
    seed.Configure();
    seed.Init(boost);

    InitTimer();

    // dirty - clean this up
    // seed.sdram_handle.state = DSY_SDRAM_STATE_DISABLE;
    // _SDRAM_MspDeInit();
    
    InitAudio();    

    InitCvOutputs();
    InitEncoder();
    InitGates();
    InitButtons();
    InitSR595();

    InitMidi();
    InitControls();
}

void DaisyStolperbeatsRev3::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyStolperbeatsRev3::SetHidUpdateRates()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].SetSampleRate(AudioCallbackRate());
    }
    encoder[ENC_1].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_2].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_3].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_4].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_5].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_6].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_7].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_8].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_9].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_10].SetUpdateRate(AudioCallbackRate());


    button[BUTTON_1].SetUpdateRate(AudioCallbackRate());
    button[BUTTON_2].SetUpdateRate(AudioCallbackRate());
    button[BUTTON_3].SetUpdateRate(AudioCallbackRate());
    button[BUTTON_4].SetUpdateRate(AudioCallbackRate());
    button[BUTTON_5].SetUpdateRate(AudioCallbackRate());
    button[BUTTON_6].SetUpdateRate(AudioCallbackRate());
    button[BUTTON_7].SetUpdateRate(AudioCallbackRate());
    button[BUTTON_8].SetUpdateRate(AudioCallbackRate());
}


void DaisyStolperbeatsRev3::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyStolperbeatsRev3::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyStolperbeatsRev3::StopAudio()
{
    seed.StopAudio();
}

void DaisyStolperbeatsRev3::SetAudioSampleRate(
    SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisyStolperbeatsRev3::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisyStolperbeatsRev3::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyStolperbeatsRev3::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisyStolperbeatsRev3::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyStolperbeatsRev3::StartAdc()
{
    seed.adc.Start();
}

/** Stops Transfering data from the ADC */
void DaisyStolperbeatsRev3::StopAdc()
{
    seed.adc.Stop();
}

//ex DaisyStolperbeatsRev3::UpdateAnalogControls()
void DaisyStolperbeatsRev3::ProcessAnalogControls()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Process();
    }
}

//ex  DaisyStolperbeatsRev3::GetCtrlValue(Ctrl k)
float DaisyStolperbeatsRev3::GetKnobValue(Ctrl k)
{
    return (controls[k].Value());
}

//ex DaisyStolperbeatsRev3::DebounceControls()
void DaisyStolperbeatsRev3::ProcessDigitalControls()
{
    encoder[ENC_1].Debounce16();
    encoder[ENC_2].Debounce16();
    encoder[ENC_3].Debounce16();
    encoder[ENC_4].Debounce16();
    encoder[ENC_5].Debounce16();
    encoder[ENC_6].Debounce16();
    encoder[ENC_7].Debounce16();
    encoder[ENC_8].Debounce16();
    encoder[ENC_9].Debounce16();
    encoder[ENC_10].Debounce16();


    button[BUTTON_1].Debounce16();
    button[BUTTON_2].Debounce16();
    button[BUTTON_3].Debounce16();
    button[BUTTON_4].Debounce16();
    button[BUTTON_5].Debounce16();
    button[BUTTON_6].Debounce16();
    button[BUTTON_7].Debounce16();
    button[BUTTON_8].Debounce16();
}


void DaisyStolperbeatsRev3::InitTimer()
{
    
        
    // Configure and start highspeed timer.
    // TIM 5 is a 32-bit counter on APB1 Clock (RM0433 Rev7 - pg 458)
    // TIM 5 counter UP (defaults to fastest tick/longest period).
    TimerHandle::Config tim5_cfg;
    tim5_cfg.periph = TimerHandle::Config::Peripheral::TIM_5;
    tim5_cfg.dir    = TimerHandle::Config::CounterDir::UP;
    tim5_.Init(tim5_cfg);
    tim5_.Start();

    // Sets the period of the Timer.
    // This is the number of ticks it takes before it wraps back around.
    // For self-managed timing, this can be left at the default. (0xffff for 16-bit
    // and 0xffffffff for 32-bit timers). 
    // This can be changed "on-the-fly" 
     
    //uint32_t _ticks;
    //tim5_.SetPeriod(_ticks);

    // Sets the Prescalar applied to the TIM peripheral. 
    // This can be any number up to 0xffff 
    // This will adjust the rate of ticks:
    // Calculated as APBN_Freq / prescalar per tick
    // where APBN is APB1 for Most general purpose timers,
    // and APB2 for HRTIM,a nd the advanced timers. 
    // This can be changed "on-the-fly" 

    //uint32_t _val = 0xBB7F; // 47.999 - 480 MHz / 48000 = 10 kHz -> 1 tick every 0.0001 sec
    //tim5_.SetPrescaler(_val);
}


void DaisyStolperbeatsRev3::InitLEDMatrix()
{
    /* static constexpr I2CHandle::Config i2c_config = {
        I2CHandle::Config::Peripheral::I2C_1,
        {{DSY_GPIOB, 8},
         {DSY_GPIOB, 9}}, 
        I2CHandle::Config::Speed::I2C_400KHZ}; */

    I2CHandle::Config i2c_config;

    i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_config.pin_config.scl = {DSY_GPIOB, 8},
    i2c_config.pin_config.sda = {DSY_GPIOB, 9};
    i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;

    uint8_t addr[2] = { 0b01110100, 0b01110101 }; // 0x74, 0x75

    I2CHandle i2c;
    i2c.Init(i2c_config);

    // init with i2c handle, array of adresses, number of driver chips
    ledmatrix.Init(i2c, addr, 2);
}


static LedDriverIs31fl3731<2, true>::DmaBuffer DMA_BUFFER_MEM_SECTION
    hog_led_dma_buffer_a,
    hog_led_dma_buffer_b; 

void DaisyStolperbeatsRev3::InitLEDMatrixDMA()
{

    I2CHandle::Config i2c_config;

    i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_config.pin_config.scl = {DSY_GPIOB, 8},
    i2c_config.pin_config.sda = {DSY_GPIOB, 9};
    i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;

    uint8_t addr[2] = { 0b01110100, 0b01110101 }; // 0x74, 0x75

    I2CHandle i2c;
    i2c.Init(i2c_config);

    // LEDs
    // 2x PCA9685 addresses 0x00, and 0x02
    ledmatrix_dma.Init(i2c, addr, hog_led_dma_buffer_a, hog_led_dma_buffer_b);

}


// Private Function Implementations
// set SAI2 stuff -- run this between seed configure and init
void DaisyStolperbeatsRev3::InitAudio()
{
    // Handle Seed Audio as-is and then
    // SaiHandle::Config sai_config;
    // Internal Codec
    /* sai_config.periph          = SaiHandle::Config::Peripheral::SAI_1;
    sai_config.sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config.bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config.a_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config.b_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config.a_dir           = SaiHandle::Config::Direction::TRANSMIT;
    sai_config.b_dir           = SaiHandle::Config::Direction::RECEIVE;
    sai_config.pin_config.fs   = {DSY_GPIOE, 4};
    sai_config.pin_config.mclk = {DSY_GPIOE, 2};
    sai_config.pin_config.sck  = {DSY_GPIOE, 5};
    sai_config.pin_config.sa   = {DSY_GPIOE, 6};
    sai_config.pin_config.sb   = {DSY_GPIOE, 3};

    SaiHandle sai_1_handle;
    sai_1_handle.Init(sai_config); */

    // Reset Pin for AK4556
    // Built-in AK4556 was reset during Seed Init
    /* dsy_gpio_pin codec_reset_pin = seed.GetPin(29);
    Ak4556::Init(codec_reset_pin);  */

    // Audio
    /* AudioHandle::Config audio_config;
    audio_config.blocksize  = 48;
    audio_config.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    audio_config.postgain   = 1.f;
    seed.audio_handle.Init(audio_config, sai_1_handle); */
}

void DaisyStolperbeatsRev3::InitControls()
{
    AdcChannelConfig cfg[CTRL_LAST];

    // Init ADC channels with Pins
    cfg[CTRL_1].InitSingle({DSY_GPIOC, 4});  // CV IN KICK - ADC1_INP4
    cfg[CTRL_2].InitSingle({DSY_GPIOC, 1});  // CV IN SNARE - ADC1_INP11
    cfg[CTRL_3].InitSingle({DSY_GPIOA, 6});  // CV IN HIHAT 1 - ADC1_INP3
    cfg[CTRL_4].InitSingle({DSY_GPIOA, 7});  // CV IN HIHAT 2 - ADC1_INP7
    cfg[CTRL_5].InitSingle({DSY_GPIOB, 1});  // CV IN PERC 1 - ADC1_INP5
    cfg[CTRL_6].InitSingle({DSY_GPIOA, 3});  // CV IN PERC 2 - ADC1_INP15
    cfg[CTRL_7].InitSingle({DSY_GPIOC, 0});  // CV IN SHUFFLE - ADC1_INP10
    cfg[CTRL_8].InitSingle({DSY_GPIOB, 0});  // CV IN BANK - ADC12_INP9

    // Initialize ADC
    seed.adc.Init(cfg, CTRL_LAST);

    // Initialize AnalogControls, with flip set to true
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Init(seed.adc.GetPtr(i), AudioCallbackRate(), true);
    }
}

void DaisyStolperbeatsRev3::InitMidi()
{
    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);
}

void DaisyStolperbeatsRev3::InitCvOutputs()
{
    DacHandle::Config cfg;
    cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    cfg.buff_state = DacHandle::BufferState::ENABLED;
    cfg.mode       = DacHandle::Mode::POLLING;
    cfg.chn        = DacHandle::Channel::BOTH;
    seed.dac.Init(cfg);
    seed.dac.WriteValue(DacHandle::Channel::BOTH, 0);
}

void DaisyStolperbeatsRev3::InitEncoder()
{

    // A, B. CLICK
    encoder[ENC_1].Init({DSY_GPIOB,  5}, {DSY_GPIOD, 11},  {DSY_GPIOC, 15}, AudioCallbackRate()); // ENCODER TEMPO
    encoder[ENC_2].Init({DSY_GPIOA, 10}, {DSY_GPIOD, 12},  {DSY_GPIOE,  1}, AudioCallbackRate()); // ENCODER KICK
    encoder[ENC_3].Init({DSY_GPIOD, 14}, {DSY_GPIOC, 12},  {DSY_GPIOD,  5}, AudioCallbackRate()); // ENCODER SNARE
    encoder[ENC_4].Init({DSY_GPIOB, 13}, {DSY_GPIOD,  8},  {DSY_GPIOE, 13}, AudioCallbackRate()); // ENCODER HIHAT
    encoder[ENC_5].Init({DSY_GPIOB, 12}, {DSY_GPIOB, 15},  {DSY_GPIOE, 14}, AudioCallbackRate()); // ENCODER HIHAT 2
    encoder[ENC_6].Init({DSY_GPIOC, 11}, {DSY_GPIOE,  0},  {DSY_GPIOD,  6}, AudioCallbackRate()); // ENCODER SHUFFLE
    encoder[ENC_7].Init({DSY_GPIOA,  8}, {DSY_GPIOD,  1},  {DSY_GPIOD,  9}, AudioCallbackRate()); // ENCODER PERC 1
    encoder[ENC_8].Init({DSY_GPIOC,  7}, {DSY_GPIOD,  7},  {DSY_GPIOD, 10}, AudioCallbackRate()); // ENCODER PERC 2
    encoder[ENC_9].Init({DSY_GPIOE,  9}, {DSY_GPIOE, 11},  {DSY_GPIOE,  7}, AudioCallbackRate()); // ENCODER SUBDIV
    encoder[ENC_10].Init({DSY_GPIOE, 10}, {DSY_GPIOE, 12},  {DSY_GPIOE, 8}, AudioCallbackRate()); // ENCODER SYNC
}

void DaisyStolperbeatsRev3::InitSR595()
{
    num_sr_chained = 3;

    sr_pin_cfg[SR_LATCH] = {DSY_GPIOD, 3}; //seed.GetPin(PIN_SR_LATCH);
    sr_pin_cfg[SR_CLOCK] = {DSY_GPIOD, 4}; //seed.GetPin(PIN_SR_CLOCK);
    sr_pin_cfg[SR_DATA]  = {DSY_GPIOD, 2}; //seed.GetPin(PIN_SR_DATA);

    sr_595.Init(sr_pin_cfg, num_sr_chained);
}

void DaisyStolperbeatsRev3::InitButtons()
{
    // Buttons
    // dsy_gpio_pin pin;
    // float _AudioCallbackRate = AudioCallbackRate();

    // BUTTON TAP;
    button[BUTTON_1].Init({DSY_GPIOC, 3}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON LINEAR
    button[BUTTON_2].Init({DSY_GPIOD, 0}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON BANK DEC
    button[BUTTON_3].Init({DSY_GPIOC, 13}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON BANK INC
    button[BUTTON_4].Init({DSY_GPIOC, 14}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON SHUFFLE DILLA
    button[BUTTON_5].Init({DSY_GPIOC, 10}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON SHUFFLE SHAKE
    button[BUTTON_6].Init({DSY_GPIOC, 8}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON SHUFFLE PUSH
    button[BUTTON_7].Init({DSY_GPIOC, 2}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON SHUFFLE CLAVE
    button[BUTTON_8].Init({DSY_GPIOD, 13}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);

}

void DaisyStolperbeatsRev3::InitGates()
{
    // LED Linear Drumming
    led_linear.pin = {DSY_GPIOB, 4}; // LED LINEAR DRUMMING
    led_linear.mode = DSY_GPIO_MODE_OUTPUT_PP;
    led_linear.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&led_linear);

    // Gate Output
    gate_output[GATE_OUT_1].pin = {DSY_GPIOA, 0}; // TRIGGER OUT KICK
    gate_output[GATE_OUT_1].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_1].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_1]);
    //dsy_gpio_write(&gate_output[0], 1);

    gate_output[GATE_OUT_2].pin  = {DSY_GPIOC, 6}; // TRIGGER OUT SNARE
    gate_output[GATE_OUT_2].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_2].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_2]);
    //dsy_gpio_write(&gate_output[1], 1);

    gate_output[GATE_OUT_3].pin  = {DSY_GPIOA, 1}; // TRIGGER OUT HIHAT 1
    gate_output[GATE_OUT_3].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_3].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_3]);
    //dsy_gpio_write(&gate_output[2], 1);

    gate_output[GATE_OUT_4].pin  = {DSY_GPIOA, 2}; // TRIGGER OUT HIHAT 2
    gate_output[GATE_OUT_4].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_4].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_4]);
    //dsy_gpio_write(&gate_output[3], 1);

    
    gate_output[GATE_OUT_5].pin = {DSY_GPIOC, 9}; // TRIGGER OUT PERC 1
    gate_output[GATE_OUT_5].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_5].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_5]);
    //dsy_gpio_write(&gate_output[4], 1);

    gate_output[GATE_OUT_6].pin  = {DSY_GPIOC, 5}; // TRIGGER OUT PERC 2
    gate_output[GATE_OUT_6].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_6].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_6]);
    //dsy_gpio_write(&gate_output[5], 1);

    gate_output[GATE_OUT_7].pin  = {DSY_GPIOB, 14}; // TRIGGER OUT SUBDIV // Rev 1 was C11 (not working)
    gate_output[GATE_OUT_7].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_7].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_7]);
    //dsy_gpio_write(&gate_output[6], 1);

    gate_output[GATE_OUT_8].pin  = {DSY_GPIOE, 15}; // TRIGGER OUT SYNC // Rev 1 was C12 (not working)
    gate_output[GATE_OUT_8].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_8].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_8]);
    //dsy_gpio_write(&gate_output[7], 1);

    // Gate Inputs
    // init when declared as gatein (no debounce)

    dsy_gpio_pin pin;
    pin = {DSY_GPIOB,  2}; gate_input[GATE_IN_1].Init(&pin);  // TAP
    pin = {DSY_GPIOB, 10}; gate_input[GATE_IN_2].Init(&pin);  // LINEAR DRUMMING 

}
