#pragma once
#ifndef DSY_DEV_LED_IS31FL3731_DMA_H
#define DSY_DEV_LED_IS31FL3731_DMA_H /**< & */

#ifdef __cplusplus

#include <stdint.h>
#include "per/i2c.h"
#include "per/gpio.h"

namespace daisy
{
/** LED driver for one or multiple Is31fl3731 8it PWM chips connected to
 * a single I2C peripheral.
 * It includes gamma correction from 8bit brightness values but it 
 * can also be supplied with raw 8bit values.
 * This driver uses two buffers - one for drawing, one for transmitting.
 * Multiple LedDriverIs31fl3731 instances can be used at the same time.
 * \param Is31fl3731_numDrivers    The number of Is31fl3731 driver attached to the I2C
 *                      peripheral.
 * \param Is31fl3731_persistentBufferContents If set to true, the current draw buffer 
 *                      contents will be copied to the next draw buffer 
 *                      during SwapBuffersAndTransmit(). Use this, if you plan
 *                      to write single leds at a time. 
 *                      If you will alway update all leds before calling 
 *                      SwapBuffersAndTransmit(), you can set this to false
 *                      and safe some cycles.
 */
template <int Is31fl3731_numDrivers, bool Is31fl3731_persistentBufferContents = true>
class LedDriverIs31fl3731
{
  public:
    /** Buffer Type for a single Is31fl3731 driver chip. */
    /** struct __attribute__((packed)) Is31fl3731TransmitBuffer */

    struct Is31fl3731TransmitBuffer
    {
        /** data */
        uint8_t data[145];  // 147
    };
    /** Buffer type for the entire DMA buffer. */
    using DmaBuffer = Is31fl3731TransmitBuffer[Is31fl3731_numDrivers];

    /** Initialises the driver. 
     * \param i2c           The I2C peripheral to use.
     * \param addresses     An array of addresses for each of the driver chips.
     * \param dma_buffer_a  The first buffer for the DMA. This must be placed in 
     *                      D2 memory by adding the DMA_BUFFER_MEM_SECTION attribute
     *                      like this: `LedDriverIs31fl3731<2>::DmaBuffer DMA_BUFFER_MEM_SECTION bufferA;`
     * \param dma_buffer_b  The second buffer for the DMA. This must be placed in 
     *                      D2 memory by adding the DMA_BUFFER_MEM_SECTION attribute
     *                      like this: `LedDriverIs31fl3731<2>::DmaBuffer DMA_BUFFER_MEM_SECTION bufferB;`
     * \param oe_pin        If the output enable pin is used, supply its configuration here.
     *                      It will automatically be pulled low by the driver.
    */
    void Init(I2CHandle i2c,
              const uint8_t (&addresses)[Is31fl3731_numDrivers],
              DmaBuffer    dma_buffer_a,
              DmaBuffer    dma_buffer_b,
              dsy_gpio_pin oe_pin = {DSY_GPIOX, 0})
    {
        i2c_             = i2c;
        draw_buffer_     = dma_buffer_a;
        transmit_buffer_ = dma_buffer_b;
        oe_pin_          = oe_pin;
        for(int d = 0; d < Is31fl3731_numDrivers; d++)
            addresses_[d] = addresses[d];
        current_driver_idx_ = -1;

        InitializeBuffers();
        InitializeDrivers();
    }

    /** Returns the number of leds available from this driver. */
    constexpr int GetNumLeds() const { return Is31fl3731_numDrivers * 144; }

    /** Sets all leds to a gamma corrected brightness between 0.0f and 1.0f. */
    void SetAllTo(float brightness)
    {
        const uint8_t intBrightness
            = (uint8_t)(clamp(brightness * 255.0f, 0.0f, 255.0f));
        SetAllTo(intBrightness);
    }

    /** Sets all leds to a gamma corrected brightness between 0 and 255. */
    void SetAllTo(uint8_t brightness)
    {
        const uint16_t cycles = gamma_table_[brightness];
        SetAllToRaw(cycles);
    }

    /** Sets all leds to a raw 12bit brightness between 0 and 4095. */
    void SetAllToRaw(uint8_t rawBrightness)
    {
        for(int led = 0; led < GetNumLeds(); led++)
            SetLedRaw(led, rawBrightness);
    }

    /** Sets a single led to a gamma corrected brightness between 0.0f and 1.0f. */
    void SetLed(int ledIndex, float brightness)
    {
        const uint8_t intBrightness
            = (uint8_t)(clamp(brightness * 255.0f, 0.0f, 255.0f));
        SetLed(ledIndex, intBrightness);
    }

    /** Sets a single led to a gamma corrected brightness between 0 and 255. */
    void SetLed(int ledIndex, uint8_t brightness)
    {
        const uint16_t cycles = gamma_table_[brightness];
        SetLedRaw(ledIndex, cycles);
    }

    /** Sets a single led to a raw 12bit brightness between 0 and 4095. */
    void SetLedRaw(int ledIndex, uint8_t rawBrightness)
    {
        const auto d  = GetDriverForLed(ledIndex);
        const auto ch = GetDriverChannelForLed(ledIndex);

        //draw_buffer_[d].data[0]          = 0xFD;
        //draw_buffer_[d].data[1]          = 0x00;
        draw_buffer_[d].data[0]          = 0x24; //Is31fl3731_LED0;

        // clip raw brightness at 0xFF
        draw_buffer_[d].data[ch+1] = rawBrightness & 0xFF;
    }

    /** Sets a line of 16 LED to a 16 raw 8bit brightness from an array with values between 0x00 and 0xFF */
    /** clips at the driver's LED limit of 144 leds */
    void Set16Raw(int8_t driver_id, uint8_t first_pixel, uint8_t data[16])
    {
        const auto d  = driver_id;
        const auto ch = first_pixel + 1; // first byte is 0x24 

        // write 16 bytes at once
        for(uint8_t i = 0; i < 16; i++)
        {
            if( first_pixel + i >= 144) return;
            // clip raw brightness at 0xFF
            draw_buffer_[d].data[ch+i] = data[i] & 0xFF;
        }
    }    

    void Set1Raw(int8_t driver_id, uint8_t first_pixel, uint8_t data)
    {
        const auto d  = driver_id;
        const auto ch = first_pixel + 1; // first byte is 0x24 

        // write one byte
        if( ch > 144) return;
        // clip raw brightness at 0xFF
        draw_buffer_[d].data[ch] = data & 0xFF;
    }


    /** Swaps the current draw buffer and the current transmit buffer and
     *  starts transmitting the values to all chips.
     */
    void SwapBuffersAndTransmit()
    {
        // wait for current transmission to complete
        while(current_driver_idx_ >= 0) {};

        // swap buffers
        auto tmp         = transmit_buffer_;
        transmit_buffer_ = draw_buffer_;
        draw_buffer_     = tmp;

        // copy current transmit buffer contents to the new draw buffer
        // to keep the led settings (if required)
        if(Is31fl3731_persistentBufferContents)
        {
            for(int d = 0; d < Is31fl3731_numDrivers; d++)
                for(int ch = 0; ch < 145; ch++)
                    draw_buffer_[d].data[ch]
                        = transmit_buffer_[d].data[ch];
        }

        // start transmission
        current_driver_idx_ = -1;
        ContinueTransmission();
    }

  private:
    void ContinueTransmission()
    {
        current_driver_idx_++;
        if(current_driver_idx_ >= Is31fl3731_numDrivers)
        {
            current_driver_idx_ = -1;
            return;
        }

        const auto    d       = current_driver_idx_;
        const uint8_t address = Is31fl3731_I2C_BASE_ADDRESS | addresses_[d];
        const auto    status  = i2c_.TransmitDma(address,
                                             (uint8_t*)&transmit_buffer_[d],
                                             145,
                                             &TxCpltCallback,
                                             this);
        if(status != I2CHandle::Result::OK)
        {
            // TODO: fix this :-)
            // Reinit I2C (probably a flag to kill, but hey this works fairly well for now.)
            i2c_.Init(i2c_.GetConfig());
        }
    }
    /*uint16_t GetStartCycleForLed(int ledIndex) const
    {
        return (ledIndex << 2) & 0x0FFF; // shift each led by 4 cycles
    }*/

    uint8_t GetDriverForLed(int ledIndex) const {  

        // return driver for every 144 LEDs
        
        /* int n = ledIndex;
        n = n + 0x70;
        n = n & 0xFF;
        n = n >> 8;
        return n; */

        return (uint8_t)(ledIndex / 144);    
    }

    uint8_t GetDriverChannelForLed(int ledIndex) const
    {
        // constrain to 
        // int 0    - 143 
        // hex 0x00 - 0x8F

        /* int n = ledIndex;
        n = n + 0x70;
        n = n & 0xFF;
        n = n - 0x70;
        return n; */

        return ledIndex % 144;
    }

    void InitializeBuffers()
    {
        for(int d = 0; d < Is31fl3731_numDrivers; d++)
        {
            for(uint16_t led = 0; led < 145; led++)
            {
                draw_buffer_[d].data[led]     = 0x00;
                transmit_buffer_[d].data[led] = 0x00;
            }

            draw_buffer_[d].data[0]     = 0x24;
            transmit_buffer_[d].data[0] = 0x24;
        }

        /* for(int led = 0; led < GetNumLeds(); led++)
        {
            const auto d                     = GetDriverForLed(led);
            const auto ch                    = GetDriverChannelForLed(led);
            //const auto startCycle            = GetStartCycleForLed(led);

            draw_buffer_[d].leds[ch].pwm     = 0x00;
            transmit_buffer_[d].leds[ch].pwm = 0x00;
        } */
    }

    void InitializeDrivers()
    {
        // init OE pin and pull low to enable outputs
        if(oe_pin_.port != DSY_GPIOX)
        {
            oe_pin_gpio_.pin  = oe_pin_;
            oe_pin_gpio_.mode = DSY_GPIO_MODE_OUTPUT_PP;
            oe_pin_gpio_.pull = DSY_GPIO_NOPULL;
            dsy_gpio_init(&oe_pin_gpio_);
            dsy_gpio_write(&oe_pin_gpio_, 0);
        }

        // init the individual drivers
        // Init procedure used here is from the ISSI example code.
        for(int d = 0; d < Is31fl3731_numDrivers; d++)
        {
            const uint8_t address = Is31fl3731_I2C_BASE_ADDRESS | addresses_[d];
            uint8_t       buffer[2];

            // shutdown
            // 0xFD, 0x0B  write function register
            buffer[0] = 0xFD;
            buffer[1] = 0x0B;
            i2c_.TransmitBlocking(address, buffer, 2, 1);
            // 0x0A, 0x00  enter software shutdown mode
            buffer[0] = 0x0A;
            buffer[1] = 0x00;
            i2c_.TransmitBlocking(address, buffer, 2, 1);

            System::Delay(10);

            // out of shutdown
            // 0xFD, 0x0B  write function register
            buffer[0] = 0xFD;
            buffer[1] = 0x0B;
            i2c_.TransmitBlocking(address, buffer, 2, 1);
            // 0x0A, 0x00  exit software shutdown mode
            buffer[0] = 0x0A;
            buffer[1] = 0x01;
            i2c_.TransmitBlocking(address, buffer, 2, 1);

            // picture mode
            // 0xFD, 0x0B  write function register
            buffer[0] = 0xFD;
            buffer[1] = 0x0B;
            i2c_.TransmitBlocking(address, buffer, 2, 1);
            // 0x00, 0x00  picture mode
            buffer[0] = 0x00;
            buffer[1] = 0x00;
            i2c_.TransmitBlocking(address, buffer, 2, 1);

            // display first Frame
            // 0xFD, 0x0B  write function register
            buffer[0] = 0xFD;
            buffer[1] = 0x0B;
            i2c_.TransmitBlocking(address, buffer, 2, 1);
            // 0x01, 0x00  select first frame
            buffer[0] = 0x01;
            buffer[1] = 0x00;
            i2c_.TransmitBlocking(address, buffer, 2, 1);
            //System::Delay(20);


            // 0xFD, 0x00 select Bank
            buffer[0] = 0xFD;
            buffer[1] = 0x00;
            
            i2c_.TransmitBlocking(address, buffer, 2, 1);

            //turn on all LED
            for(uint8_t k = 0; k < 0x12; k++)
            {
                buffer[0] = k;
                buffer[1] = 0xff;

                i2c_.TransmitBlocking(address, buffer, 2, 1);
            }

            // 0xFD, 0x00 select Bank
            buffer[0] = 0xFD;
            buffer[1] = 0x00;
            //Need to turn off the position where LED is not mounted
            //write all PWM set 0x00
            for(uint8_t k = 0x24; k < 0xB4; k++)
            {
                buffer[0] = k;
                buffer[1] = 0x00;

                i2c_.TransmitBlocking(address, buffer, 2, 1);
            }

            // 0x0A, 0x01  normal operation
            //buffer[0] = 0x0A;
            //buffer[1] = 0x01;
            //i2c_.TransmitBlocking(address, buffer, 2, 1);
            //System::Delay(20);

            // 0xFD, 0x00  write first frame
            buffer[0] = 0xFD;
            buffer[1] = 0x00;
            i2c_.TransmitBlocking(address, buffer, 2, 1);

            
            //Test: write all PWM set 0x80
            /*for(uint8_t k = 0x24; k < 0xB4; k++)
            {
                buffer[0] = k;
                buffer[1] = 0x80;

                i2c_.TransmitBlocking(address, buffer, 2, 1);
            }*/

            //System::DelayUs(80);
        }
    }

    // no std::clamp available in C++14.... remove this when C++17 is available
    template <typename T>
    T clamp(T in, T low, T high)
    {
        return (in < low) ? low : (high < in) ? high : in;
    }

    // an internal function to handle i2c callbacks
    // called when an I2C transmission completes and the next driver must be updated
    static void TxCpltCallback(void* context, I2CHandle::Result result)
    {
        auto drv_ptr = reinterpret_cast<
            LedDriverIs31fl3731<Is31fl3731_numDrivers, Is31fl3731_persistentBufferContents>*>(context);
        drv_ptr->ContinueTransmission();
    }

    I2CHandle              i2c_;
    Is31fl3731TransmitBuffer* draw_buffer_;
    Is31fl3731TransmitBuffer* transmit_buffer_;
    uint8_t                addresses_[Is31fl3731_numDrivers];
    dsy_gpio_pin           oe_pin_;
    dsy_gpio               oe_pin_gpio_;
    // index of the dirver that is currently updated.
    volatile int8_t current_driver_idx_;
    const uint16_t  gamma_table_[256] = {
        0,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        2,    2,    2,    2,    2,    2,    2,    3,    3,    4,    4,    5,
        5,    6,    7,    8,    8,    9,    10,   11,   12,   13,   15,   16,
        17,   18,   20,   21,   23,   25,   26,   28,   30,   32,   34,   36,
        38,   40,   43,   45,   48,   50,   53,   56,   59,   62,   65,   68,
        71,   75,   78,   82,   85,   89,   93,   97,   101,  105,  110,  114,
        119,  123,  128,  133,  138,  143,  149,  154,  159,  165,  171,  177,
        183,  189,  195,  202,  208,  215,  222,  229,  236,  243,  250,  258,
        266,  273,  281,  290,  298,  306,  315,  324,  332,  341,  351,  360,
        369,  379,  389,  399,  409,  419,  430,  440,  451,  462,  473,  485,
        496,  508,  520,  532,  544,  556,  569,  582,  594,  608,  621,  634,
        648,  662,  676,  690,  704,  719,  734,  749,  764,  779,  795,  811,
        827,  843,  859,  876,  893,  910,  927,  944,  962,  980,  998,  1016,
        1034, 1053, 1072, 1091, 1110, 1130, 1150, 1170, 1190, 1210, 1231, 1252,
        1273, 1294, 1316, 1338, 1360, 1382, 1404, 1427, 1450, 1473, 1497, 1520,
        1544, 1568, 1593, 1617, 1642, 1667, 1693, 1718, 1744, 1770, 1797, 1823,
        1850, 1877, 1905, 1932, 1960, 1988, 2017, 2045, 2074, 2103, 2133, 2162,
        2192, 2223, 2253, 2284, 2315, 2346, 2378, 2410, 2442, 2474, 2507, 2540,
        2573, 2606, 2640, 2674, 2708, 2743, 2778, 2813, 2849, 2884, 2920, 2957,
        2993, 3030, 3067, 3105, 3143, 3181, 3219, 3258, 3297, 3336, 3376, 3416,
        3456, 3496, 3537, 3578, 3619, 3661, 3703, 3745, 3788, 3831, 3874, 3918,
        3962, 4006, 4050, 4095};

    static constexpr uint8_t Is31fl3731_I2C_BASE_ADDRESS = 0b01110100;  // 0x74
    static constexpr uint8_t Is31fl3731_LED0 = 0x24; // location for start of LED0 registers
    static constexpr uint8_t PRE_SCALE_MODE = 0xFE; //location for setting prescale (clock speed)

    static constexpr uint8_t  Is31fl3731_REG_CONFIG = 0x00;
    static constexpr uint8_t  Is31fl3731_REG_CONFIG_PICTUREMODE = 0x00;
    static constexpr uint8_t  Is31fl3731_REG_CONFIG_AUTOPLAYMODE = 0x08;
    static constexpr uint8_t  Is31fl3731_REG_CONFIG_AUDIOPLAYMODE = 0x18;

    static constexpr uint8_t  Is31fl3731_CONF_PICTUREMODE = 0x00;
    static constexpr uint8_t  Is31fl3731_CONF_AUTOFRAMEMODE = 0x04;
    static constexpr uint8_t  Is31fl3731_CONF_AUDIOMODE = 0x08;

    static constexpr uint8_t  Is31fl3731_REG_PICTUREFRAME = 0x01;

    static constexpr uint8_t  Is31fl3731_REG_SHUTDOWN = 0x0A;
    static constexpr uint8_t  Is31fl3731_REG_AUDIOSYNC = 0x06;

    static constexpr uint8_t  Is31fl3731_COMMANDREGISTER = 0xFD;
    static constexpr uint8_t  Is31fl3731_BANK_FUNCTIONREG = 0x0B; // helpfully called 'page nine'
};

} // namespace daisy

#endif
#endif
/** @} */
