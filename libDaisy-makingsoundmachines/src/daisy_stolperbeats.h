#pragma once
#ifndef DSY_STOLPERBEATS_BSP_H
#define DSY_STOLPERBEATS_H
#include "daisy_seed.h"

namespace daisy
{
/**
    @brief Class that handles initializing all of the hardware specific to the Stolperbeats, based on the Daisy Patch Board. \n 
    Helper funtions are also in place to provide easy access to built-in controls and peripherals.
    @author Stephen Hensley, Making Sound Machines
    @date August 2021
    @ingroup boards
*/
class DaisyStolperbeats
{
  public:
    /** Enum of Ctrls to represent the CV/Knob combos on Stolperbeats
     */
    enum Ctrl
    {
        CTRL_1,    /**< CV IN KICK */
        CTRL_2,    /**< CV IN SNARE */
        CTRL_3,    /**< CV IN HIHAT 1 */
        CTRL_4,    /**< CV IN HIHAT 2 */
        CTRL_5,    /**< CV IN PERC 1 */
        CTRL_6,    /**< CV IN PERC 2 */
        CTRL_7,    /**< CV IN SHUFFLE */
        CTRL_8,    /**< CV IN BANK */
        CTRL_LAST, /**< */
    };

    /** Stolperbeats gate inputs */
    enum GateInput
    {
        GATE_IN_1,    /**< TAP */
        GATE_IN_2,    /**< LINEAR DRUMMING */
        GATE_IN_3,    /**< EXPANDER 1 */
        GATE_IN_4,    /**< EXPANDER 2 */
        GATE_IN_5,    /**< EXPANDER 3 */
        GATE_IN_6,    /**< EXPANDER 4 */
        GATE_IN_7,    /**< EXPANDER 5 */
        GATE_IN_8,    /**< EXPANDER 6 */
        GATE_IN_9,    /**< EXPANDER 7 */
        GATE_IN_10,   /**< EXPANDER 8 */
        GATE_IN_LAST, /**< */
    };

    /** Stolperbeats gate outputs */
    enum GateOutput
    {
        GATE_OUT_1,    /**< TRIGGER OUT KICK */
        GATE_OUT_2,    /**< TRIGGER OUT SNARE */
        GATE_OUT_3,    /**< TRIGGER OUT HIHAT 1 */
        GATE_OUT_4,    /**< TRIGGER OUT HIHAT 2 */
        GATE_OUT_5,    /**< TRIGGER OUT PERC 1 */
        GATE_OUT_6,    /**< TRIGGER OUT PERC 2 */
        GATE_OUT_7,    /**< TRIGGER OUT SUBDIV */
        GATE_OUT_8,    /**< TRIGGER OUT SYNC */
        GATE_OUT_9,    /**< TRIGGER OUT EXPANDER 1 */
        GATE_OUT_10,   /**< TRIGGER OUT EXPANDER 2 */
        GATE_OUT_11,   /**< TRIGGER OUT EXPANDER 3 */
        GATE_OUT_12,   /**< TRIGGER OUT EXPANDER 4 */
        GATE_OUT_13,   /**< TRIGGER OUT EXPANDER 5 */
        GATE_OUT_14,   /**< TRIGGER OUT EXPANDER 6 */
        GATE_OUT_15,   /**< TRIGGER OUT EXPANDER 7 */
        GATE_OUT_16,   /**< TRIGGER OUT EXPANDER 8 */
        GATE_OUT_LAST, /**< */
    };

    /** Stolperbeats gate inputs */
    enum Button
    {
        BUTTON_1,    /**< BUTTON TAP */
        BUTTON_2,    /**< BUTTON LINEAR */  
        BUTTON_3,    /**< BUTTON BANK DEC */
        BUTTON_4,    /**< BUTTON BANK INC */
        BUTTON_5,    /**< BUTTON SHUFFLE DILLA */
        BUTTON_6,    /**< BUTTON SHUFFLE SHAKE */
        BUTTON_7,    /**< BUTTON SHUFFLE PUSH */
        BUTTON_8,    /**< BUTTON SHUFFLE CLAVE */            
        BUTTON_LAST, /**< */
    };

    /** Stolperbeats gate inputs */
    enum Enc
    {
        ENC_1,    /**< ENCODER TEMPO */
        ENC_2,    /**< ENCODER KICK */
        ENC_3,    /**< ENCODER SNARE */
        ENC_4,    /**< ENCODER HIHAT 1 */
        ENC_5,    /**< ENCODER HIHAT 2 */
        ENC_6,    /**< ENCODER SHUFFLE */
        ENC_7,    /**< ENCODER PERC 1 */
        ENC_8,    /**< ENCODER PERC 2 */ 
        ENC_9,    /**< ENCODER SUBDIV */
        ENC_10,   /**< ENCODER SYNC */       
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
    DaisyStolperbeats() {}
    /** Destructor */
    ~DaisyStolperbeats() {}

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
    Switch            button[BUTTON_LAST];             /**<  Buttons  */
    MidiUartHandler   midi;                            /**< Handles midi*/
    Dac8568           dac_8568;                        /**< Handles ext DAC*/

    // TODO: Add class for Gate output
    dsy_gpio gate_output[GATE_OUT_LAST]; /**< Gate outputs  */
    dsy_gpio led_linear; /**< LED Linear Drumming  */

    dsy_gpio_pin sr_pin_cfg[SR_LAST];
    ShiftRegister595 sr_595;

    /**  Init a LED Matrix using an IS31FL3731 chip */
    Is31fl3731 ledmatrix;
    void       InitLEDMatrix();

    /**  Init a LED Matrix with DMA using an IS31FL3731 chip */
    LedDriverIs31fl3731<2, true>                ledmatrix_dma;
    void       InitLEDMatrixDMA();


    /**  Init a Bela Trill */
    Trill bela_trill;
    void  InitBelaTrill();

  private:
    void SetHidUpdateRates();
    void InitAudio();
    void InitControls();
    /** void InitDisplay(); */
    void InitMidi();
    void InitCvOutputs();
    void InitEncoder();
    void InitGates();
    void InitButtons();
    void InitSR595();
    void InitDAC8568();

    dsy_gpio ak4556_reset_pin_;
    uint32_t screen_update_last_, screen_update_period_;
    size_t   num_sr_chained;
};

} // namespace daisy

#endif
