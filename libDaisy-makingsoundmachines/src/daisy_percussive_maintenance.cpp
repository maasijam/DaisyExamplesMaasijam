#include "daisy_percussive_maintenance.h"
// #include "dev/codec_ak4556.h"

using namespace daisy;

// Hardware Definitions
// #define PIN_ENC_CLICK 0
// #define PIN_ENC_B 11
// #define PIN_ENC_A 12
// #define PIN_OLED_DC 9
// #define PIN_OLED_RESET 30
// #define PIN_MIDI_OUT 13
// #define PIN_MIDI_IN 14
// #define PIN_GATE_OUT 17
// #define PIN_GATE_IN_1 20
// #define PIN_GATE_IN_2 19
// #define PIN_SAI_SCK_A 28
// #define PIN_SAI2_FS_A 27
// #define PIN_SAI2_SD_A 26
// #define PIN_SAI2_SD_B 25
// #define PIN_SAI2_MCLK 24

// #define PIN_AK4556_RESET 29

// #define PIN_CTRL_1 15
// #define PIN_CTRL_2 16
// #define PIN_CTRL_3 21
// #define PIN_CTRL_4 18

void DaisyPercussiveMaintenance::Init(bool boost)
{
    // Configure Seed first
    seed.Configure();
    seed.Init(boost);

    //already configured in seed.Init()
    InitAudio();
    //InitDisplay();

    InitCvOutputs();
    InitEncoder();
    InitGates();
    InitButtons();

    InitDisplay();
    InitMidi();
    InitControls();
    // Set Screen update vars
    screen_update_period_ = 17; // roughly 60Hz
    screen_update_last_   = seed.system.GetNow();
}

void DaisyPercussiveMaintenance::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyPercussiveMaintenance::SetHidUpdateRates()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].SetSampleRate(AudioCallbackRate());
    }

    encoder[ENC_1].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_2].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_3].SetUpdateRate(AudioCallbackRate());
    encoder[ENC_4].SetUpdateRate(AudioCallbackRate());

    //button[BUTTON_1].SetUpdateRate(AudioCallbackRate());  
    //button[BUTTON_2].SetUpdateRate(AudioCallbackRate()); 
}

void DaisyPercussiveMaintenance::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyPercussiveMaintenance::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyPercussiveMaintenance::StopAudio()
{
    seed.StopAudio();
}

void DaisyPercussiveMaintenance::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisyPercussiveMaintenance::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisyPercussiveMaintenance::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyPercussiveMaintenance::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisyPercussiveMaintenance::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyPercussiveMaintenance::StartAdc()
{
    seed.adc.Start();
}

/** Stops Transfering data from the ADC */
void DaisyPercussiveMaintenance::StopAdc()
{
    seed.adc.Stop();
}

void DaisyPercussiveMaintenance::ProcessAnalogControls()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Process();
    }
}

float DaisyPercussiveMaintenance::GetKnobValue(Ctrl k)
{
    return (controls[k].Value());
}

void DaisyPercussiveMaintenance::ProcessDigitalControls()
{
    encoder[ENC_1].Debounce();
    encoder[ENC_2].Debounce();
    encoder[ENC_3].Debounce();
    encoder[ENC_4].Debounce();

    button[BUTTON_1].Debounce();
    button[BUTTON_2].Debounce();
}

// This will render the display with the controls as vertical bars
void DaisyPercussiveMaintenance::DisplayControls(bool invert)
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
        barwidth            = 4;
        barspacing          = 8;

        display[0].Fill(off);
        display[1].Fill(off);

        // Bars for all four knobs.
        for(size_t i = 0; i < DaisyPercussiveMaintenance::CTRL_LAST; i++)
        {
            float  v;
            size_t dest;
            curx = (barspacing * i + 1) + (barwidth * i);

            cury = display[0].Height();

            v    = GetKnobValue(static_cast<DaisyPercussiveMaintenance::Ctrl>(i));


            dest = (v * display[0].Height());

            for(size_t j = dest; j > 0; j--)
            {
                for(size_t k = 0; k < barwidth; k++)
                {
                    display[0].DrawPixel(curx + k, cury - j, on);
                    display[1].DrawPixel(curx + k, cury - j, on);
                }
            }
        }

        display[0].Update();
        display[1].Update();
    }
}

// Private Function Implementations
// set SAI2 stuff -- run this between seed configure and init
void DaisyPercussiveMaintenance::InitAudio()
{
    // SAI1 -- Peripheral
    // Configure
    // Internal Codec

    SaiHandle::Config sai_config;
    sai_config.periph          = SaiHandle::Config::Peripheral::SAI_1;
    sai_config.sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config.bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config.a_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config.b_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config.pin_config.fs   = {DSY_GPIOE, 4};
    sai_config.pin_config.mclk = {DSY_GPIOE, 2};
    sai_config.pin_config.sck  = {DSY_GPIOE, 5};

    // Data Line Directions
    sai_config.a_dir         = SaiHandle::Config::Direction::TRANSMIT;
    sai_config.pin_config.sa = {DSY_GPIOE, 6};
    sai_config.b_dir         = SaiHandle::Config::Direction::RECEIVE;
    sai_config.pin_config.sb = {DSY_GPIOE, 3};

    // I2C on I2C1
    I2CHandle::Config i2c_config;
    i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_config.pin_config.scl = {DSY_GPIOB, 8};
    i2c_config.pin_config.sda = {DSY_GPIOB, 9};

    I2CHandle i2c_handle;
    i2c_handle.Init(i2c_config);
    Wm8731::Config codec_cfg;
    codec_cfg.Defaults();
    Wm8731 codec;
    codec.Init(codec_cfg, i2c_handle);


    SaiHandle sai_handle;
    sai_handle.Init(sai_config);

    // Reinit Audio for codec
    AudioHandle::Config cfg;
    cfg.blocksize  = 48;
    cfg.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    cfg.postgain   = 0.5f; //1.f;
    seed.audio_handle.Init(cfg, sai_handle);
}

void DaisyPercussiveMaintenance::InitControls()
{
    AdcChannelConfig cfg[CTRL_LAST];

    // Init ADC channels with Pins
    // Init ADC channels with Pins
    cfg[CTRL_1].InitSingle({DSY_GPIOC, 4});  // Pitch 1 Jack
    cfg[CTRL_2].InitSingle({DSY_GPIOC, 1});  // Pitch 2 Jack
    cfg[CTRL_3].InitSingle({DSY_GPIOA, 6});  // Pitch 1 Pot
    cfg[CTRL_4].InitSingle({DSY_GPIOA, 7});  // Pitch 2 Pot
    cfg[CTRL_5].InitSingle({DSY_GPIOB, 1});  // Decay 1
    cfg[CTRL_6].InitSingle({DSY_GPIOA, 3});  // Decay 2
    cfg[CTRL_7].InitSingle({DSY_GPIOC, 0});  // Timbre 1
    cfg[CTRL_8].InitSingle({DSY_GPIOB, 0});  // Timbre 2
    cfg[CTRL_9].InitSingle({DSY_GPIOC, 2});  // Env Follower
    cfg[CTRL_10].InitSingle({DSY_GPIOC, 3}); // Ghost 1
    cfg[CTRL_11].InitSingle({DSY_GPIOC, 5}); // Ghost 2

    // Initialize ADC
    seed.adc.Init(cfg, CTRL_LAST);

    // Initialize AnalogControls, with flip set to true
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Init(seed.adc.GetPtr(i), AudioCallbackRate(), true);
    }
}

void DaisyPercussiveMaintenance::InitDisplay()
{
    OledDisplay<SSD130x4WireSpi128x64Driver>::Config display_config[2];

    dsy_gpio_pin pin_oled_1_dc, pin_oled_2_dc, pin_oled_1_reset, pin_oled_2_reset;


    // OLED 1       Pin
    // SPI4 (SPI5)
    //
    // sck          E12
    // sda / mosi   E14      
    // reset        A0
    // dc / miso    E13
    // cs / nss     E11

    pin_oled_1_dc    = {DSY_GPIOE, 13};
    pin_oled_1_reset = {DSY_GPIOA, 0};

    display_config[0].driver_config.transport_config.pin_config.dc = pin_oled_1_dc; // 
    display_config[0].driver_config.transport_config.pin_config.reset = pin_oled_1_reset;

    // OLED 2       Pin
    // SPI2
    //
    // sck          B13
    // sda / mosi   B15
    // reset        A1
    // dc / miso    B14
    // cs / nss     B12

    pin_oled_2_dc    = {DSY_GPIOB, 14};
    pin_oled_2_reset = {DSY_GPIOA, 1};

    display_config[1].driver_config.transport_config.pin_config.dc = pin_oled_2_dc; // 
    display_config[1].driver_config.transport_config.pin_config.reset = pin_oled_2_reset;

    display[0].Init_OLED_1(display_config[0]);
    display[1].Init_OLED_2(display_config[1]);
}

void DaisyPercussiveMaintenance::InitMidi()
{
    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);
}

void DaisyPercussiveMaintenance::InitCvOutputs()
{
    DacHandle::Config cfg;
    cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    cfg.buff_state = DacHandle::BufferState::ENABLED;
    cfg.mode       = DacHandle::Mode::POLLING;
    cfg.chn        = DacHandle::Channel::BOTH;
    seed.dac.Init(cfg);
    seed.dac.WriteValue(DacHandle::Channel::BOTH, 0);
}

void DaisyPercussiveMaintenance::InitEncoder()
{
    // A, B. CLICK
    encoder[ENC_1].Init({DSY_GPIOB,  5}, {DSY_GPIOC, 13},  {DSY_GPIOC, 15}, AudioCallbackRate()); // ENCODER TEMPO
    encoder[ENC_2].Init({DSY_GPIOA, 10}, {DSY_GPIOE,  0},  {DSY_GPIOE,  1}, AudioCallbackRate()); // ENCODER KICK
    encoder[ENC_3].Init({DSY_GPIOD, 14}, {DSY_GPIOE,  7},  {DSY_GPIOD, 13}, AudioCallbackRate()); // ENCODER SNARE
    encoder[ENC_4].Init({DSY_GPIOC,  6}, {DSY_GPIOD,  8},  {DSY_GPIOD,  9}, AudioCallbackRate()); // ENCODER HIHAT
}


void DaisyPercussiveMaintenance::InitButtons()
{
    // BUTTON Cherry 1
    button[BUTTON_1].Init({DSY_GPIOA,  2}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);
    // BUTTON Cherry 2
    button[BUTTON_2].Init({DSY_GPIOD, 10}, AudioCallbackRate(), Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_NONE);

    //dsy_gpio_pin pin;
    //pin = {DSY_GPIOA,  2}; button[BUTTON_1].Init(&pin);  // Trig 1
    //pin = {DSY_GPIOD, 10}; button[BUTTON_2].Init(&pin);  // Trig 2 

}


void DaisyPercussiveMaintenance::InitGates()
{
    // Gate Output
    // gate_output[GATE_OUT_1].pin = {DSY_GPIOA, 0}; // TRIGGER OUT KICK
    // gate_output[GATE_OUT_1].mode = DSY_GPIO_MODE_OUTPUT_PP;
    // gate_output[GATE_OUT_1].pull = DSY_GPIO_NOPULL;
    // dsy_gpio_init(&gate_output[GATE_OUT_1]);
    //dsy_gpio_write(&gate_output[0], 1);

    // Gate Inputs
    // init when declared as gatein (no debounce)

    dsy_gpio_pin pin;
    pin = {DSY_GPIOD,  7}; gate_input[GATE_IN_1].Init(&pin);  // Trig 1
    pin = {DSY_GPIOC, 14}; gate_input[GATE_IN_2].Init(&pin);  // Trig 2 
    pin = {DSY_GPIOA,  8}; gate_input[GATE_IN_3].Init(&pin);  // Acc 1
    pin = {DSY_GPIOC,  7}; gate_input[GATE_IN_4].Init(&pin);  // Acc 2 
    pin = {DSY_GPIOD,  0}; gate_input[GATE_IN_5].Init(&pin);  // Ghost 1
    pin = {DSY_GPIOD,  1}; gate_input[GATE_IN_6].Init(&pin);  // Ghost 2 
    pin = {DSY_GPIOD, 11}; gate_input[GATE_IN_7].Init(&pin);  // Piezo Trigger
    pin = {DSY_GPIOD, 12}; gate_input[GATE_IN_8].Init(&pin);  // Piezo Gate 
}
