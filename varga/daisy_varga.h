#pragma once

#include "daisy_seed.h"


namespace daisy
{
class DaisyVarga
{
  public:
    
    enum 
    {
        KNOB_0,   /**< */
        KNOB_1,   /**< */
        KNOB_2,   /**< */
        KNOB_3,   /**< */
        KNOB_LAST /**< */
    };

    enum 
    {
        CV_0,   /**< */
        CV_1,   /**< */
        CV_2,   /**< */
        CV_3,   /**< */
        CV_4,   /**< */
        CV_5,   /**< */
        CV_LAST /**< */
    };


    enum 
    {
        S_0,
        S_1,
        S_LAST
    };

    enum 
    {
        LED_0,
        LED_1,
        LED_LAST
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


    /** Constructor */
    DaisyVarga() {}
    /** Destructor */
    ~DaisyVarga() {}

    /** Initializes the daisy seed, and bluemchen hardware.*/
    void Init(bool boost = false);

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

    /** Getter for Knob objects.
        \param idx The CV input of interest.
    */
    AnalogControl* GetKnob(size_t idx);

    /** Returns true if gate in is HIGH */
    bool Gate();

    /** Set an LED (idx < 4) to a color */
    void SetLed(size_t idx, float red, float green, float blue);
    void SetRGBColor(size_t idx, Colors color);

    /** Reset Leds*/
    void ClearLeds();
    /** Update LED PWM state. Call this once per main loop update to correctly display led colors */
    void UpdateLeds();

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

    DaisySeed       seed;                /**< Seed object */
    AnalogControl   knob[KNOB_LAST]; /**< Array of AnalogControls */
    AnalogControl   cv[CV_LAST]; /**< Array of AnalogControls */
    Switch        s[S_LAST];
    GateIn        gate;
    RgbLed        leds[LED_LAST];
    
    

  private:
    void SetHidUpdateRates();
    
    
};

} // namespace daisy
