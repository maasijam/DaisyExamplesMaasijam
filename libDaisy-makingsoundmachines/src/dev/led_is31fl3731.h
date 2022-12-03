#pragma once
#ifndef DSY_DEV_LED_IS31FL3731_H
#define DSY_DEV_LED_IS31FL3731_H /**< Macro */
#include <stdlib.h>
#include <stdint.h>
#include "daisy_core.h"
#include "per/i2c.h"


namespace daisy
{
/** @addtogroup led
    @{ 
*/

/** 
 * Driver for ISSI IS31FL3731 144 ch PWM LED Matrix
 * Ported from the Adafruit IS31FL3731 library - https://github.com/adafruit/Adafruit_IS31FL3731/
 * Port for Daisy by Making Sound Machines     - https://github.com/makingsoundmachines
 *
 * LED driver for one or multiple ISSI IS31FL3731 144ch 8bit PWM chips connected to
 * a single I2C peripheral.
 * This driver uses two buffers - one for drawing, one for transmitting.
 * Multiple Is31fl3731 instances can be used at the same time.
 */

class Is31fl3731
{
  public:
    Is31fl3731() {}
    ~Is31fl3731() {}

    /** 
     * @brief Initialize ISSI IS31FL3731 driver
     * \param i2c           The I2C peripheral to use.
     * \param addresses     An array of addresses for each of the driver chips.
     * \param numDrivers    The number of IS31FL3731 drivers attached to the I2C
     *                      peripheral.
    */
    void
    Init(const I2CHandle i2c, const uint8_t* addresses, uint8_t numDrivers);

    /** 
     * @brief Alternate Init (Adafruit) - Initialize hardware and clear display
     * \param i2c           The I2C peripheral to use.
     * \param addresses     An array of addresses for each of the driver chips.
     * \param numDrivers    The number of IS31FL3731 drivers attached to the I2C
     *                      peripheral.
    */
    void
    Begin(const I2CHandle i2c, const uint8_t* addresses, uint8_t numDrivers);

    /** 
     *  @brief Test mode from the ISSI code example - LFO-pulses all LEDs
    */
    void Test_mode();

    /** 
     *  @brief Sets all LEDs on & 0 PWM for current frame.
     *  @param i2c_adress The I2C adress
    */
    void Clear(uint8_t i2c_address);

    /** 
     *  @brief Low level accesssor - sets a 8-bit PWM pixel value to a bank location
     *  does not handle rotation, x/y or any rearrangements!
     *  @param i2c_adress The I2C adress
     *  @param lednum The offset into the bank that corresponds to the LED
     *  @param bank The bank/frame we will set the data in
     *  @param pwm brightnes, from 0 (off) to 255 (max on)
    */
    void SetLEDPWM(uint8_t i2c_address, uint8_t lednum, uint8_t pwm, uint8_t bank);

    void Set16(uint8_t i2c_address, uint8_t first_pixel, uint8_t data[16]);

    /** 
     *  @brief Sets a 8-bit PWM pixel value, handles rotation 
     *  and pixel arrangement, unlike setLEDPWM
     *  @param i2c_adress The I2C adress
     *  @param x The x position, starting with 0 for left-most side
     *  @param y The y position, starting with 0 for top-most side
     *  @param pwm takes 0 (off) to 255 (max on)
    */
    void DrawPixel(uint8_t i2c_address, int16_t x, int16_t y, uint8_t pwm);

    /** 
     *  @brief Set's this object's frame tracker (does not talk to the chip)
     *  @param frame Ranges from 0 - 7 for the 8 frames
     *
     *  TODO: Make this work for multiple chips
    */
    void SetFrame(uint8_t frame);

    /** 
     *  @brief Have the chip set the display to the contents of a frame
     *  @param i2c_adress The I2C adress
     *  @param frame Ranges from 0 - 7 for the 8 frames
    */
    void DisplayFrame(uint8_t i2c_address, uint8_t frame);

    /** 
     *  @brief Enable the audio 'sync' for brightness pulsing (untested)
     *  @param i2c_adress The I2C adress
     *  @param sync True to enable, False to disable
    */
    void AudioSync(uint8_t i2c_address, bool sync);

    /** 
     *  @brief Write one byte to a register located in a given bank
     *  @param i2c_adress The I2C adress
     *  @param bank The IS31 bank to write the register location
     *  @param reg the offset into the bank to write
     *  @param data The byte value
    */
    void WriteRegister8(uint8_t i2c_address,
                        uint8_t bank,
                        uint8_t reg,
                        uint8_t data);

    /** 
     *  @brief Write one byte to a register
     *  @param i2c_adress The I2C adress
     *  @param reg the offset into the bank to write
     *  @param data The byte value
    */
    void WriteIs31fl3731(uint8_t address, uint8_t reg, uint8_t data);


    /************************************************************************/
    /*!
      @brief      Get rotation setting for display
      @returns    0 thru 3 corresponding to 4 cardinal rotations
    */
    /************************************************************************/
    uint8_t getRotation(void) const { return rotation; }

    /************************************************************************/
    /*!
      @brief      Set rotation setting for display
      @returns    0 thru 3 corresponding to 4 cardinal rotations
    */
    /************************************************************************/
    void setRotation(uint8_t r);

  private:
    void Reset();
    void SetInternalRef(bool enabled);

    uint8_t* addresses_;
    uint8_t  numDrivers_;

    uint8_t _frame;

    int16_t WIDTH  = 16;  ///< This is the 'raw' display width - never changes
    int16_t HEIGHT = 9;   ///< This is the 'raw' display height - never changes
    int16_t _width;       ///< Display width as modified by current rotation
    int16_t _height;      ///< Display height as modified by current rotation
    uint8_t rotation = 0; ///< Display rotation (0 thru 3)
};
/** @} */
} // namespace daisy

#endif
