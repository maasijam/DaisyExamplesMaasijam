
#include "daisy_stolperbeats.h"
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

void DaisyStolperbeats::Init(bool boost)
{
    // Configure Seed first
    seed.Configure();
    seed.Init(boost);

    // dirty - clean this up
    seed.sdram_handle.DeInit();
    
    InitAudio();

    InitCvOutputs();
    InitEncoder();
    InitGates();
    InitButtons();
    InitSR595();

    //InitDAC8568();
    InitMidi();
    InitControls();
    // Reset AK4556
    /* dsy_gpio_write(&ak4556_reset_pin_, 0);
    DelayMs(10);
    dsy_gpio_write(&ak4556_reset_pin_, 1); */
}

void DaisyStolperbeats::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyStolperbeats::SetHidUpdateRates()
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


void DaisyStolperbeats::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyStolperbeats::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyStolperbeats::StopAudio()
{
    seed.StopAudio();
}

void DaisyStolperbeats::SetAudioSampleRate(
    SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisyStolperbeats::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisyStolperbeats::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyStolperbeats::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisyStolperbeats::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyStolperbeats::StartAdc()
{
    seed.adc.Start();
}

/** Stops Transfering data from the ADC */
void DaisyStolperbeats::StopAdc()
{
    seed.adc.Stop();
}

//ex DaisyStolperbeats::UpdateAnalogControls()
void DaisyStolperbeats::ProcessAnalogControls()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Process();
    }
}

//ex  DaisyStolperbeats::GetCtrlValue(Ctrl k)
float DaisyStolperbeats::GetKnobValue(Ctrl k)
{
    return (controls[k].Value());
}

//ex DaisyStolperbeats::DebounceControls()
void DaisyStolperbeats::ProcessDigitalControls()
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


void DaisyStolperbeats::InitLEDMatrix()
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

void DaisyStolperbeats::InitLEDMatrixDMA()
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



void DaisyStolperbeats::InitBelaTrill()
{
    /* static constexpr I2CHandle::Config i2c_config = {
        I2CHandle::Config::Peripheral::I2C_1,
        {{DSY_GPIOB, 8},
         {DSY_GPIOB, 9}}, 
        I2CHandle::Config::Speed::I2C_400KHZ}; */

    I2CHandle::Config i2c_config;

    i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_config.pin_config.scl = {DSY_GPIOB, 8},
    i2c_config.pin_config.sda = {DSY_GPIOB, 9};
    i2c_config.speed          = I2CHandle::Config::Speed::I2C_100KHZ; // I2C_400KHZ

    uint8_t addr = 0b00101000; // 0x28 - BELA SQUARE

    I2CHandle i2c;
    i2c.Init(i2c_config);

    // init with i2c handle, array of adresses, number of driver chips
    bela_trill.setup(i2c, Trill::device::SQUARE, addr);
}


// Private Function Implementations
// set SAI2 stuff -- run this between seed configure and init
void DaisyStolperbeats::InitAudio()
{
    // Handle Seed Audio as-is and then
    SaiHandle::Config sai_config[2];
    // Internal Codec
    sai_config[0].periph          = SaiHandle::Config::Peripheral::SAI_1;
    sai_config[0].sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config[0].bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config[0].a_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config[0].b_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config[0].a_dir           = SaiHandle::Config::Direction::TRANSMIT;
    sai_config[0].b_dir           = SaiHandle::Config::Direction::RECEIVE;
    sai_config[0].pin_config.fs   = {DSY_GPIOE, 4};
    sai_config[0].pin_config.mclk = {DSY_GPIOE, 2};
    sai_config[0].pin_config.sck  = {DSY_GPIOE, 5};
    sai_config[0].pin_config.sa   = {DSY_GPIOE, 6};
    sai_config[0].pin_config.sb   = {DSY_GPIOE, 3};

    // External Codec
    sai_config[1].periph          = SaiHandle::Config::Peripheral::SAI_2;
    sai_config[1].sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config[1].bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config[1].a_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config[1].b_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config[1].a_dir           = SaiHandle::Config::Direction::TRANSMIT;
    sai_config[1].b_dir           = SaiHandle::Config::Direction::RECEIVE;
    sai_config[1].pin_config.fs   = seed.GetPin(27);
    sai_config[1].pin_config.mclk = seed.GetPin(24);
    sai_config[1].pin_config.sck  = seed.GetPin(28);
    sai_config[1].pin_config.sb   = seed.GetPin(25);
    sai_config[1].pin_config.sa   = seed.GetPin(26);

    SaiHandle sai_handle[2];
    sai_handle[0].Init(sai_config[0]);
    sai_handle[1].Init(sai_config[1]);

    // Reset Pin for AK4556
    // Built-in AK4556 was reset during Seed Init
    dsy_gpio_pin codec_reset_pin = seed.GetPin(29);
    Ak4556::Init(codec_reset_pin);

    // Reinit Audio for _both_ codecs...
    AudioHandle::Config cfg;
    cfg.blocksize  = 48;
    cfg.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    cfg.postgain   = 0.5f;
    seed.audio_handle.Init(cfg, sai_handle[0], sai_handle[1]);
}

void DaisyStolperbeats::InitControls()
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

void DaisyStolperbeats::InitDAC8568()
{
    dsy_gpio_pin pincfg[Dac8568::NUM_PINS];
    pincfg[Dac8568::SYNC] = {DSY_GPIOB, 4}; //seed.GetPin(PIN_DAC8568_SYNC);
    dac_8568.Init(pincfg);
}

void DaisyStolperbeats::InitMidi()
{
    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);
}

void DaisyStolperbeats::InitCvOutputs()
{
    DacHandle::Config cfg;
    cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    cfg.buff_state = DacHandle::BufferState::ENABLED;
    cfg.mode       = DacHandle::Mode::POLLING;
    cfg.chn        = DacHandle::Channel::BOTH;
    seed.dac.Init(cfg);
    seed.dac.WriteValue(DacHandle::Channel::BOTH, 0);
}

void DaisyStolperbeats::InitEncoder()
{

    // A, B. CLICK
    encoder[ENC_1].Init({DSY_GPIOF,  3}, {DSY_GPIOF,  4},  {DSY_GPIOC, 15}, AudioCallbackRate()); // ENCODER TEMPO
    encoder[ENC_2].Init({DSY_GPIOF,  5}, {DSY_GPIOH,  2},  {DSY_GPIOE,  1}, AudioCallbackRate()); // ENCODER KICK
    encoder[ENC_3].Init({DSY_GPIOG, 12}, {DSY_GPIOG, 15},  {DSY_GPIOD,  6}, AudioCallbackRate()); // ENCODER SNARE
    encoder[ENC_4].Init({DSY_GPIOB, 13}, {DSY_GPIOD,  8},  {DSY_GPIOE, 13}, AudioCallbackRate()); // ENCODER HIHAT
    encoder[ENC_5].Init({DSY_GPIOB, 12}, {DSY_GPIOB, 15},  {DSY_GPIOE, 14}, AudioCallbackRate()); // ENCODER HIHAT 2
    encoder[ENC_6].Init({DSY_GPIOG, 14}, {DSY_GPIOE,  0},  {DSY_GPIOD,  7}, AudioCallbackRate()); // ENCODER SHUFFLE
    encoder[ENC_7].Init({DSY_GPIOD,  0}, {DSY_GPIOD,  2},  {DSY_GPIOG,  8}, AudioCallbackRate()); // ENCODER PERC 1
    encoder[ENC_8].Init({DSY_GPIOC,  7}, {DSY_GPIOD,  1},  {DSY_GPIOG,  7}, AudioCallbackRate()); // ENCODER PERC 2
    encoder[ENC_9].Init({DSY_GPIOE,  9}, {DSY_GPIOE, 11},  {DSY_GPIOE,  7}, AudioCallbackRate()); // ENCODER SUBDIV
    encoder[ENC_10].Init({DSY_GPIOE, 10}, {DSY_GPIOE, 12},  {DSY_GPIOE, 8}, AudioCallbackRate()); // ENCODER SYNC
}

void DaisyStolperbeats::InitSR595()
{
    num_sr_chained = 4;

    sr_pin_cfg[SR_LATCH] = {DSY_GPIOD, 4}; //seed.GetPin(PIN_SR_LATCH);
    sr_pin_cfg[SR_CLOCK] = {DSY_GPIOD, 5}; //seed.GetPin(PIN_SR_CLOCK);
    sr_pin_cfg[SR_DATA]  = {DSY_GPIOD, 3}; //seed.GetPin(PIN_SR_DATA);

    sr_595.Init(sr_pin_cfg, num_sr_chained);
}

void DaisyStolperbeats::InitButtons()
{
    // Buttons
    // dsy_gpio_pin pin;
    // float _AudioCallbackRate = AudioCallbackRate();

    // BUTTON TAP;
    button[BUTTON_1].Init({DSY_GPIOE, 15}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON LINEAR
    button[BUTTON_2].Init({DSY_GPIOF, 12}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON BANK DEC
    button[BUTTON_3].Init({DSY_GPIOC, 13}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON BANK INC
    button[BUTTON_4].Init({DSY_GPIOC, 14}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON SHUFFLE DILLA
    button[BUTTON_5].Init({DSY_GPIOG, 13}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON SHUFFLE SHAKE
    button[BUTTON_6].Init({DSY_GPIOC, 8}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON SHUFFLE PUSH
    button[BUTTON_7].Init({DSY_GPIOI, 8}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON SHUFFLE CLAVE
    button[BUTTON_8].Init({DSY_GPIOI, 11}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);

}

void DaisyStolperbeats::InitGates()
{
    // LED Linear Drumming
    led_linear.pin = {DSY_GPIOF, 2}; // LED LINEAR DRUMMING
    led_linear.mode = DSY_GPIO_MODE_OUTPUT_PP;
    led_linear.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&led_linear);

    // Gate Output
    gate_output[GATE_OUT_1].pin = {DSY_GPIOA, 8}; // TRIGGER OUT KICK
    gate_output[GATE_OUT_1].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_1].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_1]);
    //dsy_gpio_write(&gate_output[0], 1);

    gate_output[GATE_OUT_2].pin  = {DSY_GPIOC, 6}; // TRIGGER OUT SNARE
    gate_output[GATE_OUT_2].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_2].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_2]);
    //dsy_gpio_write(&gate_output[1], 1);

    gate_output[GATE_OUT_3].pin  = {DSY_GPIOD, 12}; // TRIGGER OUT HIHAT 1
    gate_output[GATE_OUT_3].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_3].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_3]);
    //dsy_gpio_write(&gate_output[2], 1);

    gate_output[GATE_OUT_4].pin  = {DSY_GPIOH, 6}; // TRIGGER OUT HIHAT 2
    gate_output[GATE_OUT_4].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_4].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_4]);
    //dsy_gpio_write(&gate_output[3], 1);

    
    gate_output[GATE_OUT_5].pin = {DSY_GPIOC, 9}; // TRIGGER OUT PERC 1
    gate_output[GATE_OUT_5].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_5].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_5]);
    //dsy_gpio_write(&gate_output[4], 1);

    gate_output[GATE_OUT_6].pin  = {DSY_GPIOC, 10}; // TRIGGER OUT PERC 2
    gate_output[GATE_OUT_6].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_6].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_6]);
    //dsy_gpio_write(&gate_output[5], 1);

    gate_output[GATE_OUT_7].pin  = {DSY_GPIOG, 4}; // TRIGGER OUT SUBDIV // Rev 1 was C11 (not working)
    gate_output[GATE_OUT_7].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_7].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_7]);
    //dsy_gpio_write(&gate_output[6], 1);

    gate_output[GATE_OUT_8].pin  = {DSY_GPIOG, 2}; // TRIGGER OUT SYNC // Rev 1 was C12 (not working)
    gate_output[GATE_OUT_8].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_8].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_8]);
    //dsy_gpio_write(&gate_output[7], 1);
    
    // Expander
    gate_output[GATE_OUT_9].pin = {DSY_GPIOH, 9}; // TRIGGER OUT EXPANDER 1 // Rev 1 was G4
    gate_output[GATE_OUT_9].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_9].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_9]);
    //dsy_gpio_write(&gate_output[8], 1);

    gate_output[GATE_OUT_10].pin  = {DSY_GPIOH, 10}; // TRIGGER OUT EXPANDER 2 // Rev 1 was G2
    gate_output[GATE_OUT_10].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_10].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_10]);
    //dsy_gpio_write(&gate_output[9], 1);

    gate_output[GATE_OUT_11].pin  = {DSY_GPIOD, 15}; // TRIGGER OUT EXPANDER 3
    gate_output[GATE_OUT_11].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_11].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_11]);
    //dsy_gpio_write(&gate_output[10], 1);

    gate_output[GATE_OUT_12].pin  = {DSY_GPIOD, 14}; // TRIGGER OUT EXPANDER 4
    gate_output[GATE_OUT_12].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_12].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_12]);
    //sy_gpio_write(&gate_output[11], 1);

    
    gate_output[GATE_OUT_13].pin = {DSY_GPIOG, 5}; // TRIGGER OUT EXPANDER 5
    gate_output[GATE_OUT_13].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_13].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_13]);
    //dsy_gpio_write(&gate_output[12], 1);

    gate_output[GATE_OUT_14].pin  = {DSY_GPIOD, 10}; // TRIGGER OUT EXPANDER 6
    gate_output[GATE_OUT_14].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_14].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_14]);
    //dsy_gpio_write(&gate_output[13], 1);

    gate_output[GATE_OUT_15].pin  = {DSY_GPIOD, 9}; // TRIGGER OUT EXPANDER 7
    gate_output[GATE_OUT_15].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_15].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_15]);
    //dsy_gpio_write(&gate_output[14], 1);

    gate_output[GATE_OUT_16].pin  = {DSY_GPIOD, 13}; // TRIGGER OUT EXPANDER 8
    gate_output[GATE_OUT_16].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_16].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_16]);
    //dsy_gpio_write(&gate_output[15], 1);

    // Gate Inputs
    // init when declared as gatein (no debounce)

    dsy_gpio_pin pin;
    pin = {DSY_GPIOB,  2}; gate_input[GATE_IN_1].Init(&pin);  // TAP
    pin = {DSY_GPIOB, 10}; gate_input[GATE_IN_2].Init(&pin);  // LINEAR DRUMMING 
    pin = {DSY_GPIOF, 12}; gate_input[GATE_IN_3].Init(&pin);  // EXPANDER 1
    pin = {DSY_GPIOF, 13}; gate_input[GATE_IN_4].Init(&pin);  // EXPANDER 2
    pin = {DSY_GPIOF, 11}; gate_input[GATE_IN_5].Init(&pin);  // EXPANDER 3
    pin = {DSY_GPIOH,  3}; gate_input[GATE_IN_6].Init(&pin);  // EXPANDER 4
    pin = {DSY_GPIOF, 14}; gate_input[GATE_IN_7].Init(&pin);  // EXPANDER 5
    pin = {DSY_GPIOF, 15}; gate_input[GATE_IN_8].Init(&pin);  // EXPANDER 6
    pin = {DSY_GPIOG,  1}; gate_input[GATE_IN_9].Init(&pin);  // EXPANDER 7
    pin = {DSY_GPIOG,  0}; gate_input[GATE_IN_10].Init(&pin); // EXPANDER 8

}
