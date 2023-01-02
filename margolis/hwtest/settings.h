#ifndef HWTEST_SETTINGS_H_
#define HWTEST_SETTINGS_H_

#include "../daisy_margolis.h"

using namespace daisy;

#define FLASH_BLOCK 4096

  
/** Custom struct for  data to save. 
  *  The defaults can be set in the constructor,
  *  the initialization list, or manually set before
  *  calling the "PesistentStorage::Init" function.
*/
struct MargolisSettings 
{ 
    MargolisSettings() : ledcount(0), engine(0), ledatt{false} {}
    int ledcount, engine;
    bool ledatt[3];


    /** @brief checks sameness */
    bool operator==(const MargolisSettings &rhs)
    {
        if(ledcount != rhs.ledcount)
        {
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
    bool operator!=(const MargolisSettings &rhs) { return !operator==(rhs); }
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
    void SaveSettings();
    void LoadSettings();
    void SaveStateSettings();
    void LoadStateSettings();

    int ledcount;
    int engine;
    bool ledatt[3];
    uint8_t decay;
  
   /** @brief Sets the cv offset from an externally array of data */
    inline void SetSettingsData(int ledcnt, int eng);
    /** @brief Sets the cv offset from an externally array of data */
    inline void SetAttData(bool *ledatts);

    /** @brief Sets the cv offset from an externally array of data */
    inline void GetSettingsData(int &ledcnt, int &eng);
    /** @brief Sets the cv offset from an externally array of data */
    inline void GetAttData(bool *ledatts);

    /** @brief Sets the cv offset from an externally array of data */
    inline void GetStateSettingsData(uint8_t &dec);
    /** @brief Sets the cv offset from an externally array of data */
    inline void SetStateSettingsData(uint8_t dec);
  
 private:
    DaisyMargolis* hw_;
    
};


#endif  // HWTEST_SETTINGS_H_