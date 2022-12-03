#include "led_is31fl3731.h"
#include "../daisy_core.h"
#include "../per/i2c.h"
#include "../per/gpio.h"
#include "../sys/system.h"


// Driver for ISSI IS31FL3731 144 ch PWM LED Matrix
// Ported from the Adafruit IS31FL3731 library - https://github.com/adafruit/Adafruit_IS31FL3731/
// Port for Daisy by Making Sound Machines     - https://github.com/makingsoundmachines

// Pin Names in Schematics
//
// Patch       Seed           STM32H7   IS31FL3731
//
// PIN_ENC_B   I2C1_SDA  11   PB8       SDA
// PIN_ENC_A   I2C1_SCL  12   PB9       SCL


#ifndef _swap_int16_t
#define _swap_int16_t(a, b) \
    {                       \
        int16_t t = a;      \
        a         = b;      \
        b         = t;      \
    }
#endif

// Commands

#define ISSI_ADDR_DEFAULT 0b01110100 // 0x74

#define ISSI_REG_CONFIG 0x00
#define ISSI_REG_CONFIG_PICTUREMODE 0x00
#define ISSI_REG_CONFIG_AUTOPLAYMODE 0x08
#define ISSI_REG_CONFIG_AUDIOPLAYMODE 0x18

#define ISSI_CONF_PICTUREMODE 0x00
#define ISSI_CONF_AUTOFRAMEMODE 0x04
#define ISSI_CONF_AUDIOMODE 0x08

#define ISSI_REG_PICTUREFRAME 0x01

#define ISSI_REG_SHUTDOWN 0x0A
#define ISSI_REG_AUDIOSYNC 0x06

#define ISSI_COMMANDREGISTER 0xFD
#define ISSI_BANK_FUNCTIONREG 0x0B // helpfully called 'page nine'


using namespace daisy;

//static uint8_t Dac8568_Buffer[128];

typedef struct
{
    uint8_t Initialized;
} Is31fl3731_t;


static Is31fl3731_t Is31fl3731_;
static I2CHandle    i2c_;


void Is31fl3731::Init(const I2CHandle i2c,
                      const uint8_t*  addresses,
                      uint8_t         numDrivers)
{
    i2c_        = i2c;
    numDrivers_ = numDrivers;
    for(int d = 0; d < numDrivers_; d++)
        addresses_[d] = addresses[d];

    // Init procedure used here is from the ISSI example code.

    // init the individual drivers
    for(int d = 0; d < numDrivers_; d++)
    {
        WriteIs31fl3731(
            addresses_[d], 0xFD, 0x0B); // 0xFD, 0x0B  write function register
        WriteIs31fl3731(addresses_[d],
                        0x0A,
                        0x00); // 0x0A, 0x00  enter software shutdown mode
        WriteIs31fl3731(
            addresses_[d], 0xFD, 0x00); // 0xFD, 0x00  write first frame

        //turn on all LED
        for(uint8_t k = 0; k < 0x12; k++)
        {
            WriteIs31fl3731(addresses_[d], k, 0xff);
        }

        //Need to turn off the position where LED is not mounted
        //write all PWM set 0x00
        for(uint8_t k = 0x24; k < 0xB4; k++)
        {
            WriteIs31fl3731(addresses_[d], k, 0x00);
        }

        WriteIs31fl3731(
            addresses_[d], 0xFD, 0x0B); // 0xFD, 0x0B  write function register
        WriteIs31fl3731(addresses_[d], 0x00, 0x00); // 0x00, 0x00  picture mode
        WriteIs31fl3731(
            addresses_[d], 0x01, 0x00); // 0x01, 0x00  select first frame
        WriteIs31fl3731(
            addresses_[d], 0x0A, 0x01); // 0x0A, 0x01  normal operation
        WriteIs31fl3731(
            addresses_[d], 0xFD, 0x00); // 0xFD, 0x00  write first frame

        //write all PWM set 0x80
        /*for(uint8_t k = 0x24; k < 0xB4; k++)
        {
            i2c_buff[0] = k;
            i2c_buff[1] = 0x80;
            i2c.TransmitBlocking(IS31FL3731_ADDRESS, i2c_buff, 2, 1000);
        }*/
        System::DelayUs(20); // wait for clear
    }

    Is31fl3731_.Initialized = 1;
}


void Is31fl3731::WriteIs31fl3731(uint8_t i2c_address, uint8_t reg, uint8_t data)
{
    uint8_t b[2];

    b[0] = reg;
    b[1] = data;

    i2c_.TransmitBlocking(i2c_address, b, 2, 1); // 1000
}


void Is31fl3731::Test_mode()
{
    for(uint8_t j = 0x00; j < 0xFF; j++) //all LED ramping up
    {
        for(uint8_t i = 0x24; i < 0xB4; i++)
        {
            for(int d = 0; d < numDrivers_; d++)
            {
                WriteIs31fl3731(addresses_[d], i, j);
            }
        }
    }

    for(uint8_t j = 0xFF; j > 0x00; j--) //all LED ramping down
    {
        for(uint8_t i = 0x24; i < 0xB4; i++)
        {
            for(int d = 0; d < numDrivers_; d++)
            {
                WriteIs31fl3731(addresses_[d], i, j);
            }
        }
    }
}


void Is31fl3731::Begin(const I2CHandle i2c,
                       const uint8_t*  addresses,
                       uint8_t         numDrivers)
{
    i2c_        = i2c;
    numDrivers_ = numDrivers;
    for(int d = 0; d < numDrivers_; d++)
        addresses_[d] = addresses[d];

    _width = WIDTH = 16;
    _height = HEIGHT = 9;
    rotation         = 0;

    // init the individual drivers
    for(int d = 0; d < numDrivers_; d++)
    {
        // shutdown
        WriteRegister8(
            addresses_[d], ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x00);
        //delay(10);

        // out of shutdown
        WriteRegister8(
            addresses_[d], ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x01);

        // picture mode
        WriteRegister8(addresses_[d],
                       ISSI_BANK_FUNCTIONREG,
                       ISSI_REG_CONFIG,
                       ISSI_REG_CONFIG_PICTUREMODE);

        DisplayFrame(addresses_[d], _frame);

        // all LEDs on & 0 PWM
        Clear(addresses_[d]); // set each led to 0 PWM

        for(uint8_t f = 0; f < 8; f++)
        {
            for(uint8_t i = 0; i <= 0x11; i++)
                WriteRegister8(addresses_[d], f, i, 0xff); // each 8 LEDs on
        }
        AudioSync(addresses_[d], false);
    }
    Is31fl3731_.Initialized = 1;
}


void Is31fl3731::Clear(uint8_t i2c_address)
{
    uint8_t b[2];

    //select Bank (current frame)
    b[0] = ISSI_COMMANDREGISTER;
    b[1] = _frame;
    i2c_.TransmitBlocking(i2c_address, b, 2, 1);

    uint8_t msg[25];

    for(uint8_t i = 0; i < 6; i++)
    {
        msg[0] = 0x24 + i * 24;

        // write 24 bytes to 0 at once
        for(uint8_t j = 1; j < 25; j++)
        {
            msg[j] = 0;
        }

        i2c_.TransmitBlocking(i2c_address, msg, 25, 1);
    }
}

void Is31fl3731::Set16(uint8_t i2c_address, uint8_t first_pixel, uint8_t data[16])
{
    uint8_t b[2];

    //select Bank (current frame)
    b[0] = ISSI_COMMANDREGISTER;
    b[1] = _frame;
    i2c_.TransmitBlocking(i2c_address, b, 2, 1);

    uint8_t msg[17];

    msg[0] = 0x24 + first_pixel;

    // write 16 bytes at once
    for(uint8_t i = 0; i < 16; i++)
    {
        if( first_pixel + i >= 144) return;
        msg[i+1] = data[i];
    }

    i2c_.TransmitBlocking(i2c_address, msg, 17, 1);
}

void Is31fl3731::SetLEDPWM(uint8_t i2c_address,
                           uint8_t lednum,
                           uint8_t pwm,
                           uint8_t bank)
{
    if(lednum >= 144)
        return;
    WriteRegister8(i2c_address, bank, (0x24 + lednum), pwm);
}


void Is31fl3731::DrawPixel(uint8_t i2c_address,
                           int16_t x,
                           int16_t y,
                           uint8_t pwm)
{
    // check rotation, move pixel around if necessary
    switch(getRotation())
    {
        case 1:
            _swap_int16_t(x, y);
            x = 16 - x - 1;
            break;
        case 2:
            x = 16 - x - 1;
            y = 9 - y - 1;
            break;
        case 3:
            _swap_int16_t(x, y);
            y = 9 - y - 1;
            break;
    }

    if((x < 0) || (x >= 16))
        return;
    if((y < 0) || (y >= 9))
        return;
    if(pwm > 255)
        pwm = 255; // PWM 8bit max

    SetLEDPWM(i2c_address, x + y * 16, pwm, _frame);
    return;
}


// Make this work for multiple chips
void Is31fl3731::SetFrame(uint8_t frame)
{
    _frame = frame;
}


void Is31fl3731::DisplayFrame(uint8_t i2c_address, uint8_t frame)
{
    if(frame > 7)
        frame = 0;
    WriteRegister8(
        i2c_address, ISSI_BANK_FUNCTIONREG, ISSI_REG_PICTUREFRAME, frame);
}


void Is31fl3731::AudioSync(uint8_t i2c_address, bool sync)
{
    if(sync)
    {
        WriteRegister8(
            i2c_address, ISSI_BANK_FUNCTIONREG, ISSI_REG_AUDIOSYNC, 0x1);
    }
    else
    {
        WriteRegister8(
            i2c_address, ISSI_BANK_FUNCTIONREG, ISSI_REG_AUDIOSYNC, 0x0);
    }
}


void Is31fl3731::WriteRegister8(uint8_t i2c_address,
                                uint8_t bank,
                                uint8_t reg,
                                uint8_t data)
{
    uint8_t b[2];

    //select Bank
    b[0] = ISSI_COMMANDREGISTER;
    b[1] = bank;
    i2c_.TransmitBlocking(i2c_address, b, 2, 1);

    // send data
    b[0] = reg;
    b[1] = data;
    i2c_.TransmitBlocking(i2c_address, b, 2, 1);
}

void Is31fl3731::setRotation(uint8_t x)
{
    rotation = (x & 3);
    switch(rotation)
    {
        case 0:
        case 2:
            _width  = WIDTH;
            _height = HEIGHT;
            break;
        case 1:
        case 3:
            _width  = HEIGHT;
            _height = WIDTH;
            break;
    }
}
