#pragma once

#include "daisy_seed.h"


namespace saul
{

using namespace daisy;

#define FLASH_BLOCK 4096

enum Pots
    {
        KNOB_0,   /**< */
        KNOB_1,   /**< */
        KNOB_2,   /**< */
        KNOB_3,   /**< */
        KNOB_4,   /**< */
        KNOB_5,   /**< */
        KNOB_6,   /**< */
        KNOB_7,   /**< */
        KNOB_8,   /**< */
        KNOB_9,   /**< */
        KNOB_10,   /**< */
        KNOB_LAST /**< */
    };

    enum CvIns
    {
        CV_0,   /**< */
        CV_1,   /**< */
        CV_2,   /**< */
        CV_3,   /**< */
        CV_4,   /**< */
        CV_5,   /**< */
        CV_6,   /**< */
        CV_7,   /**< */
        CV_8,   /**< */
        CV_9,   /**< */
        CV_10,   /**< */
        CV_LAST /**< */
    };

    enum OnOffOns
    {
        SW_0,
        SW_1,
        SW_LAST
    };

    enum Switches
    {
        S_0,
        S_1,
        S_2,
        S_3,
        S_4,
        S_5,
        S_6,
        S_LAST
    };

    enum 
    {
        LED_0,
        LED_1,
        LED_2,
        LED_3,
        LED_4,
        LED_5,
        LED_6,
        LED_7,
        LED_8 /*Led1 R*/,
        LED_9 /*Led1 G*/,
        LED_10 /*Led1 B*/,
        LED_11 /*Led2 R*/,
        LED_12 /*Led2 G*/,
        LED_13 /*Led2 B*/,
        LED_14 /*Led3 R*/,
        LED_15 /*Led3 G*/,
        LED_16 /*Led3 B*/,
        LED_17 /*Led4 R*/,
        LED_18 /*Led4 G*/,
        LED_19 /*Led4 B*/,
        LED_20, 
        LED_21, 
        LED_LAST
    };

    enum Colors {
      RED,
      GREEN,
      BLUE,
      YELLOW,
      PURPLE,
      AQUA,
      OFF,
      WHITE
  };

  

class DaisySaul
{
  public:
  
    /** Constructor */
    DaisySaul() {}
    /** Destructor */
    ~DaisySaul() {}

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

    /** Returns true if gate in is HIGH */
    bool Gate();

    void SetLed(uint8_t idx, bool state);
    void SetRGBLed(uint8_t idx, uint8_t color);

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


    ShiftRegister595 all_leds; 
    

    DaisySeed       seed;                /**< Seed object */
    AnalogControl   knob[KNOB_LAST]; /**< Array of AnalogControls */
    AnalogControl   cv[CV_LAST]; /**< Array of AnalogControls */
    Switch3       sw[SW_LAST];
    Switch        s[S_LAST];
    GateIn        gate;

    

  private:
    void InitControls();
    void SetHidUpdateRates();
    void SetRGBColor(uint8_t ridx,uint8_t gidx,uint8_t bidx, uint8_t color);

    
    
};

} // namespace daisy
