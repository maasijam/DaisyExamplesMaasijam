
#include "daisy_heart-of-gold.h"
#include "dev/codec_ak4556.h"

using namespace daisy;

/** MSM modified for use with Heart of Gold */

// Hardware Definitions
#define PIN_ENC_1_CLICK 0 // HoG Enc 1 Click PB12
#define PIN_ENC_1_A 11 // HoG Enc 1 Click PB8
#define PIN_ENC_1_B 12 // HoG Enc 1 Click PB9

#define PIN_ENC_2_CLICK 44 // PC15
#define PIN_ENC_2_A 30 // PB15
#define PIN_ENC_2_B 45 // PB13

#define PIN_ENC_3_CLICK 46 // PD13
#define PIN_ENC_3_A 47 // PD7
#define PIN_ENC_3_B 48 // PD6

#define PIN_ENC_4_CLICK 49 // PG12
#define PIN_ENC_4_A 7 // PG10
#define PIN_ENC_4_B 50 // PG7

#define PIN_MIDI_OUT 13 // HoG MIDI Out PB6
#define PIN_MIDI_IN 14 // HoG MIDI In PB7

#define PIN_GATE_OUT_1 17 // HoG Trig Out 1 PA8
#define PIN_GATE_OUT_2 39 // PC6
#define PIN_GATE_OUT_3 40 // PD12
#define PIN_GATE_OUT_4 41 // PH6

#define PIN_GATE_IN_1 20 // HoG Trig In 1 PB2
#define PIN_GATE_IN_2 19 // HoG Trig In 2 PB10
#define PIN_GATE_IN_3 42 // PC13
#define PIN_GATE_IN_4 43 // PC14

#define PIN_ROUTE_BUTTON_1 54 // PG13
#define PIN_ROUTE_BUTTON_2 55 // PH7
#define PIN_ROUTE_BUTTON_3 56 // PI8
#define PIN_ROUTE_BUTTON_4 57 // PI11

#define PIN_SAI_SCK_A 28 // HoG SAI2_SCK_B PA2
#define PIN_SAI2_FS_A 27 // HoG SAI2_FS_B PG9
#define PIN_SAI2_SD_A 26 // HoG SAI2_SD_A PD 11
#define PIN_SAI2_SD_B 25 // HoG SAI2_SD_B PA0
#define PIN_SAI2_MCLK 24 // HoG SAI2_MCLK_B PA1

#define PIN_AK4556_RESET 29 // HoG PB14

#define PIN_CTRL_1 21 // PC4
#define PIN_CTRL_2 31 // PC1
#define PIN_CTRL_3 32 // PA6
#define PIN_CTRL_4 18 // PA7
#define PIN_CTRL_5 33 // PB1
#define PIN_CTRL_6 16 // PA3
#define PIN_CTRL_7 15 // PC0
#define PIN_CTRL_8 34 // PB0
#define PIN_CTRL_9 35 // PC2 should be 35
#define PIN_CTRL_10 36 // PC3 should be 36
#define PIN_CTRL_11 37 // PC5 should be 37
#define PIN_CTRL_12 38 // PH4 should be 38

#define PIN_SR_DATA 51 // PD3
#define PIN_SR_CLOCK 52 // PD5
#define PIN_SR_LATCH 53 // PD4

#define PIN_DAC8568_SYNC 9 //PB4

void DaisyHeartOfGold::Init(bool boost)
{
    // Configure Seed first
    seed.Configure();
    seed.Init(boost);
    sdram_handle.Init();
    InitAudio();

    InitCvOutputs();
    InitEncoder();
    InitGates();
    InitRouteButtons();
    InitSR595();

    InitDAC8568();
    InitMidi();
    InitControls();
    // Set Screen update vars
    screen_update_period_ = 17; // roughly 60Hz
    screen_update_last_   = seed.system.GetNow();
}

void DaisyHeartOfGold::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyHeartOfGold::SetHidUpdateRates()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].SetSampleRate(AudioCallbackRate());
    }
    encoder[ENC_1].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_2].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_3].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_4].SetUpdateRate(AudioCallbackRate());

    // use when declared as switch
    // do not use when declared as gatein
    /* gate_input[GATE_IN_1].SetUpdateRate(AudioCallbackRate());
    gate_input[GATE_IN_2].SetUpdateRate(AudioCallbackRate());
    gate_input[GATE_IN_3].SetUpdateRate(AudioCallbackRate());
    gate_input[GATE_IN_4].SetUpdateRate(AudioCallbackRate()); */

    // use when declared as switch
    // do not use when declared as gatein
    /* route_button[ROUTE_BUTTON_1].SetUpdateRate(AudioCallbackRate());
    route_button[ROUTE_BUTTON_2].SetUpdateRate(AudioCallbackRate());
    route_button[ROUTE_BUTTON_3].SetUpdateRate(AudioCallbackRate());
    route_button[ROUTE_BUTTON_4].SetUpdateRate(AudioCallbackRate()); */
}


void DaisyHeartOfGold::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyHeartOfGold::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyHeartOfGold::StopAudio()
{
    seed.StopAudio();
}

void DaisyHeartOfGold::SetAudioSampleRate(
    SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisyHeartOfGold::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisyHeartOfGold::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyHeartOfGold::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisyHeartOfGold::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyHeartOfGold::StartAdc()
{
    seed.adc.Start();
}

/** Stops Transfering data from the ADC */
void DaisyHeartOfGold::StopAdc()
{
    seed.adc.Stop();
}

//ex DaisyHeartOfGold::UpdateAnalogControls()
void DaisyHeartOfGold::ProcessAnalogControls()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Process();
    }
}

//ex  DaisyHeartOfGold::GetCtrlValue(Ctrl k)
float DaisyHeartOfGold::GetKnobValue(Ctrl k)
{
    return (controls[k].Value());
}

//ex DaisyHeartOfGold::DebounceControls()
void DaisyHeartOfGold::ProcessDigitalControls()
{
    encoder[ENC_1].Debounce();
    encoder[ENC_2].Debounce();
    encoder[ENC_3].Debounce();
    encoder[ENC_4].Debounce();

    // debounce when declared as switch
    // do not debounce when declared as gatein
    /* gate_input[GATE_IN_1].Debounce();
    gate_input[GATE_IN_2].Debounce();
    gate_input[GATE_IN_3].Debounce();
    gate_input[GATE_IN_4].Debounce(); */

    // debounce when declared as switch
    // do not debounce when declared as gatein
    /* route_button[ROUTE_BUTTON_1].Debounce();
    route_button[ROUTE_BUTTON_2].Debounce();
    route_button[ROUTE_BUTTON_3].Debounce();
    route_button[ROUTE_BUTTON_4].Debounce(); */
}


void DaisyHeartOfGold::InitLEDMatrix()
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

void DaisyHeartOfGold::InitLEDMatrixDMA()
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



void DaisyHeartOfGold::InitBelaTrill()
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
    i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;

    uint8_t addr = 0b00101000; // 0x28 - BELA SQUARE

    I2CHandle i2c;
    i2c.Init(i2c_config);

    // init with i2c handle, array of adresses, number of driver chips
    bela_trill.setup(i2c, Trill::device::SQUARE, addr);
}

// This will render the display with the controls as vertical bars
void DaisyHeartOfGold::DisplayControls(bool invert)
{
    bool on, off;
    on  = invert ? false : true;
    off = invert ? true : false;
    if(seed.system.GetNow() - screen_update_last_ > screen_update_period_)
    {
        // Graph Knobs
        size_t barwidth, barspacing;
        size_t curx, cury;
        screen_update_last_ = seed.system.GetNow();
        barwidth            = 15;
        barspacing          = 20;
        display.Fill(off);
        // Bars for all four knobs.
        for(size_t i = 0; i < 11; i++) // (size_t i = 0; i < DaisyHeartOfGold::CTRL_LAST; i++)
        {
            float  v;
            size_t dest;
            curx = (barspacing * i + 1) + (barwidth * i);
            cury = display.Height();
            v    = GetKnobValue(static_cast<DaisyHeartOfGold::Ctrl>(i));
            dest = (v * display.Height());
            for(size_t j = dest; j > 0; j--)
            {
                for(size_t k = 0; k < barwidth; k++)
                {
                    display.DrawPixel(curx + k, cury - j, on);
                }
            }
        }
        display.Update();
    }
}


// Private Function Implementations
// set SAI2 stuff -- run this between seed configure and init
void DaisyHeartOfGold::InitAudio()
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

void DaisyHeartOfGold::InitControls()
{
    AdcChannelConfig cfg[CTRL_LAST];

    // Init ADC channels with Pins
    cfg[CTRL_1].InitSingle({DSY_GPIOC, 4});  //seed.GetPin(PIN_CTRL_1));
    cfg[CTRL_2].InitSingle({DSY_GPIOC, 1});  //seed.GetPin(PIN_CTRL_2));
    cfg[CTRL_3].InitSingle({DSY_GPIOA, 6});  //seed.GetPin(PIN_CTRL_3));
    cfg[CTRL_4].InitSingle({DSY_GPIOA, 7});  //seed.GetPin(PIN_CTRL_4));
    cfg[CTRL_5].InitSingle({DSY_GPIOB, 1});  //seed.GetPin(PIN_CTRL_5));
    cfg[CTRL_6].InitSingle({DSY_GPIOA, 3});  //seed.GetPin(PIN_CTRL_6));
    cfg[CTRL_7].InitSingle({DSY_GPIOC, 0});  //seed.GetPin(PIN_CTRL_7));
    cfg[CTRL_8].InitSingle({DSY_GPIOB, 0});  //seed.GetPin(PIN_CTRL_8));
    cfg[CTRL_9].InitSingle({DSY_GPIOC, 2});  //seed.GetPin(PIN_CTRL_9));
    cfg[CTRL_10].InitSingle({DSY_GPIOC, 3}); //seed.GetPin(PIN_CTRL_10));
    cfg[CTRL_11].InitSingle({DSY_GPIOC, 5}); //seed.GetPin(PIN_CTRL_11));
    //cfg[CTRL_12].InitSingle({DSY_GPIOH, 4}); //seed.GetPin(PIN_CTRL_12));

    // Initialize ADC
    //seed.adc.Init(cfg, CTRL_LAST);
    seed.adc.Init(cfg, 11);

    // Initialize AnalogControls, with flip set to true
    // for(size_t i = 0; i < CTRL_LAST; i++)
    for(size_t i = 0; i < 11; i++)
    {
        controls[i].Init(seed.adc.GetPtr(i), AudioCallbackRate(), true);
    }
}

void DaisyHeartOfGold::InitDAC8568()
{
    dsy_gpio_pin pincfg[Dac8568::NUM_PINS];
    pincfg[Dac8568::SYNC] = {DSY_GPIOB, 4}; //seed.GetPin(PIN_DAC8568_SYNC);
    dac_8568.Init(pincfg);
}


void DaisyHeartOfGold::InitDisplay()
{

    OledDisplay<SSD130xI2c128x64Driver>::Config display_config;

    display_config.driver_config.transport_config.i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
    display_config.driver_config.transport_config.i2c_config.speed          = I2CHandle::Config::Speed::I2C_1MHZ;
    display_config.driver_config.transport_config.i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    display_config.driver_config.transport_config.i2c_config.pin_config.scl = {DSY_GPIOB, 8};
    display_config.driver_config.transport_config.i2c_config.pin_config.sda = {DSY_GPIOB, 9};
    display_config.driver_config.transport_config.i2c_address               = 0x3C;

    display.Init(display_config);
}


void DaisyHeartOfGold::InitMidi()
{
    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);
}

void DaisyHeartOfGold::InitCvOutputs()
{
    DacHandle::Config cfg;
    cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    cfg.buff_state = DacHandle::BufferState::ENABLED;
    cfg.mode       = DacHandle::Mode::POLLING;
    cfg.chn        = DacHandle::Channel::BOTH;
    seed.dac.Init(cfg);
    seed.dac.WriteValue(DacHandle::Channel::BOTH, 0);
}

void DaisyHeartOfGold::InitEncoder()
{
#ifdef HEARTOFGOLD_REV1

    encoder[ENC_1].Init(
        {DSY_GPIOB, 8},  //seed.GetPin(PIN_ENC_1_A), HoG v2 {DSY_GPIOC, 7}
        {DSY_GPIOB, 9},  //seed.GetPin(PIN_ENC_1_B), HoG v2 {DSY_GPIOG, 14}
        {DSY_GPIOB, 12}, //seed.GetPin(PIN_ENC_1_CLICK),
        AudioCallbackRate());

#else

    encoder[ENC_1].Init(
        {DSY_GPIOG, 14}, //seed.GetPin(PIN_ENC_1_A), HoG v2 {DSY_GPIOG, 14}
        {DSY_GPIOC, 7},  //seed.GetPin(PIN_ENC_1_B), HoG v2 {DSY_GPIOC, 7}
        {DSY_GPIOB, 12}, //seed.GetPin(PIN_ENC_1_CLICK),
        AudioCallbackRate());

#endif

    encoder[ENC_2].Init({DSY_GPIOB, 15}, //seed.GetPin(PIN_ENC_2_A)
                        {DSY_GPIOB, 13}, //seed.GetPin(PIN_ENC_2_B)
                        {DSY_GPIOC, 15}, //seed.GetPin(PIN_ENC_2_CLICK),
                        AudioCallbackRate());

    encoder[ENC_3].Init({DSY_GPIOD, 7},  //seed.GetPin(PIN_ENC_3_A)
                        {DSY_GPIOD, 6},  //seed.GetPin(PIN_ENC_3_B)
                        {DSY_GPIOD, 13}, //seed.GetPin(PIN_ENC_3_CLICK),
                        AudioCallbackRate());

    encoder[ENC_4].Init({DSY_GPIOG, 10}, //seed.GetPin(PIN_ENC_4_A)
                        {DSY_GPIOG, 7},  //seed.GetPin(PIN_ENC_4_B)
                        {DSY_GPIOG, 12}, //seed.GetPin(PIN_ENC_4_CLICK),
                        AudioCallbackRate());
}

void DaisyHeartOfGold::InitSR595()
{
    num_sr_chained = 4;

    sr_pin_cfg[SR_LATCH] = {DSY_GPIOD, 4}; //seed.GetPin(PIN_SR_LATCH);
    sr_pin_cfg[SR_CLOCK] = {DSY_GPIOD, 5}; //seed.GetPin(PIN_SR_CLOCK);
    sr_pin_cfg[SR_DATA]  = {DSY_GPIOD, 3}; //seed.GetPin(PIN_SR_DATA);

    sr_595.Init(sr_pin_cfg, num_sr_chained);
}

void DaisyHeartOfGold::InitRouteButtons()
{
    // Route Buttons
    dsy_gpio_pin pin;
    pin = {DSY_GPIOG, 13};
    route_button[ROUTE_BUTTON_1].Init(&pin);
    pin = {DSY_GPIOH, 7};
    route_button[ROUTE_BUTTON_2].Init(&pin);
    pin = {DSY_GPIOI, 8};
    route_button[ROUTE_BUTTON_3].Init(&pin);
    pin = {DSY_GPIOI, 11};
    route_button[ROUTE_BUTTON_4].Init(&pin);

    // Gate Inputs
    // init when declared as switch (with debounce)
    //route_button[ROUTE_BUTTON_1].Init({DSY_GPIOG, 13}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    //route_button[ROUTE_BUTTON_2].Init({DSY_GPIOH, 7}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    //route_button[ROUTE_BUTTON_3].Init({DSY_GPIOI, 8}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    //route_button[ROUTE_BUTTON_4].Init({DSY_GPIOI, 11}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
}

void DaisyHeartOfGold::InitGates()
{
    // Gate Output
    gate_output[GATE_OUT_1].pin = {DSY_GPIOA, 8}; //seed.GetPin(PIN_GATE_OUT_1);
    gate_output[GATE_OUT_1].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_1].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_1]);

    gate_output[GATE_OUT_2].pin = {DSY_GPIOC, 6}; //seed.GetPin(PIN_GATE_OUT_2);
    gate_output[GATE_OUT_2].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_2].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_2]);

    gate_output[GATE_OUT_3].pin
        = {DSY_GPIOD, 12}; //seed.GetPin(PIN_GATE_OUT_3);
    gate_output[GATE_OUT_3].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_3].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_3]);

    gate_output[GATE_OUT_4].pin = {DSY_GPIOH, 6}; //seed.GetPin(PIN_GATE_OUT_4);
    gate_output[GATE_OUT_4].mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output[GATE_OUT_4].pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output[GATE_OUT_4]);

    // Gate Inputs
    // init when declared as switch (with debounce)
    // gate_input[GATE_IN_1].Init({DSY_GPIOB, 2}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // gate_input[GATE_IN_2].Init({DSY_GPIOB, 10}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // gate_input[GATE_IN_3].Init({DSY_GPIOC, 13}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // gate_input[GATE_IN_4].Init({DSY_GPIOC, 14}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);

    // Gate Inputs
    // init when declared as gatein (no debounce)
    
    dsy_gpio_pin pin;
    pin = {DSY_GPIOB, 2};  //seed.GetPin(PIN_GATE_IN_1);
    gate_input[GATE_IN_1].Init(&pin);
    pin = {DSY_GPIOB, 10}; //seed.GetPin(PIN_GATE_IN_2);
    gate_input[GATE_IN_2].Init(&pin);
    pin = {DSY_GPIOC, 13}; //seed.GetPin(PIN_GATE_IN_3);
    gate_input[GATE_IN_3].Init(&pin);
    pin = {DSY_GPIOC, 14}; //seed.GetPin(PIN_GATE_IN_4);
    gate_input[GATE_IN_4].Init(&pin); 
    
}
