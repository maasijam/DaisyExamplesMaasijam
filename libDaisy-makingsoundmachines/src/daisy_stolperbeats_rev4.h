#pragma once
#ifndef DSY_STOLPERBEATS_REV3_BSP_H
#define DSY_STOLPERBEATS_REV3_H
#include "daisy_seed_stolperbeats.h"

namespace daisy
{
/**
    @brief Class that handles initializing all of the hardware specific to the Stolperbeats, based on the Daisy Patch Board. \n 
    Helper funtions are also in place to provide easy access to built-in controls and peripherals.
    @author Stephen Hensley, Making Sound Machines
    @date August 2021
    @ingroup boards
*/
class DaisyStolperbeatsRev4
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
        GATE_OUT_LAST, /**< */
    };

    /** Stolperbeats gate inputs */
    enum Button
    {
        BUTTON_1,    /**< BUTTON TAP */            
        BUTTON_LAST, /**< */
    };

    /** Stolperbeats SR 4021 inputs */
    enum SR_4021_inputs
    {
        SR_4021_BUTTON_1,    /**< BUTTON BANK DEC */
        SR_4021_BUTTON_2,    /**< BUTTON BANK INC */
        SR_4021_BUTTON_3,    /**< BUTTON SHUFFLE DILLA */
        SR_4021_BUTTON_4,    /**< BUTTON SHUFFLE SHAKE */
        SR_4021_BUTTON_5,    /**< BUTTON SHUFFLE PUSH */
        SR_4021_BUTTON_6,    /**< BUTTON SHUFFLE CLAVE */ 
        SR_4021_BUTTON_7,    /**< BUTTON LINEAR */  
        SR_4021_BUTTON_8,    /**< ENCODER SHUFFLE CLICK */ 

        SR_4021_BUTTON_9,    /**< EXPANDER BUTTON SUBDIV */   
        SR_4021_BUTTON_10,   /**< EXPANDER BUTTON PERC 2 */ 
        SR_4021_BUTTON_11,   /**< EXPANDER BUTTON PERC 1 */ 
        SR_4021_BUTTON_12,   /**< EXPANDER BUTTON HIHAT 2 */ 
        SR_4021_BUTTON_13,   /**< EXPANDER BUTTON LINEAR */ 
        SR_4021_BUTTON_14,   /**< EXPANDER BUTTON KICK */ 
        SR_4021_BUTTON_15,   /**< EXPANDER BUTTON SNARE */ 
        SR_4021_BUTTON_16,   /**< EXPANDER BUTTON HIHAT 1 */ 

        SR_4021_BUTTON_LAST, /**< */
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
    DaisyStolperbeatsRev4() {}
    /** Destructor */
    ~DaisyStolperbeatsRev4() {}

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

    /** Returns true if the key has not been pressed recently
        \param idx the key of interest
    */
    bool SR_4021State(size_t idx) const;

    /** Returns true if the key has just been pressed
        \param idx the key of interest
    */
    bool SR_4021RisingEdge(size_t idx) const;

    /** Returns true if the key has just been released
        \param idx the key of interest
    */
    bool SR_4021FallingEdge(size_t idx) const;


    /**  Control the display */
    void DisplayControls(bool invert = true);

    /* These are exposed for the user to access and manipulate directly
       Helper functions above provide easier access to much of what they are capable of.
    */
    DaisySeedStolperbeats  seed;                            /**< Seed object */
    Encoder                encoder[ENC_LAST];               /**< Encoder object */
    AnalogControl          controls[CTRL_LAST];             /**< Array of controls*/
    GateIn                 gate_input[GATE_IN_LAST];        /**< Gate inputs  */
    Switch                 button[BUTTON_LAST];             /**<  Buttons  */
    MidiUartHandler        midi;                            /**< Handles midi*/

    // TODO: Add class for Gate output
    dsy_gpio gate_output[GATE_OUT_LAST]; /**< Gate outputs  */

    /**  Shift Register for LEDs (output) */
    dsy_gpio_pin sr_pin_cfg[SR_LAST];
    ShiftRegister595 sr_595;

    /**  Shift Register for Button inputs, Enc 6 click and Expander (input) */
    ShiftRegister4021<2, 1> sr_4021;                           /**< Two 4021s daisy-chained. */
    uint16_t                sr_4021_state_[16];                /**< Save the state of an input on the SR 4021 */
    float                   sr_4021_time_held_[16];


    /**  Init a LED Matrix using an IS31FL3731 chip */

    /**  Init a LED Matrix using an IS31FL3731 chip */
    Is31fl3731 ledmatrix;
    void       InitLEDMatrix();

    /**  Init a LED Matrix with DMA using an IS31FL3731 chip */
    LedDriverIs31fl3731<2, true>                ledmatrix_dma;
    void       InitLEDMatrixDMA();

    /**  Init a Timer */
    TimerHandle tim5_;

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
    void InitSR4021();
    void UpdateSR4021();
    void InitTimer();    

    dsy_gpio ak4556_reset_pin_;
    uint32_t screen_update_last_, screen_update_period_;
    size_t   num_sr_chained;
};

} // namespace daisy

#endif
