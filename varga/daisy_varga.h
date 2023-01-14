#pragma once
#ifndef VARGA_HW_H
#define VARGA_HW_H

#include "daisy_seed.h"


namespace varga
{

using namespace daisy;


#define FLASH_BLOCK 4096

enum Pots
    {
        KNOB_1,   /**< */
        KNOB_2,   /**< */
        KNOB_3,   /**< */
        KNOB_4,   /**< */
        KNOB_LAST /**< */
    };

    enum CvIns
    {
        CV_1,   /**< */
        CV_2,   /**< */
        CV_3,   /**< */
        CV_VOCT,   /**< */
        CV_4,   /**< */
        CV_5,
        CV_LAST /**< */
    };


    enum Switches
    {
        S_1,
        S_2,
        S_LAST
    };

    enum Leds
    {
        LED_1,
        LED_2,
        LED_LAST
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

class DaisyVarga
{
  public:
    
    


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

    float CVKnobCombo(float CV_Val,float Pot_Val);
    
    float GetWarpVoct();  

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
    void LoadCalibrationData();

    /** Cal data */
    float                  warp_v1_, warp_v3_;
    daisy::VoctCalibration voct_cal;
    float                  cv_offsets_[CV_LAST];
    

    bool cal_save_flag_;
    
};

} // namespace varga
#endif