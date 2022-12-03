#pragma once
#ifndef DSY_PERCUSSIVE_MAINTENANCE_BSP_H
#define DSY_PERCUSSIVE_MAINTENANCE_BSP_H
#include "daisy_seed_percussive.h"
#include "dev/oled_ssd130x.h"

namespace daisy
{
/**
    @brief Class that handles initializing all of the hardware specific to the Percussive Maintenance, a distant relative of the Daisy Patch Board. \n 
    Helper funtions are also in place to provide easy access to built-in controls and peripherals.
    @author Stephen Hensley, Making Sound Machines
    @date April 2022
    @ingroup boards
*/
class DaisyPercussiveMaintenance
{
  public:
    /** Enum of Ctrls to represent the four CV/Knob combos on Percussive Maintenance
     */
    enum Ctrl
    {
        CTRL_1,    /**< CV IN Pitch 1 Jack */
        CTRL_2,    /**< CV IN Pitch 2 Jack */
        CTRL_3,    /**< CV IN Pitch 1 Pot */
        CTRL_4,    /**< CV IN Pitch 2 Pot */
        CTRL_5,    /**< CV IN Decay 1 */
        CTRL_6,    /**< CV IN Decay 2 */
        CTRL_7,    /**< CV IN Timbre 1 */
        CTRL_8,    /**< CV IN Timbre 2 */
        CTRL_9,    /**< CV IN Env Follower */
        CTRL_10,   /**< CV IN Ghost 1 */
        CTRL_11,   /**< CV IN Ghost 2 */
        CTRL_LAST, /**< */
    };

    /** Percussive Maintenance gate inputs */
    enum GateInput
    {
        GATE_IN_1,    /**< Trig 1 */
        GATE_IN_2,    /**< Trig 2 */
        GATE_IN_3,    /**< Acc 1 */
        GATE_IN_4,    /**< Acc 2 */
        GATE_IN_5,    /**< Ghost 1 */
        GATE_IN_6,    /**< Ghost 2 */
        GATE_IN_7,    /**< Piezo Trigger */
        GATE_IN_8,    /**< Piezo gate */
        GATE_IN_LAST, /**< */
    };

    /** Percussive Maintenance button inputs */
    enum Button
    {
        BUTTON_1,    /**< BUTTON Cherry 1 */  
        BUTTON_2,    /**< BUTTON Cherry 2 */           
        BUTTON_LAST, /**< */
    };

    /** Percussive Maintenance Encoder inputs */
    enum Enc
    {
        ENC_1,    /**< ENCODER MODE 1 */
        ENC_2,    /**< ENCODER FX 1 */
        ENC_3,    /**< ENCODER MODE 2 */
        ENC_4,    /**< ENCODER FX 2 */      
        ENC_LAST, /**< */
    };

    /** Constructor */
    DaisyPercussiveMaintenance() {}
    /** Destructor */
    ~DaisyPercussiveMaintenance() {}

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
    DaisySeedPercussive       seed;                             /**< Seed object */
    Encoder                   encoder[ENC_LAST];               /**< Encoder object */
    AnalogControl             controls[CTRL_LAST];              /**< Array of controls*/
    GateIn                    gate_input[GATE_IN_LAST];         /**< Gate inputs  */
    Switch                    button[BUTTON_LAST];              /**< Buttons  */
    MidiUartHandler           midi;                             /**< Handles midi*/
    OledDisplay<SSD130x4WireSpi128x64Driver> display[2]; /**< & */

    // TODO: Add class for Gate output
    dsy_gpio gate_output; /**< &  */


  private:
    void SetHidUpdateRates();
    void InitAudio();
    void InitControls();
    void InitDisplay();
    void InitMidi();
    void InitCvOutputs();
    void InitEncoder();
    void InitGates();
    void InitButtons();

    uint32_t screen_update_last_, screen_update_period_;
};

} // namespace daisy

#endif
