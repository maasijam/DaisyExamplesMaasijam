#include "FLASH_Settings.h"
#include <string.h>
#include <math.h>

// Flash handling - load and save
// 8MB of flash
// 4kB blocks
// assume our settings < 4kB, so put one patch per block
#define FLASH_BLOCK 4096


uint8_t DSY_QSPI_BSS qspi_buffer[FLASH_BLOCK * 16];



//save data to flash memory
QSPIHandle::Result SaveSettings(const Settings &currentSetting)
{
    
    size_t start_address = (size_t)qspi_buffer;

    size_t size = sizeof(Settings);
    
	size_t slot_address = start_address + (0 * FLASH_BLOCK);

    hw.seed.qspi.Erase(slot_address, slot_address + size);

    QSPIHandle::Result retvalue{};    //initialise return value
    retvalue = hw.seed.qspi.Write(slot_address, size, (uint8_t*)&currentSetting);

    return retvalue;
}

//retreive data from flash memory/
Settings LoadSettings()
{
    Settings SettingsInFlash;
    size_t size = sizeof(Settings);

    memcpy(&SettingsInFlash, &qspi_buffer[0 * FLASH_BLOCK], size);

return &SettingsInFlash;

}

