#include "settings.h"


using namespace daisy;

void Settings::Init(DaisyMargolis* hw) {
    hw_ = hw;
    LoadSettings();
    LoadStateSettings();
  
    
}

 /** @brief Sets the cv offset from an externally array of data */
inline void Settings::SetSettingsData(int ledcnt, int eng)
{
    ledcount = ledcnt;
    engine = eng;
    
}
 /** @brief Sets the cv offset from an externally array of data */
inline void Settings::SetAttData(bool *ledatts)
{
    
    for(int i = 0; i < 3; i++)
        {
            ledatt[i] = ledatts[i];
        }
}

 /** @brief Sets the cv offset from an externally array of data */
inline void Settings::SetStateSettingsData(uint8_t dec)
{
    decay = dec;
      
}

/** @brief Sets the cv offset from an externally array of data */
inline void Settings::GetSettingsData(int &ledcnt, int &eng)
{
    ledcnt = ledcount;
    eng = engine;
    
}
 /** @brief Sets the cv offset from an externally array of data */
inline void Settings::GetAttData(bool *ledatts)
{
    
    for(int i = 0; i < 3; i++)
        {
            ledatts[i] = ledatt[i];
        }
}

/** @brief Sets the cv offset from an externally array of data */
inline void Settings::GetStateSettingsData(uint8_t &dec)
{
    dec = decay;
       
}





/** @brief Loads and sets settings data */
void Settings::LoadSettings()
{
    daisy::PersistentStorage<MargolisSettings> settings_storage(hw_->seed.qspi);
    MargolisSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK);
    auto &settings_data = settings_storage.GetSettings();
    
    SetSettingsData(settings_data.ledcount,settings_data.engine);
    SetAttData(settings_data.ledatt);
    
}

/** @brief Loads and sets settings data */
void Settings::LoadStateSettings()
{
    daisy::PersistentStorage<StateSettings> settings_storage(hw_->seed.qspi);
    StateSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK*2);
    auto &settings_data = settings_storage.GetSettings();
    
    SetStateSettingsData(settings_data.decay);
        
}

void Settings::SaveSettings()
{
    daisy::PersistentStorage<MargolisSettings> settings_storage(hw_->seed.qspi);
    MargolisSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK);
    auto &settings_data = settings_storage.GetSettings();
    GetSettingsData(settings_data.ledcount,settings_data.engine);
    GetAttData(settings_data.ledatt);
    settings_storage.Save();
    
}

void Settings::SaveStateSettings()
{
    daisy::PersistentStorage<StateSettings> settings_storage(hw_->seed.qspi);
    StateSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK*2);
    auto &settings_data = settings_storage.GetSettings();
    GetStateSettingsData(settings_data.decay);
    settings_storage.Save();
    
}

void Settings::RestoreSettings()
{
    daisy::PersistentStorage<MargolisSettings> settings_storage(hw_->seed.qspi);
    MargolisSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK);
    settings_storage.RestoreDefaults();
    
}