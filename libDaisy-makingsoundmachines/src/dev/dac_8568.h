#pragma once
#ifndef DSY_DEV_DAC_8568_H
#define DSY_DEV_DAC_8568_H /**< Macro */
#include <stdlib.h>
#include <stdint.h>
#include "daisy_core.h"


namespace daisy
{
/** @addtogroup analog_digital_conversion
    @{ 
*/

/** 
    Driver for DAC8568, allowing for CV Out using a TI DAC 8568
    Based on Code from Westlicht Performer   - https://westlicht.github.io/performer/
    Port for Daisy by Making Sound Machines  - https://github.com/makingsoundmachines

    You will need adjusted SPI settings for this to work:

    In libdaisy\src\per\spi.cpp

    hspi1.Init.CLKPolarity       = SPI_POLARITY_HIGH; // was SPI_POLARITY_LOW;
    hspi1.Init.NSS               = SPI_NSS_SOFT; // was SPI_NSS_HARD_OUTPUT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; // was SPI_BAUDRATEPRESCALER_8;
*/

class Dac8568
{
  public:
    /** GPIO Pins that need to be used independent of peripheral used. */
    enum Pins
    {
        SYNC,     /**< Sync pin. */
        NUM_PINS, /**< Num pins */
    };

    enum class Type
    {
        DAC8568C,
        DAC8568A
    };

    static constexpr int Channels = 8; // #define CONFIG_DAC_CHANNELS 8

    typedef uint16_t Value;

    Dac8568(Type type = Type::DAC8568C)
    {
        switch(type)
        {
            case Type::DAC8568C: _dataShift = 0; break;
            case Type::DAC8568A: _dataShift = 1; break;
        }
    }
    ~Dac8568() {}


    // configuration currently only uses SPI1, w/ soft chip select.

    /** 
    Takes an argument for the pin cfg
    \param pin_cfg should be a pointer to an array of Dac8568::NUM_PINS dsy_gpio_pins
    */
    void Init(dsy_gpio_pin* pin_cfg);

    void Set(int channel, Value value) { _values[channel] = value; }

    void Write(int channel);
    void Write();

  private:
    void WriteDac8568(uint8_t  command,
                      uint8_t  address,
                      uint16_t data,
                      uint8_t  function);

    void Reset();
    void SetInternalRef(bool enabled);

    enum ClearCode
    {
        ClearZeroScale = 0,
        ClearMidScale  = 1,
        ClearFullScale = 2,
        ClearIgnore    = 3,
    };

    void SetClearCode(ClearCode code);

    Value    _values[Channels];
    uint32_t _dataShift = 0;
};
/** @} */
} // namespace daisy

#endif
