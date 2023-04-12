#pragma once

#include "daisy_seed.h"

//#define EXT_OLED
//#define EXT_POTCVUSB
//#define EXT_AUDIOIO
#ifdef EXT_OLED
#include "dev/oled_ssd130x.h"
#endif  // EXT_OLED




namespace daisy
{
class DaisyHank
{
  public:
    
    enum 
    {
        KNOB_1,   /**< */
        KNOB_2,   /**< */
        KNOB_3,   /**< */
        KNOB_4,   /**< */
        KNOB_LAST /**< */
    };

    enum 
    {
        CV_1,   /**< */
        CV_LAST /**< */
    };


    enum 
    {
        LED_1 /*Led1 R*/,
        LED_2 /*Led1 G*/,
        LED_3 /*Led1 B*/,
        LED_4 /*Led2 R*/,
        LED_5 /*Led2 G*/,
        LED_6 /*Led2 B*/,
        LED_7 /*Led3 R*/,
        LED_8 /*Led3 G*/,
        LED_9 /*Led3 B*/,
        LED_10 /*Led4 R*/,
        LED_11 /*Led4 G*/,
        LED_12 /*Led4 B*/,
        LED_LAST
    };

    enum Rgbs
    {
        RGB_LED_1,   /**< */
        RGB_LED_2,   /**< */
        RGB_LED_3,   /**< */
        RGB_LED_4,   /**< */
        RGB_LED_LAST,   /**< */ /**< */
    };



 

    




    /** Constructor */
    DaisyHank() {}
    /** Destructor */
    ~DaisyHank() {}

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
    bool GateIn1();
    bool TrigIn1();

    /** Gets a random 32-bit value */
    inline uint32_t GetRandomValue() { return Random::GetValue(); }

    /** Gets a random floating point value between the specified minimum, and maxmimum */
    inline float GetRandomFloat(float min = 0.f, float max = 1.f)
    {
        return Random::GetFloat(min, max);
    }

    /** Set an LED (idx < 4) to a color */
    void SetLed(size_t idx, float red, float green, float blue);  

    /**
  General delay _\param del Delay time in ms.
  */
    void DelayMs(size_t del);

    /** Turn all leds off */
    void ClearLeds();

    /** Update Leds to values you had set. */
    void UpdateLeds();


    float CVKnobCombo(float CV_Val,float Pot_Val);

    

    DaisySeed       seed;                /**< Seed object */
    AnalogControl   knob[KNOB_LAST]; /**< Array of AnalogControls */
    AnalogControl   cv[CV_LAST]; /**< Array of AnalogControls */
    GateIn        gate_in1;
    RgbLed        rgb[RGB_LED_LAST]; /**< & */
    Switch        s1;



    

  private:
    void SetHidUpdateRates();
    
    
};

} // namespace daisy
