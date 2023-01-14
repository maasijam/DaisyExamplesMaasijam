#pragma once
#ifndef MARGOLIS_HW_H
#define MARGOLIS_HW_H

#include "daisy_seed.h"



namespace margolis
{

using namespace daisy;

#define FLASH_BLOCK 4096

enum  Pots
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

enum CvIns
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
      RED,
      GREEN,
      BLUE,
      YELLOW,
      CYAN,
      PURPLE,
      ORANGE,
      DARKGREEN,
      DARKBLUE,
      DARKRED,
      TURQ,
      GREY,
      DARKORANGE,
      WHITE,
      OFF
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

   
  enum Switches 
  {
      S1,
      S2,
      S3,
      S4,
      S5,
      S6,
      S_LAST
  };

enum LedOrder
{
    LED_1_R = 30,
    LED_1_G = 31,
    LED_1_B = 0,
    LED_2_R = 28,
    LED_2_G = 29,
    LED_2_B = 1,
    LED_3_R = 26,
    LED_3_G = 27,
    LED_3_B = 2,
    LED_4_R = 24,
    LED_4_G = 25,
    LED_4_B = 3,
    LED_5_R = 17,
    LED_5_G = 16,
    LED_5_B = 4,
    LED_6_R = 19,
    LED_6_G = 18,
    LED_6_B = 5,
    LED_7_R = 21,
    LED_7_G = 20,
    LED_7_B = 6,
    LED_8_R = 23,
    LED_8_G = 22,
    LED_8_B = 7,
    LED_G_1 = 10,
    LED_G_2 = 9,
    LED_G_3 = 8,
    LED_LAST = 27,
};

/** @brief Calibration data container for Margolis 
*/
struct CalibrationData
{
    CalibrationData() : warp_scale(60.f), warp_offset(0.f), cv_offset{0.f} {}
    float warp_scale, warp_offset;
    float cv_offset[CV_LAST];
    

    /** @brief checks sameness */
    bool operator==(const CalibrationData &rhs)
    {
        if(warp_scale != rhs.warp_scale)
        {
            return false;
        }
        else if(warp_offset != rhs.warp_offset)
        {
            return false;
        }
        else
        {
            for(int i = 0; i < CV_LAST; i++)
            {
                if(cv_offset[i] != rhs.cv_offset[i])
                    return false;
            }
        }
        return true;
    }

    /** @brief Not equal operator */
    bool operator!=(const CalibrationData &rhs) { return !operator==(rhs); }
};

class DaisyMargolis
{
  public:
    
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

    float GetWarpVoct();  

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

    /** @brief called during a customized calibration UI to record the 1V value */
    inline void CalibrateV1(float v1) { warp_v1_ = v1; }

    /** @brief called during a customized calibration UI to record the 3V value 
     *         and set that calibraiton has completed and can be saved. 
     */
    inline void CalibrateV3(float v3)
    {
        warp_v3_ = v3;
        voct_cal.Record(warp_v1_, warp_v3_);
        cal_save_flag_ = true;
    }

    /** @brief Sets the calibration data for 1V/Octave over Warp CV 
     *  typically set after reading stored data from external memory.
     */
    inline void SetWarpCalData(float scale, float offset)
    {
        voct_cal.SetData(scale, offset);
    }

    /** @brief Gets the current calibration data for 1V/Octave over Warp CV 
     *  typically used to prepare data for storing after successful calibration
     */
    inline void GetWarpCalData(float &scale, float &offset)
    {
        voct_cal.GetData(scale, offset);
    }

    /** @brief Sets the cv offset from an externally array of data */
    inline void SetCvOffsetData(float *data)
    {
        for(int i = 0; i < CV_LAST; i++)
        {
            cv_offsets_[i] = data[i];
        }
    }

    /** @brief Fills an array with the offset data currently being used */
    inline void GetCvOffsetData(float *data)
    {
        for(int i = 0; i < CV_LAST; i++)
        {
            data[i] = cv_offsets_[i];
        }
    }

    /** @brief Checks to see if calibration has been completed and needs to be saved */
    inline bool ReadyToSaveCal() const { return cal_save_flag_; }

    /** @brief signal the cal-save flag to clear once calibration data has been written to ext. flash memory */
    inline void ClearSaveCalFlag() { cal_save_flag_ = false; }

    
    
    

    DaisySeed       seed;                /**< Seed object */
    AnalogControl   knob[KNOB_LAST]; /**< Array of AnalogControls */
    AnalogControl   cv[CV_LAST]; /**< Array of AnalogControls */
    GateIn        gate_in;
    Switch        s[S_LAST];
    
    
    

  private:
    void SetHidUpdateRates();
    void InitLeds();
    void LoadCalibrationData();
    LedDriverPca9685<2, true> led_driver_;


    /** Cal data */
    float                  warp_v1_, warp_v3_;
    daisy::VoctCalibration voct_cal;
    float                  cv_offsets_[CV_LAST];
    

    bool cal_save_flag_;
    
};

} // namespace margolis
#endif