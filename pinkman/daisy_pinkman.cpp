#include "daisy_pinkman.h"
#include <vector>

namespace daisy
{
namespace pinkman
{
    /** Const definitions */
    static constexpr dsy_gpio_pin DUMMYPIN        = {DSY_GPIOX, 0};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_1  = {DSY_GPIOA, 3};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_2  = {DSY_GPIOA, 6};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_3  = {DSY_GPIOA, 2};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_4  = {DSY_GPIOA, 7};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_5  = {DSY_GPIOB, 1};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_6  = {DSY_GPIOC, 4};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_7  = {DSY_GPIOC, 0};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_8  = {DSY_GPIOC, 1};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_9  = {DSY_GPIOA, 1};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_10 = {DSY_GPIOA, 0};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_11 = {DSY_GPIOC, 3};
    static constexpr dsy_gpio_pin PIN_ADC_CTRL_12 = {DSY_GPIOC, 2};
    static constexpr dsy_gpio_pin PIN_USER_LED    = {DSY_GPIOC, 7};

    static constexpr dsy_gpio_pin PIN_ADC_KNOB_MUX = {DSY_GPIOA, 1};
    static constexpr dsy_gpio_pin PIN_MUX_A = {DSY_GPIOB, 14};
    static constexpr dsy_gpio_pin PIN_MUX_B = {DSY_GPIOB, 4};
    static constexpr dsy_gpio_pin PIN_MUX_C = {DSY_GPIOB, 15};

    static constexpr dsy_gpio_pin BTN_TOP = {DSY_GPIOD, 3};
    static constexpr dsy_gpio_pin BTN_BIG = {DSY_GPIOB, 8};
    static constexpr dsy_gpio_pin BTN_RED = {DSY_GPIOB, 9};

    static constexpr dsy_gpio_pin SW_TOGGLE = {DSY_GPIOA, 0};

    static constexpr dsy_gpio_pin LED1 = {DSY_GPIOA, 4};
    static constexpr dsy_gpio_pin LED2 = {DSY_GPIOC, 2};
    static constexpr dsy_gpio_pin LED3 = {DSY_GPIOC, 3};
    static constexpr dsy_gpio_pin LED4 = {DSY_GPIOC, 14};
    static constexpr dsy_gpio_pin LED5 = {DSY_GPIOC, 13};

    

    const dsy_gpio_pin kPinMap[4][10] = {
        /** Header Bank A */
        {
            DUMMYPIN,        /**< A1  - -12V Power Input */
            {DSY_GPIOA, 1},  /**< A2  - UART1 Rx */
            {DSY_GPIOA, 0},  /**< A3  - UART1 Tx */
            DUMMYPIN,        /**< A4  - GND */
            DUMMYPIN,        /**< A5  - +12V Power Input */
            DUMMYPIN,        /**< A6  - +5V Power Output */
            DUMMYPIN,        /**< A7  - GND */
            {DSY_GPIOB, 14}, /**< A8  - USB DM */
            {DSY_GPIOB, 15}, /**< A9  - USB DP */
            DUMMYPIN,        /**< A10 - +3V3 Power Output */
        },
        /** Header Bank B */
        {
            DUMMYPIN,        /**< B1  - Audio Out Right */
            DUMMYPIN,        /**< B2  - Audio Out Left*/
            DUMMYPIN,        /**< B3  - Audio In Right */
            DUMMYPIN,        /**< B4  - Audio In Left */
            {DSY_GPIOC, 14}, /**< B5  - GATE OUT 1 */
            {DSY_GPIOC, 13}, /**< B6  - GATE OUT 2 */
            {DSY_GPIOB, 8},  /**< B7  - I2C1 SCL */
            {DSY_GPIOB, 9},  /**< B8  - I2C1 SDA */
            {DSY_GPIOG, 14}, /**< B9  - GATE IN 2 */
            {DSY_GPIOG, 13}, /**< B10 - GATE IN 1 */
        },
        /** Header Bank C */
        {
            {DSY_GPIOA, 5}, /**< C1  - CV Out 2 */
            PIN_ADC_CTRL_4, /**< C2  - CV In 4 */
            PIN_ADC_CTRL_3, /**< C3  - CV In 3 */
            PIN_ADC_CTRL_2, /**< C4  - CV In 2 */
            PIN_ADC_CTRL_1, /**< C5  - CV In 1 */
            PIN_ADC_CTRL_5, /**< C6  - CV In 5 */
            PIN_ADC_CTRL_6, /**< C7  - CV In 6 */
            PIN_ADC_CTRL_7, /**< C8  - CV In 7 */
            PIN_ADC_CTRL_8, /**< C9  - CV In 8 */
            {DSY_GPIOA, 4}, /**< C10 - CV Out 1 */
        },
        /** Header Bank D */
        {
            {DSY_GPIOB, 4},  /**< D1  - SPI2 CS */
            {DSY_GPIOC, 11}, /**< D2  - SDMMC D3 */
            {DSY_GPIOC, 10}, /**< D3  - SDMMC D2*/
            {DSY_GPIOC, 9},  /**< D4  - SDMMC D1*/
            {DSY_GPIOC, 8},  /**< D5  - SDMMC D0 */
            {DSY_GPIOC, 12}, /**< D6  - SDMMC CK */
            {DSY_GPIOD, 2},  /**< D7  - SDMMC CMD */
            {DSY_GPIOC, 2},  /**< D8  - SPI2 MISO */
            {DSY_GPIOC, 3},  /**< D9  - SPI2 MOSI */
            {DSY_GPIOD, 3},  /**< D10 - SPI2 SCK  */
        },
    };

    const dsy_gpio_pin DaisyPinkman::A1  = kPinMap[0][0];
    const dsy_gpio_pin DaisyPinkman::A2  = kPinMap[0][1];
    const dsy_gpio_pin DaisyPinkman::A3  = kPinMap[0][2];
    const dsy_gpio_pin DaisyPinkman::A4  = kPinMap[0][3];
    const dsy_gpio_pin DaisyPinkman::A5  = kPinMap[0][4];
    const dsy_gpio_pin DaisyPinkman::A6  = kPinMap[0][5];
    const dsy_gpio_pin DaisyPinkman::A7  = kPinMap[0][6];
    const dsy_gpio_pin DaisyPinkman::A8  = kPinMap[0][7];
    const dsy_gpio_pin DaisyPinkman::A9  = kPinMap[0][8];
    const dsy_gpio_pin DaisyPinkman::A10 = kPinMap[0][9];
    const dsy_gpio_pin DaisyPinkman::B1  = kPinMap[1][0];
    const dsy_gpio_pin DaisyPinkman::B2  = kPinMap[1][1];
    const dsy_gpio_pin DaisyPinkman::B3  = kPinMap[1][2];
    const dsy_gpio_pin DaisyPinkman::B4  = kPinMap[1][3];
    const dsy_gpio_pin DaisyPinkman::B5  = kPinMap[1][4];
    const dsy_gpio_pin DaisyPinkman::B6  = kPinMap[1][5];
    const dsy_gpio_pin DaisyPinkman::B7  = kPinMap[1][6];
    const dsy_gpio_pin DaisyPinkman::B8  = kPinMap[1][7];
    const dsy_gpio_pin DaisyPinkman::B9  = kPinMap[1][8];
    const dsy_gpio_pin DaisyPinkman::B10 = kPinMap[1][9];
    const dsy_gpio_pin DaisyPinkman::C1  = kPinMap[2][0];
    const dsy_gpio_pin DaisyPinkman::C2  = kPinMap[2][1];
    const dsy_gpio_pin DaisyPinkman::C3  = kPinMap[2][2];
    const dsy_gpio_pin DaisyPinkman::C4  = kPinMap[2][3];
    const dsy_gpio_pin DaisyPinkman::C5  = kPinMap[2][4];
    const dsy_gpio_pin DaisyPinkman::C6  = kPinMap[2][5];
    const dsy_gpio_pin DaisyPinkman::C7  = kPinMap[2][6];
    const dsy_gpio_pin DaisyPinkman::C8  = kPinMap[2][7];
    const dsy_gpio_pin DaisyPinkman::C9  = kPinMap[2][8];
    const dsy_gpio_pin DaisyPinkman::C10 = kPinMap[2][9];
    const dsy_gpio_pin DaisyPinkman::D1  = kPinMap[3][0];
    const dsy_gpio_pin DaisyPinkman::D2  = kPinMap[3][1];
    const dsy_gpio_pin DaisyPinkman::D3  = kPinMap[3][2];
    const dsy_gpio_pin DaisyPinkman::D4  = kPinMap[3][3];
    const dsy_gpio_pin DaisyPinkman::D5  = kPinMap[3][4];
    const dsy_gpio_pin DaisyPinkman::D6  = kPinMap[3][5];
    const dsy_gpio_pin DaisyPinkman::D7  = kPinMap[3][6];
    const dsy_gpio_pin DaisyPinkman::D8  = kPinMap[3][7];
    const dsy_gpio_pin DaisyPinkman::D9  = kPinMap[3][8];
    const dsy_gpio_pin DaisyPinkman::D10 = kPinMap[3][9];

    /** outside of class static buffer(s) for DMA access */
    uint16_t DMA_BUFFER_MEM_SECTION dsy_patch_sm_dac_buffer[2][48];

    class DaisyPinkman::Impl
    {
      public:
        Impl()
        {
            dac_running_            = false;
            dac_buffer_size_        = 48;
            dac_output_[0]          = 0;
            dac_output_[1]          = 0;
            internal_dac_buffer_[0] = dsy_patch_sm_dac_buffer[0];
            internal_dac_buffer_[1] = dsy_patch_sm_dac_buffer[1];
        }

        void InitDac();

        void StartDac(DacHandle::DacCallback callback);

        void StopDac();

        static void InternalDacCallback(uint16_t **output, size_t size);

        /** Based on a 0-5V output with a 0-4095 12-bit DAC */
        static inline uint16_t VoltageToCode(float input)
        {
            float pre = input * 819.f;
            if(pre > 4095.f)
                pre = 4095.f;
            else if(pre < 0.f)
                pre = 0.f;
            return (uint16_t)pre;
        }

        inline void WriteCvOut(int channel, float voltage)
        {
            if(channel == 0 || channel == 1)
                dac_output_[0] = VoltageToCode(voltage);
            if(channel == 0 || channel == 2)
                dac_output_[1] = VoltageToCode(voltage);
        }

        size_t    dac_buffer_size_;
        uint16_t *internal_dac_buffer_[2];
        uint16_t  dac_output_[2];
        DacHandle dac_;

      private:
        bool dac_running_;
    };

    /** Static Local Object */
    static DaisyPinkman::Impl pinkman_hw;

    /** Impl function definintions */

    void DaisyPinkman::Impl::InitDac()
    {
        DacHandle::Config dac_config;
        dac_config.mode     = DacHandle::Mode::DMA;
        dac_config.bitdepth = DacHandle::BitDepth::
            BITS_12; /**< Sets the output value to 0-4095 */
        dac_config.chn               = DacHandle::Channel::BOTH;
        dac_config.buff_state        = DacHandle::BufferState::ENABLED;
        dac_config.target_samplerate = 48000;
        dac_.Init(dac_config);
    }

    void DaisyPinkman::Impl::StartDac(DacHandle::DacCallback callback)
    {
        if(dac_running_)
            dac_.Stop();
        dac_.Start(internal_dac_buffer_[0],
                   internal_dac_buffer_[1],
                   dac_buffer_size_,
                   callback == nullptr ? InternalDacCallback : callback);
        dac_running_ = true;
    }

    void DaisyPinkman::Impl::StopDac()
    {
        dac_.Stop();
        dac_running_ = false;
    }


    void DaisyPinkman::Impl::InternalDacCallback(uint16_t **output, size_t size)
    {
        /** We could add some smoothing, interp, or something to make this a bit less waste-y */
        // std::fill(&output[0][0], &output[0][size], patch_sm_hw.dac_output_[0]);
        // std::fill(&output[1][1], &output[1][size], patch_sm_hw.dac_output_[1]);
        for(size_t i = 0; i < size; i++)
        {
            output[0][i] = pinkman_hw.dac_output_[0];
            output[1][i] = pinkman_hw.dac_output_[1];
        }
    }

    /** Actual DaisyPatchSM implementation
 *  With the pimpl model in place, we can/should probably
 *  move the rest of the implementation to the Impl class
 */

    void DaisyPinkman::Init()
    {
        /** Assign pimpl pointer */
        pimpl_ = &pinkman_hw;
        /** Initialize the MCU and clock tree */
        System::Config syscfg;
        syscfg.Boost();

        auto memory = System::GetProgramMemoryRegion();
        if(memory != System::MemoryRegion::INTERNAL_FLASH)
            syscfg.skip_clocks = true;

        system.Init(syscfg);
        /** Memories */
        if(memory == System::MemoryRegion::INTERNAL_FLASH)
        {
            /** FMC SDRAM */
            sdram.Init();
        }
        if(memory != System::MemoryRegion::QSPI)
        {
            /** QUADSPI FLASH */
            QSPIHandle::Config qspi_config;
            qspi_config.device = QSPIHandle::Config::Device::IS25LP064A;
            qspi_config.mode   = QSPIHandle::Config::Mode::MEMORY_MAPPED;
            qspi_config.pin_config.io0 = {DSY_GPIOF, 8};
            qspi_config.pin_config.io1 = {DSY_GPIOF, 9};
            qspi_config.pin_config.io2 = {DSY_GPIOF, 7};
            qspi_config.pin_config.io3 = {DSY_GPIOF, 6};
            qspi_config.pin_config.clk = {DSY_GPIOF, 10};
            qspi_config.pin_config.ncs = {DSY_GPIOG, 6};
            qspi.Init(qspi_config);
        }
        /** Audio */
        // Audio Init
        SaiHandle::Config sai_config;
        sai_config.periph          = SaiHandle::Config::Peripheral::SAI_1;
        sai_config.sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
        sai_config.bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
        sai_config.a_sync          = SaiHandle::Config::Sync::MASTER;
        sai_config.b_sync          = SaiHandle::Config::Sync::SLAVE;
        sai_config.a_dir           = SaiHandle::Config::Direction::RECEIVE;
        sai_config.b_dir           = SaiHandle::Config::Direction::TRANSMIT;
        sai_config.pin_config.fs   = {DSY_GPIOE, 4};
        sai_config.pin_config.mclk = {DSY_GPIOE, 2};
        sai_config.pin_config.sck  = {DSY_GPIOE, 5};
        sai_config.pin_config.sa   = {DSY_GPIOE, 6};
        sai_config.pin_config.sb   = {DSY_GPIOE, 3};
        SaiHandle sai_1_handle;
        sai_1_handle.Init(sai_config);
        I2CHandle::Config i2c_cfg;
        i2c_cfg.periph         = I2CHandle::Config::Peripheral::I2C_2;
        i2c_cfg.mode           = I2CHandle::Config::Mode::I2C_MASTER;
        i2c_cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
        i2c_cfg.pin_config.scl = {DSY_GPIOB, 10};
        i2c_cfg.pin_config.sda = {DSY_GPIOB, 11};
        I2CHandle i2c2;
        i2c2.Init(i2c_cfg);
        codec.Init(i2c2);

        AudioHandle::Config audio_config;
        audio_config.blocksize  = 48;
        audio_config.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
        audio_config.postgain   = 1.f;
        audio.Init(audio_config, sai_1_handle);
        callback_rate_ = AudioSampleRate() / AudioBlockSize();

        #ifdef PINKMAN_EXP
            /** ADC Init */
            AdcChannelConfig adc_config[9];
            /** Order of pins to match enum expectations */
            dsy_gpio_pin adc_pins[] = {
                PIN_ADC_CTRL_1,
                PIN_ADC_CTRL_2,
                PIN_ADC_CTRL_3,
                PIN_ADC_CTRL_4,
                PIN_ADC_CTRL_5,
                PIN_ADC_CTRL_6,
                PIN_ADC_CTRL_7,
                PIN_ADC_CTRL_8,
                PIN_ADC_CTRL_9
            };

            for(int i = 0; i < 9; i++)
            {
                
                if(i == 8) {
                    adc_config[i].InitMux(PIN_ADC_KNOB_MUX,
                                8,
                                PIN_MUX_A,
                                PIN_MUX_B,
                                PIN_MUX_C);
                } else {
                    adc_config[i].InitSingle(adc_pins[i]);
                }
                
            }
            adc.Init(adc_config, 9);
            int hw_pot_order[ADC_LAST-POT_1] = {POT_1-POT_1,POT_6-POT_1,POT_5-POT_1,POT_2-POT_1,POT_8-POT_1,POT_3-POT_1,POT_4-POT_1,POT_7-POT_1};
            /** Control Init */
            for(size_t i = 0; i < ADC_LAST; i++)
            {
                if(i < POT_1)
                    controls[i].InitBipolarCv(adc.GetPtr(i), callback_rate_);
                else
                    //controls[i].Init(adc.GetPtr(i), callback_rate_);
                    controls[i].Init(adc.GetMuxPtr(8, hw_pot_order[i-POT_1]), callback_rate_); 
            }
        #else

            /** ADC Init */
            AdcChannelConfig adc_config[5];
            /** Order of pins to match enum expectations */
            dsy_gpio_pin adc_pins[] = {
                PIN_ADC_CTRL_1,
                PIN_ADC_CTRL_2,
                PIN_ADC_CTRL_3,
                PIN_ADC_CTRL_4,
                PIN_ADC_CTRL_9
            };

            for(int i = 0; i < 5; i++)
            {
                
                if(i == 4) {
                    adc_config[i].InitMux(PIN_ADC_KNOB_MUX,
                                8,
                                PIN_MUX_A,
                                PIN_MUX_B,
                                PIN_MUX_C);
                } else {
                    adc_config[i].InitSingle(adc_pins[i]);
                }
                
            }
            adc.Init(adc_config, 5);
            int hw_pot_order[ADC_LAST-POT_1] = {POT_1-POT_1,POT_6-POT_1,POT_5-POT_1,POT_2-POT_1,POT_8-POT_1,POT_3-POT_1,POT_4-POT_1,POT_7-POT_1};
            /** Control Init */
            for(size_t i = 0; i < ADC_LAST; i++)
            {
                if(i < POT_1)
                    controls[i].InitBipolarCv(adc.GetPtr(i), callback_rate_);
                else
                    //controls[i].Init(adc.GetPtr(i), callback_rate_);
                    controls[i].Init(adc.GetMuxPtr(4, hw_pot_order[i-POT_1]), callback_rate_); 
            }

        #endif  // PINKMAN_EXP

        dsy_gpio_pin s_pins[S_LAST] = {BTN_BIG,BTN_RED,BTN_TOP};
        for (size_t i = 0; i < S_LAST; i++)
        {
            s[i].Init(s_pins[i]);
        }


        //dsy_gpio_pin led_pins[LED_LAST] = {LED1,LED2,LED3,LED4,LED5};
        //for (size_t i = 0; i < LED_LAST; i++)
        //{
        led2.Init(LED2,false);
        led3.Init(LED3,false);
        //}

        toggle.Init(SW_TOGGLE);

        
        //gate_in_1.Init((dsy_gpio_pin *)&DaisyPatchSM::B10);
        gate_in_1.Init((dsy_gpio_pin *)&B10);
        gate_in_2.Init((dsy_gpio_pin *)&B9);

        gate_out_1.mode = DSY_GPIO_MODE_OUTPUT_PP;
        gate_out_1.pull = DSY_GPIO_NOPULL;
        gate_out_1.pin  = B5;
        dsy_gpio_init(&gate_out_1);

        gate_out_2.mode = DSY_GPIO_MODE_OUTPUT_PP;
        gate_out_2.pull = DSY_GPIO_NOPULL;
        gate_out_2.pin  = B6;
        dsy_gpio_init(&gate_out_2);

        /** DAC init */
        pimpl_->InitDac();

        /** Start any background stuff */
        StartAdc();
        StartDac();
    }

    void DaisyPinkman::StartAudio(AudioHandle::AudioCallback cb)
    {
        audio.Start(cb);
    }

    void DaisyPinkman::StartAudio(AudioHandle::InterleavingAudioCallback cb)
    {
        audio.Start(cb);
    }

    void DaisyPinkman::ChangeAudioCallback(AudioHandle::AudioCallback cb)
    {
        audio.ChangeCallback(cb);
    }

    void
    DaisyPinkman::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
    {
        audio.ChangeCallback(cb);
    }

    void DaisyPinkman::StopAudio() { audio.Stop(); }

    void DaisyPinkman::SetAudioBlockSize(size_t size)
    {
        audio.SetBlockSize(size);
        callback_rate_ = AudioSampleRate() / AudioBlockSize();
        for(size_t i = 0; i < ADC_LAST; i++)
        {
            controls[i].SetSampleRate(callback_rate_);
        }
    }

    void DaisyPinkman::SetAudioSampleRate(float sr)
    {
        SaiHandle::Config::SampleRate sai_sr;
        switch(int(sr))
        {
            case 8000: sai_sr = SaiHandle::Config::SampleRate::SAI_8KHZ; break;
            case 16000:
                sai_sr = SaiHandle::Config::SampleRate::SAI_16KHZ;
                break;
            case 32000:
                sai_sr = SaiHandle::Config::SampleRate::SAI_32KHZ;
                break;
            case 48000:
                sai_sr = SaiHandle::Config::SampleRate::SAI_48KHZ;
                break;
            case 96000:
                sai_sr = SaiHandle::Config::SampleRate::SAI_96KHZ;
                break;
            default: sai_sr = SaiHandle::Config::SampleRate::SAI_48KHZ; break;
        }
        audio.SetSampleRate(sai_sr);
        callback_rate_ = AudioSampleRate() / AudioBlockSize();
        for(size_t i = 0; i < ADC_LAST; i++)
        {
            controls[i].SetSampleRate(callback_rate_);
        }
    }

    void
    DaisyPinkman::SetAudioSampleRate(SaiHandle::Config::SampleRate sample_rate)
    {
        audio.SetSampleRate(sample_rate);
        callback_rate_ = AudioSampleRate() / AudioBlockSize();
        for(size_t i = 0; i < ADC_LAST; i++)
        {
            controls[i].SetSampleRate(callback_rate_);
        }
    }

    size_t DaisyPinkman::AudioBlockSize()
    {
        return audio.GetConfig().blocksize;
    }

    float DaisyPinkman::AudioSampleRate() { return audio.GetSampleRate(); }

    float DaisyPinkman::AudioCallbackRate() { return callback_rate_; }

    void DaisyPinkman::StartAdc() { adc.Start(); }

    void DaisyPinkman::StopAdc() { adc.Stop(); }

    void DaisyPinkman::ProcessAnalogControls()
    {
        for(int i = 0; i < ADC_LAST; i++)
        {
            controls[i].Process();
        }
    }

    void DaisyPinkman::ProcessDigitalControls() {}

    float DaisyPinkman::GetAdcValue(int idx) { return controls[idx].Value(); }

    dsy_gpio_pin DaisyPinkman::GetPin(const PinBank bank, const int idx)
    {
        if(idx <= 0 || idx > 10)
            return DUMMYPIN;
        else
            return kPinMap[static_cast<int>(bank)][idx - 1];
    }

    void DaisyPinkman::StartDac(DacHandle::DacCallback callback)
    {
        pimpl_->StartDac(callback);
    }

    void DaisyPinkman::StopDac() { pimpl_->StopDac(); }

    void DaisyPinkman::WriteCvOut(const int channel, float voltage)
    {
        pimpl_->WriteCvOut(channel, voltage);
    }

    void DaisyPinkman::SetLed(Leds idx,bool state) { 
        
        switch (idx)
        {
        case LED_1:
            WriteCvOut(CV_OUT_1,state ? 5.f : 0.f);
            break;
        case LED_2:
            led2.Set(state ? 1.f : 0.f);
            break;
        case LED_3:
            led3.Set(state ? 1.f : 0.f);
            break;
        case LED_4:
            dsy_gpio_write(&gate_out_1, state);
            break;
        case LED_5:
            dsy_gpio_write(&gate_out_2, state);
            break;
        }
        
    }

    void DaisyPinkman::UpdateLeds() {
        led2.Update();
        led3.Update();
    } 

    bool DaisyPinkman::ValidateSDRAM()
    {
        uint32_t *sdramptr      = (uint32_t *)0xc0000000;
        uint32_t  size_in_words = 16777216;
        uint32_t  testval       = 0xdeadbeef;
        uint32_t  num_failed    = 0;
        /** Write test val */
        for(uint32_t i = 0; i < size_in_words; i++)
        {
            uint32_t *word = sdramptr + i;
            *word          = testval;
        }
        /** Compare written */
        for(uint32_t i = 0; i < size_in_words; i++)
        {
            uint32_t *word = sdramptr + i;
            if(*word != testval)
                num_failed++;
        }
        /** Write Zeroes */
        for(uint32_t i = 0; i < size_in_words; i++)
        {
            uint32_t *word = sdramptr + i;
            *word          = 0x00000000;
        }
        /** Compare Cleared */
        for(uint32_t i = 0; i < size_in_words; i++)
        {
            uint32_t *word = sdramptr + i;
            if(*word != 0)
                num_failed++;
        }
        return num_failed == 0;
    }

    bool DaisyPinkman::ValidateQSPI(bool quick)
    {
        uint32_t start;
        uint32_t size;
        if(quick)
        {
            start = 0x400000;
            size  = 0x4000;
        }
        else
        {
            start = 0;
            size  = 0x800000;
        }
        // Erase the section to be tested
        qspi.Erase(start, start + size);
        // Create some test data
        std::vector<uint8_t> test;
        test.resize(size);
        uint8_t *testmem = test.data();
        for(size_t i = 0; i < size; i++)
            testmem[i] = (uint8_t)(i & 0xff);
        // Write the test data to the device
        qspi.Write(start, size, testmem);
        // Read it all back and count any/all errors
        // I supppose any byte where ((data & 0xff) == data)
        // would be able to false-pass..
        size_t fail_cnt = 0;
        for(size_t i = 0; i < size; i++)
            if(testmem[i] != (uint8_t)(i & 0xff))
                fail_cnt++;
        return fail_cnt == 0;
    }

    float DaisyPinkman::CVKnobCombo(float CV_Val,float Pot_Val)
    {
        float output{};
        output = CV_Val + Pot_Val;

        if(output < 0.0f)
        {
            output = 0.0f;
        }

        if(output > 1.0f)
        {
            output = 1.0f;
        }

        return output;
    }

} // namespace pinkman

} // namespace daisy