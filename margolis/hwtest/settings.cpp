#include "settings.h"


using namespace daisy;

void Settings::Init(DaisyMargolis* hw) {
    hw_ = hw;
    LoadSettings();
    LoadStateSettings();
  
    
}

 /** @brief Sets the cv offset from an externally array of data */
inline void Settings::SetSettingsData(HwtestSettings hwsettings)
{
    hwtest_settings_ = hwsettings;
    
}


 /** @brief Sets the cv offset from an externally array of data */
inline void Settings::SetStateSettingsData(StateSettings stateset)
{
    state_settings_ = stateset;
      
}

/** @brief Sets the cv offset from an externally array of data */
inline void Settings::GetSettingsData(HwtestSettings &hwsettings)
{
    hwsettings = hwtest_settings_;
    
}

/** @brief Sets the cv offset from an externally array of data */
inline void Settings::GetStateSettingsData(StateSettings &stateset)
{
    stateset = state_settings_;
       
}





/** @brief Loads and sets settings data */
void Settings::LoadSettings()
{
    daisy::PersistentStorage<HwtestSettings> settings_storage(hw_->seed.qspi);
    HwtestSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK);
    HwtestSettings &settings_data = settings_storage.GetSettings();
    
    SetSettingsData(settings_data);
    //SetAttData(settings_data.ledatt);
    
}

/** @brief Loads and sets settings data */
void Settings::LoadStateSettings()
{
    daisy::PersistentStorage<StateSettings> settings_storage(hw_->seed.qspi);
    StateSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK*2);
    StateSettings &settings_data = settings_storage.GetSettings();
    
    SetStateSettingsData(settings_data);
        
}

void Settings::SaveSettings()
{
    daisy::PersistentStorage<HwtestSettings> settings_storage(hw_->seed.qspi);
    HwtestSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK);
    HwtestSettings &settings_data = settings_storage.GetSettings();
    GetSettingsData(settings_data);
    
    settings_storage.Save();
    
}

void Settings::SaveStateSettings()
{
    daisy::PersistentStorage<StateSettings> settings_storage(hw_->seed.qspi);
    StateSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK*2);
    StateSettings &settings_data = settings_storage.GetSettings();
    GetStateSettingsData(settings_data);
    settings_storage.Save();
    
}

void Settings::RestoreSettings()
{
    daisy::PersistentStorage<HwtestSettings> settings_storage(hw_->seed.qspi);
    HwtestSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK);
    settings_storage.RestoreDefaults();
    
}


//HwtestSettings hwtest_settings_;
//StateSettings state_settings_;