#ifndef HWTEST_SETTINGS_H_
#define HWTEST_SETTINGS_H_

#include "../daisy_margolis.h"

using namespace daisy;
using namespace margolis;

#define FLASH_BLOCK 4096

  
/** Custom struct for  data to save. 
  *  The defaults can be set in the constructor,
  *  the initialization list, or manually set before
  *  calling the "PesistentStorage::Init" function.
*/
struct HwtestSettings 
{ 
    HwtestSettings() : ledcount(0), engine(0), ledatt{false} {}
    int ledcount, engine;
    bool ledatt[3];


    /** @brief checks sameness */
    bool operator==(const HwtestSettings &rhs)
    {
      if(ledcount != rhs.ledcount)  {
            return false;
        }
        else if(engine != rhs.engine)
        {
            return false;
        }
        else 
        {
            for(int i = 0; i < 3; i++)
            {
                if(ledatt[i] != rhs.ledatt[i])
                    return false;
            }
        }
        
        return true;
    }

    /** @brief Not equal operator */
    bool operator!=(const HwtestSettings &rhs) { return !operator==(rhs); }
};

struct StateSettings {
    StateSettings() : decay(0) {}
    uint8_t decay;
    /** @brief checks sameness */
    bool operator==(const StateSettings &rhs)
    {
        if(decay != rhs.decay)
        {
            return false;
        }
        
        return true;
    }
    /** @brief Not equal operator */
    bool operator!=(const StateSettings &rhs) { return !operator==(rhs); }
};

class Settings {
 public:
    Settings() { }
    ~Settings() { }
    
    void Init(DaisyMargolis* hw);

    void RestoreSettings();
    void RestoreState();
    void SaveSettings();
    void LoadSettings();
    void SaveStateSettings();
    void LoadStateSettings();

    int ledcount;
    int engine;
    bool ledatt[3];
    uint8_t decay;
  
   /** @brief Sets the cv offset from an externally array of data */
    inline void SetSettingsData(HwtestSettings hwsettings);
    /** @brief Sets the cv offset from an externally array of data */
    //inline void SetAttData(bool *ledatts);
     /** @brief Sets the cv offset from an externally array of data */
    inline void SetStateSettingsData(StateSettings stateset);

    /** @brief Sets the cv offset from an externally array of data */
    inline void GetSettingsData(HwtestSettings &hwsettings);
    /** @brief Sets the cv offset from an externally array of data */
    //inline void GetAttData(bool *ledatts);
    /** @brief Sets the cv offset from an externally array of data */
    inline void GetStateSettingsData(StateSettings &stateset);

    inline const HwtestSettings& hwtestsettings() const {
        return hwtest_settings_;
    }

    inline HwtestSettings* margolis_hwtestsettings() {
        return &hwtest_settings_;
    }
    
    inline const StateSettings& statesettings() const {
        return state_settings_;
    }

    inline StateSettings* margolis_statesettings() {
        return &state_settings_;
    }
   
  
 private:
    DaisyMargolis* hw_;
    HwtestSettings hwtest_settings_;
    StateSettings state_settings_;
    
};


#endif  // HWTEST_SETTINGS_H_