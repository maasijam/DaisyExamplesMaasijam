#ifndef MV_SETTINGS_H_
#define MV_SETTINGS_H_

#include "../daisy_hank.h"

using namespace daisy;


#define FLASH_BLOCK 4096

  
/** Custom struct for  data to save. 
  *  The defaults can be set in the constructor,
  *  the initialization list, or manually set before
  *  calling the "PesistentStorage::Init" function.
*/
struct MultiversioSettings 
{ 
    MultiversioSettings() : mode(0), cvToCtrl(0) {}
    int mode, cvToCtrl;
    


    /** @brief checks sameness */
    bool operator==(const MultiversioSettings &rhs)
    {
      if(mode != rhs.mode)  {
            return false;
        }
        else if(cvToCtrl != rhs.cvToCtrl)
        {
            return false;
        }
       
        
        return true;
    }

    /** @brief Not equal operator */
    bool operator!=(const MultiversioSettings &rhs) { return !operator==(rhs); }
};



class Settings {
 public:
    Settings() { }
    ~Settings() { }
    
    void Init(DaisyHank* hw);

    void RestoreSettings();
    void SaveSettings();
    void LoadSettings();

    int mode;
    int cvToCtrl;
    
  
   /** @brief Sets the cv offset from an externally array of data */
    inline void SetSettingsData(MultiversioSettings mvsettings);
    /** @brief Sets the cv offset from an externally array of data */
    
    /** @brief Sets the cv offset from an externally array of data */
    inline void GetSettingsData(MultiversioSettings &mvsettings);
    /** @brief Sets the cv offset from an externally array of data */
   

    inline const MultiversioSettings& mvsettings() const {
        return mv_settings_;
    }

    inline MultiversioSettings* hank_mvsettings() {
        return &mv_settings_;
    }
    
    
   
  
 private:
    DaisyHank* hw_;
    MultiversioSettings mv_settings_;
    
};


#endif  // MV_SETTINGS_H_