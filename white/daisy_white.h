#pragma once

#include "daisy_seed.h"




namespace daisy
{
class DaisyWhite
{
  public:
    
    enum 
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
        KNOB_11,   /**< */
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
        CV_6,   /**< */
        CV_7,   /**< */
        CV_LAST /**< */
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

  enum {
      LED_KNOB_1,
      LED_KNOB_2,
      LED_KNOB_3,
      LED_KNOB_4,
      LED_KNOB_5,
      LED_KNOB_6,
      LED_KNOB_7,
      LED_KNOB_8,
      LED_S1,
      LED_S2,
      LED_S3,
      LED_S4,
      LED_S5,
      LED_S6,
      LED_S7,
      LED_S8
  };


  enum LeddriverLeds
    {
        RGB_LED_1 = 0,   /**< & */
        RGB_LED_2 = 1,   /**< & */
        RGB_LED_3 = 2,   /**< & */
        RGB_LED_4 = 3,
        GREEN_LED_1 = 12,    /**< & */
        GREEN_LED_2 = 13,    /**< & */
        GREEN_LED_3 = 14,    /**< & */
        GREEN_LED_4 = 15,
        LEDDRIVER_LEDS_LAST = 8
    };

  enum DirectLeds
    {
        GREEN_D_LED_1,    /**< & */
        GREEN_D_LED_2,    /**< & */
        GREEN_D_LED_3,    /**< & */
        GREEN_D_LED_4,    /**< & */
        GREEN_D_LED_5,    /**< & */
        GREEN_D_LED_6,
        DIRECT_LEDS_LAST
    };

    




    /** Constructor */
    DaisyWhite() {}
    /** Destructor */
    ~DaisyWhite() {}

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
    bool GateIn2();
    bool TrigIn1();
    bool TrigIn2();

    /** Gets a random 32-bit value */
    inline uint32_t GetRandomValue() { return Random::GetValue(); }

    /** Gets a random floating point value between the specified minimum, and maxmimum */
    inline float GetRandomFloat(float min = 0.f, float max = 1.f)
    {
        return Random::GetFloat(min, max);
    }

    /** Returns true if the key has not been pressed recently
        \param idx the key of interest
    */
    bool SwitchState(size_t idx) const;

    /** Returns true if the key has just been pressed
        \param idx the key of interest
    */
    bool SwitchRisingEdge(size_t idx) const;

    /** Returns true if the key has just been released
        \param idx the key of interest
    */
    bool SwitchFallingEdge(size_t idx) const;
    

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

    /**
       Set Green Direct LED
       \param idx Led Index
       \param bright Brightness
     */
    void SetGreenDirectLeds(DirectLeds idx, float bright);

    float CVKnobCombo(float CV_Val,float Pot_Val);

    

    DaisySeed       seed;                /**< Seed object */
    AnalogControl   knob[KNOB_LAST]; /**< Array of AnalogControls */
    AnalogControl   cv[CV_LAST]; /**< Array of AnalogControls */
    LedDriverPca9685<1, true>   led_driver_;
    GateIn        gate_in1, gate_in2;
    dsy_gpio      gate_out_1, gate_out_2;
    Led    green_direct_leds[6]; /**< & */
    

  private:
    void SetHidUpdateRates();
    ShiftRegister4021<2> switches_sr_; /**< Two 4021s daisy-chained. */
    uint8_t              switches_state_[16];
    
};

} // namespace daisy
