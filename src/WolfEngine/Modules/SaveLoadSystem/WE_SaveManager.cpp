#define MODULE_DEBUG_ENABLED  // comment out to silence all SaveManager log output
#include "WE_SaveManager.hpp"
#include "WolfEngine/Utilities/WE_Debug.h"
#include <new>

// ─────────────────────────────────────────────────────────────────────────────
//  WE_SaveManager — implementation
// ─────────────────────────────────────────────────────────────────────────────

void WE_SaveManager::OnInit() {
    if constexpr (WE_SAVE_EEPROM_COUNT == 0 || WE_SAVE_EEPROMS[0].type == EEPROMDriverType::EEPROM_NONE) return;
    
    for (uint8_t i = 0; i < WE_SAVE_EEPROM_COUNT; i++) {
        switch (WE_SAVE_EEPROMS[i].type) {
            case EEPROMDriverType::EEPROM_NONE:
                break;
            case EEPROMDriverType::EEPROM_24LC512:
                new (m_driverBufs[i]) EEPROM24LC512(WE_SAVE_EEPROMS[i].i2cAddr);
                break;
            // case EEPROMDriverType::EEPROM_24LC256:
            //     new (m_driverBufs[i]) WE_EEPROM24LC256(WE_SAVE_EEPROMS[i].i2cAddr);
            //     break;
        }
        m_eeproms[i] = reinterpret_cast<WE_IEEPROMDriver*>(m_driverBufs[i]);
        DebugLog("SaveManager", "EEPROM[%u] type=%u addr=0x%02X capacity=%lu B",
                 i,
                 static_cast<uint8_t>(WE_SAVE_EEPROMS[i].type),
                 WE_SAVE_EEPROMS[i].i2cAddr,
                 static_cast<unsigned long>(WE_GetEEPROMCapacity(WE_SAVE_EEPROMS[i].type)));
    }

    DebugLog("SaveManager", "init complete — %u chip(s), %u slot(s), integrity=%s",
             WE_SAVE_EEPROM_COUNT,
             static_cast<uint8_t>(SAVE_SLOT_COUNT),
             WE_SAVE_INTEGRITY ? "ON" : "OFF");
}

uint16_t WE_SaveManager::getSlotAddress(SaveSlot slot) const {
    if constexpr (WE_SAVE_EEPROM_COUNT == 0 || WE_SAVE_EEPROMS[0].type == EEPROMDriverType::EEPROM_NONE) return 0;

    constexpr uint16_t HEADER = WE_SAVE_INTEGRITY ? 4 : 0;
    uint8_t  myChip = SAVE_SLOTS[slot].eepromIndex;
    uint16_t addr   = 0;

    // Sum all earlier slots that share the same chip, in enum order.
    for (uint8_t i = 0; i < static_cast<uint8_t>(slot); i++) {
        if (SAVE_SLOTS[i].eepromIndex == myChip)
            addr += HEADER + SAVE_SLOTS[i].size;
    }
    return addr;
}

esp_err_t WE_SaveManager::erase(SaveSlot slot) {
    if constexpr (WE_SAVE_EEPROM_COUNT == 0 || WE_SAVE_EEPROMS[0].type == EEPROMDriverType::EEPROM_NONE) return ESP_ERR_INVALID_ARG;

    if (static_cast<uint8_t>(slot) >= static_cast<uint8_t>(SAVE_SLOT_COUNT)) return ESP_ERR_INVALID_ARG;

    constexpr uint16_t HEADER = WE_SAVE_INTEGRITY ? 4 : 0;
    uint16_t addr  = getSlotAddress(slot);
    uint16_t len   = HEADER + SAVE_SLOTS[slot].size;
    uint8_t  eidx  = SAVE_SLOTS[slot].eepromIndex;

    // Write 0xFF in page-sized chunks using a stack buffer.
    constexpr uint16_t CHUNK = EEPROM24LC512::PAGE_SIZE;
    uint8_t blank[CHUNK];
    memset(blank, 0xFF, CHUNK);

    uint16_t written = 0;
    while (written < len) {
        uint16_t chunk = (len - written < CHUNK) ? (len - written) : CHUNK;
        esp_err_t err = m_eeproms[eidx]->writeBytes(addr + written, blank, chunk);
        if (err != ESP_OK) {
            DebugErr("SaveManager", "erase slot %u failed at offset %u: err=0x%x",
                     static_cast<uint8_t>(slot), written, err);
            return err;
        }
        written += chunk;
    }

    DebugLog("SaveManager", "erased slot %u ('%s') %u bytes on EEPROM[%u]",
             static_cast<uint8_t>(slot), SAVE_SLOTS[slot].name, len, eidx);
    return ESP_OK;
}

esp_err_t WE_SaveManager::eraseAll() {
    if constexpr (WE_SAVE_EEPROM_COUNT == 0 || WE_SAVE_EEPROMS[0].type == EEPROMDriverType::EEPROM_NONE) return ESP_ERR_INVALID_ARG;

    DebugLog("SaveManager", "erasing all %u EEPROM chip(s)...", WE_SAVE_EEPROM_COUNT);
    for (uint8_t i = 0; i < WE_SAVE_EEPROM_COUNT; i++) {
        esp_err_t err = m_eeproms[i]->eraseAll();
        if (err != ESP_OK) {
            DebugErr("SaveManager", "eraseAll failed on EEPROM[%u]: err=0x%x", i, err);
            return err;
        }
        DebugLog("SaveManager", "EEPROM[%u] erased", i);
    }
    return ESP_OK;
}
