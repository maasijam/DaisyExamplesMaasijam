#pragma once
#ifndef DSY_HEARTOFGOLD_BSP_H
#define DSY_HEARTOFGOLD_H
#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"

namespace daisy
{
/**
    @brief Class that handles initializing all of the hardware specific to the Heart of Gold, based on the Daisy Patch Board. \n 
    Helper funtions are also in place to provide easy access to built-in controls and peripherals.
    @author Stephen Hensley, Making Sound Machines
    @date March 2020
    @ingroup boards
*/
class DaisyHeartOfGold
{
  public:
    /** Enum of Ctrls to represent the four CV/Knob combos on the Patch
     */
    enum Ctrl
    {
        CTRL_1,    /**< */
        CTRL_2,    /**< */
        CTRL_3,    /**< */
        CTRL_4,    /**< */
        CTRL_5,    /**< */
        CTRL_6,    /**< */
        CTRL_7,    /**< */
        CTRL_8,    /**< */
        CTRL_9,    /**< */
        CTRL_10,   /**< */
        CTRL_11,   /**< */
        CTRL_12,   /**< */
        CTRL_LAST, /**< */
    };

    /** Daisy patch gate inputs */
    enum GateInput
    {
        GATE_IN_1,    /**< */
        GATE_IN_2,    /** <*/
        GATE_IN_3,    /** <*/
        GATE_IN_4,    /** <*/
        GATE_IN_LAST, /**< */
    };

    /** Daisy patch gate inputs */
    enum GateOutput
    {
        GATE_OUT_1,    /**< */
        GATE_OUT_2,    /** <*/
        GATE_OUT_3,    /** <*/
        GATE_OUT_4,    /** <*/
        GATE_OUT_LAST, /**< */
    };

    /** Daisy patch gate inputs */
    enum RouteButton
    {
        ROUTE_BUTTON_1,    /**< */
        ROUTE_BUTTON_2,    /** <*/
        ROUTE_BUTTON_3,    /** <*/
        ROUTE_BUTTON_4,    /** <*/
        ROUTE_BUTTON_LAST, /**< */
    };

    /** Daisy patch gate inputs */
    enum Enc
    {
        ENC_1,    /**< */
        ENC_2,    /** <*/
        ENC_3,    /** <*/
        ENC_4,    /** <*/
        ENC_LAST, /**< */
    };

    /** Shift Register Pins */
    enum SR595Pin
    {
        SR_LATCH, /**< */
        SR_CLOCK, /** <*/
        SR_DATA,  /** <*/
        SR_LAST,  /**< */
    };


    /** Constructor */
    DaisyHeartOfGold() {}
    /** Destructor */
    ~DaisyHeartOfGold() {}

    /** Initializes the daisy seed, and patch hardware.*/
    void Init(bool boost = false);

    /** 
    Wait some ms before going on.
    \param del Delay time in ms.
    */
    void DelayMs(size_t del);


    /** Starts the callback
    \cb multichannel callback function
    */
    void StartAudio(AudioHandle::AudioCallback cb);

    /**
       Switch callback functions
       \param cb New multichannel callback function.
    */
    void ChangeAudioCallback(AudioHandle::AudioCallback cb);

    /** Stops the audio */
    void StopAudio();

    /** Set the sample rate for the audio */
    void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Get sample rate */
    float AudioSampleRate();

    /** Audio Block size defaults to 48.
    Change it using this function before StartingAudio
    \param size Audio block size.
    */
    void SetAudioBlockSize(size_t size);

    /** Returns the number of samples per channel in a block of audio. */
    size_t AudioBlockSize();

    /** Returns the rate in Hz that the Audio callback is called */
    float AudioCallbackRate();

    /** Start analog to digital conversion.*/
    void StartAdc();

    /** Stops Transfering data from the ADC */
    void StopAdc();


    /** Call at same rate as reading controls for good reads. */
    void ProcessAnalogControls();

    /** Process Analog and Digital Controls */
    inline void ProcessAllControls()
    {
        ProcessAnalogControls();
        ProcessDigitalControls();
    }

    /**
       Get value for a particular control
       \param k Which control to get
     */
    float GetKnobValue(Ctrl k);

    /**  Process the digital controls */
    void ProcessDigitalControls();

    /**  Control the display */
    void DisplayControls(bool invert = true);

    /* These are exposed for the user to access and manipulate directly
       Helper functions above provide easier access to much of what they are capable of.
    */
    DaisySeed         seed;                            /**< Seed object */
    Encoder           encoder[ENC_LAST];               /**< Encoder object */
    AnalogControl     controls[CTRL_LAST];             /**< Array of controls*/
    GateIn            gate_input[GATE_IN_LAST];        /**< Gate inputs  */
    /* Switch            gate_input[GATE_IN_LAST]; */      /**< Gate inputs  */
    GateIn            route_button[ROUTE_BUTTON_LAST]; /**< Route Buttons  */
    MidiUartHandler   midi;                            /**< Handles midi*/
    Dac8568           dac_8568;                        /**< Handles ext DAC*/

    SdramHandle       sdram_handle; /**< & */

    // TODO: Add class for Gate output
    dsy_gpio gate_output[GATE_OUT_LAST]; /**< Gate outputs  */

    dsy_gpio_pin sr_pin_cfg[SR_LAST];

    ShiftRegister595 sr_595;

    /**  Init a SSD1306/SSD1309 128x64 OLED displays connected via I2C */
    OledDisplay<SSD130xI2c128x64Driver> display;       
    void       InitDisplay();

    /**  Init a LED Matrix using an IS31FL3731 chip */
    Is31fl3731 ledmatrix;
    void       InitLEDMatrix();

    /**  Init a LED Matrix with DMA using an IS31FL3731 chip */
    LedDriverIs31fl3731<2, true>                ledmatrix_dma;
    void       InitLEDMatrixDMA();


    /**  Init a LED Matrix using an IS31FL3731 chip */
    Trill bela_trill;
    void  InitBelaTrill();

  private:
    void SetHidUpdateRates();
    void InitAudio();
    void InitControls();
    void InitMidi();
    void InitCvOutputs();
    void InitEncoder();
    void InitGates();
    void InitRouteButtons();
    void InitSR595();
    void InitDAC8568();

    dsy_gpio ak4556_reset_pin_;
    uint32_t screen_update_last_, screen_update_period_;
    size_t   num_sr_chained;
};

} // namespace daisy

#endif
