#pragma once
#ifndef MARGOLIS_HW_H
#define MARGOLIS_HW_H

#include "daisy_seed.h"



namespace daisy
{


class DaisyMargolis
{
  public:
    
    enum 
    {
        KNOB_1,   
        KNOB_2,
        KNOB_3,
        KNOB_4,
        KNOB_5,
        KNOB_6,
        KNOB_7,   
        KNOB_LAST 
    };

    enum CvAdcChannel
    {
        CV_1,   
        CV_2,
        CV_3,
        CV_4,
        CV_5,
        CV_6,
        CV_VOCT,   
        CV_LAST 
    };

    enum Colors {
      red,
      green,
      blue,
      yellow,
      cyan,
      purple,
      orange,
      darkgreen,
      darkblue,
      darkred,
      turq,
      grey,
      darkorange,
      white,
      off
  };


    enum LeddriverLeds
    {
        LED_RGB_1,
        LED_RGB_2,
        LED_RGB_3,
        LED_RGB_4,
        LED_RGB_5,
        LED_RGB_6,
        LED_RGB_7,
        LED_RGB_8,
        LED_GREEN_3,
        LED_GREEN_2,
        LED_GREEN_1,
        LEDDRIVER_LAST,
    };

   
  enum {
      S1,
      S2,
      S3,
      S4,
      S5,
      S6,
      S_LAST
  };



    /** Constructor */
    DaisyMargolis() {}
    /** Destructor */
    ~DaisyMargolis() {}

    /** Initializes the daisy seed.*/
    void Init(bool boost = true);

    /** Audio Block size defaults to 48.
  Change it using this function before StartingAudio _\param size Audio block size.
  */
    void SetAudioBlockSize(size_t size);

    /** Returns the number of samples per channel in a block of audio. */
    size_t AudioBlockSize();

    /** Set the sample rate for the audio */
    void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Get sample rate */
    float AudioSampleRate();


    /** Get callback rate */
    float AudioCallbackRate();

    /** Start the saul audio with the given callback function
    \cb AudioCallback callback function
    */
    void StartAudio(AudioHandle::AudioCallback cb);

    /** Starts the callback  _\param cb Interleaved callback function
    */
    void StartAudio(AudioHandle::InterleavingAudioCallback cb);

    /**
       Switch callback functions
       \param cb New interleaved callback function.
    */
    void ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb);

        /** Turns on the built-in 12-bit DAC on the Daisy Seed */
    /** **This is now deprecated and does nothing.** 
     ** The polling use of the DACs now handles starting the tranmission.  */
    void StartDac();

    /** Sets the output of CV out 1 to a value between 0-4095 that corresponds to 0-5V */
    void SetCvOut1(uint16_t val);

    /** Sets the output of CV out 2 to a value between 0-4095 that corresponds to 0-5V */
    void SetCvOut2(uint16_t val);


    /**
     Change the AudioCallback function _\param cb The new callback function.
  */
    void ChangeAudioCallback(AudioHandle::AudioCallback cb);

    /** Stops the audio */
    void StopAudio();

    /** Start the ADC */
    void StartAdc();

    /** Stop the ADC */
    void StopAdc();

    /** Process all analog controls */
    void ProcessAnalogControls();

    /** Process all digital controls */
    void ProcessDigitalControls();

    /** Process Analog and Digital Controls */
    inline void ProcessAllControls()
    {
        ProcessAnalogControls();
        ProcessDigitalControls();
    }

     /** Getter for switch objects
        \param idx The switch of interest.
    */
    Switch* GetSwitch(size_t idx);


    /**
     Get value for a particular control _\param k Which control to get
   */
    float GetKnobValue(int idx) const;
    float GetCvValue(int idx) const;

  /** Getter for CV objects.
        \param idx The CV input of interest.
    */
    AnalogControl* GetCv(size_t idx);

    /** Returns true if gate in is HIGH */
    bool Gate();

    /** Gets a random 32-bit value */
    inline uint32_t GetRandomValue() { return Random::GetValue(); }

    /** Gets a random floating point value between the specified minimum, and maxmimum */
    inline float GetRandomFloat(float min = 0.f, float max = 1.f)
    {
        return Random::GetFloat(min, max);
    }

      

    /**
  General delay _\param del Delay time in ms.
  */
    void DelayMs(size_t del);

    /** Turn all leds off */
    void ClearLeds();

    /** Update Leds to values you had set. */
    void UpdateLeds();

        /**
       Set rgb LED colors
       \param idx Index to set
       \param r Red value
       \param g Green value
       \param b Blue value
     */
    void SetRgbLeds(LeddriverLeds idx, float r, float g, float b);
    void SetRGBColor(LeddriverLeds idx, Colors c);

    /**
       Set Green LED driver LED
       \param idx Led Index
       \param bright Brightness
     */
    void SetGreenLeds(size_t idx, float bright);


    float CVKnobCombo(float CV_Val,float Pot_Val);

    
    
    

    DaisySeed       seed;                /**< Seed object */
    AnalogControl   knob[KNOB_LAST]; /**< Array of AnalogControls */
    AnalogControl   cv[CV_LAST]; /**< Array of AnalogControls */
    GateIn        gate_in;
    Switch        s[S_LAST];
    
    

  private:
    void SetHidUpdateRates();
    void InitLeds();
    LedDriverPca9685<2, true> led_driver_;
    
};

} // namespace daisy
#endif