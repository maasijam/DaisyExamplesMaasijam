#include "settings.h"


using namespace daisy;

void Settings::Init(DaisyHank* hw) {
    hw_ = hw;
    LoadSettings();
}

 /** @brief Sets the cv offset from an externally array of data */
inline void Settings::SetSettingsData(MultiversioSettings mvsettings)
{
    mv_settings_ = mvsettings;
    
}




/** @brief Sets the cv offset from an externally array of data */
inline void Settings::GetSettingsData(MultiversioSettings &mvsettings)
{
    mvsettings = mv_settings_;
    
}




/** @brief Loads and sets settings data */
void Settings::LoadSettings()
{
    daisy::PersistentStorage<MultiversioSettings> settings_storage(hw_->seed.qspi);
    MultiversioSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK);
    MultiversioSettings &settings_data = settings_storage.GetSettings();
    
    SetSettingsData(settings_data);
    //SetAttData(settings_data.ledatt);
    
}


void Settings::SaveSettings()
{
    daisy::PersistentStorage<MultiversioSettings> settings_storage(hw_->seed.qspi);
    MultiversioSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK);
    MultiversioSettings &settings_data = settings_storage.GetSettings();
    GetSettingsData(settings_data);
    
    settings_storage.Save();
    
}


void Settings::RestoreSettings()
{
    daisy::PersistentStorage<MultiversioSettings> settings_storage(hw_->seed.qspi);
    MultiversioSettings default_settings;
    settings_storage.Init(default_settings, FLASH_BLOCK);
    settings_storage.RestoreDefaults();
    MultiversioSettings &settings_data = settings_storage.GetSettings();
    SetSettingsData(settings_data);
    
}

