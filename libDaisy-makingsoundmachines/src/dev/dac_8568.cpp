#include "dac_8568.h"
#include "../daisy_core.h"
#include "../per/spi.h"
#include "../per/gpio.h"
#include "../sys/system.h"

// Driver for DAC8568
// Based on Code from Westlicht Performer   - https://westlicht.github.io/performer/
// Port for Daisy by Making Sound Machines  - https://github.com/makingsoundmachines

// Pin Names in Schematics
//
// Patch    Seed        STM32H7   DAC8568
//
// SCK      SPI1_SCK    PG11      SCLK - spi.init GPIO_InitStruct.Pin = GPIO_PIN_11;
// DATA     SPI1_MOSI   PB5       DIN  - spi.init GPIO_InitStruct.Pin = GPIO_PIN_5
// RESET    USB_HS_D+   PB15      n.c.
// CMD      SPI1_MISO   PB4       SYNC
// CS       SPI1_NSS    PG10      n.c.


// Commands

#define WRITE_INPUT_REGISTER 0
#define UPDATE_OUTPUT_REGISTER 1
#define WRITE_INPUT_REGISTER_UPDATE_ALL 2
#define WRITE_INPUT_REGISTER_UPDATE_N 3
#define POWER_DOWN_UP_DAC 4
#define LOAD_CLEAR_CODE_REGISTER 5
#define LOAD_LDAC_REGISTER 6
#define RESET_POWER_ON 7
#define SETUP_INTERNAL_REF 8


using namespace daisy;

//static uint8_t Dac8568_Buffer[128];

typedef struct
{
    uint8_t Initialized;
} Dac8568_t;

static SpiHandle         h_spi;
static dsy_gpio          pin_sync;
static Dac8568_t         Dac8568_;
static SpiHandle::Config spi_config;

void Dac8568::Init(dsy_gpio_pin* pin_cfg)
{
    // Initialize GPIO
    pin_sync.mode = DSY_GPIO_MODE_OUTPUT_PP;
    pin_sync.pin  = pin_cfg[Dac8568::SYNC];
    dsy_gpio_init(&pin_sync);

    // Initialize SPI

    spi_config.periph         = SpiHandle::Config::Peripheral::SPI_1;
    spi_config.mode           = SpiHandle::Config::Mode::MASTER;
    spi_config.direction      = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
    spi_config.datasize       = 8;
    spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::HIGH;
    spi_config.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
    spi_config.nss            = SpiHandle::Config::NSS::SOFT;
    spi_config.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_2;

    spi_config.pin_config.sclk = {DSY_GPIOG, 11};
    spi_config.pin_config.miso = {DSY_GPIOB, 4};
    spi_config.pin_config.mosi = {DSY_GPIOB, 5};
    spi_config.pin_config.nss  = {DSY_GPIOG, 10}; //{DSY_GPIOX, 0};

    h_spi.Init(spi_config);


    // initialize DAC8568
    Reset();
    SetClearCode(ClearIgnore);
    SetInternalRef(
        false); // was true - External Ref is 1.65V so VDD = FS Out is 3.3V
    WriteDac8568(POWER_DOWN_UP_DAC, 0, 0, 0xff);

    Dac8568_.Initialized = 1;
}

void Dac8568::Write(int channel)
{
    WriteDac8568(WRITE_INPUT_REGISTER_UPDATE_N, channel, _values[channel], 15);
}

void Dac8568::Write()
{
    for(int channel = 0; channel < Channels; ++channel)
    {
        WriteDac8568(channel == 7 ? WRITE_INPUT_REGISTER_UPDATE_ALL
                                  : WRITE_INPUT_REGISTER,
                     channel,
                     _values[channel],
                     0);
    }
}

void Dac8568::WriteDac8568(uint8_t  command,
                           uint8_t  address,
                           uint16_t data,
                           uint8_t  function)
{
    // Shift data by one bit for DAC8568A
    data <<= _dataShift;

    uint8_t b[4];

    b[0] = command;
    b[1] = (address << 4) | (data >> 12);
    b[2] = data >> 4;
    b[3] = (data & 0xf) << 4 | function;

    dsy_gpio_write(&pin_sync, 0);

    // On Daisy, this seems to work without the Delays
    // hal::Delay::delay_ns<13>(); // t5 in timing diagram
    // System::DelayTicks(7);

    h_spi.BlockingTransmit(b, 4);

    // On Daisy, this seems to work without the Delays
    // hal::Delay::delay_ns<200>();
    // System::DelayTicks(100);

    dsy_gpio_write(&pin_sync, 1);

    // On Daisy, this seems to work without the Delays
    // hal::Delay::delay_ns<80>(); // t4 in timing diagram
    // System::DelayTicks(40);
}

void Dac8568::Reset()
{
    WriteDac8568(RESET_POWER_ON, 0, 0, 0);
    // hal::Delay::delay_us(50);
}

void Dac8568::SetInternalRef(bool enabled)
{
    WriteDac8568(SETUP_INTERNAL_REF, 0, 0, enabled ? 1 : 0);
}

void Dac8568::SetClearCode(ClearCode code)
{
    WriteDac8568(LOAD_CLEAR_CODE_REGISTER, 0, 0, code);
}
