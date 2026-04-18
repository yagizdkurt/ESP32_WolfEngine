#pragma once
#include "WE_SaveSettings.hpp"
#include "esp_err.h"
#include <type_traits>
#include <string.h>

// Module
#include "WolfEngine/modules/WE_IModule.hpp"

// EEPROM Drivers
#include "WolfEngine/Drivers/EepromDrivers/WE_IEEPROMDriver.hpp"
#include "WolfEngine/Drivers/EepromDrivers/WE_EEPROM24LC512.hpp"

// ─────────────────────────────────────────────────────────────────────────────
//  Custom error codes (esp_err_t compatible)
// ─────────────────────────────────────────────────────────────────────────────
#define WE_ERR_SAVE_EMPTY    ((esp_err_t)0x2001)  // slot was never written (first boot)
#define WE_ERR_SAVE_CORRUPT  ((esp_err_t)0x2002)  // CRC8 mismatch — data may be corrupted
#define WE_ERR_SAVE_VERSION  ((esp_err_t)0x2003)  // WE_SAVE_VERSION changed since last write
#define WE_ERR_SAVE_OVERFLOW ((esp_err_t)0x2004)  // sizeof(T) exceeds slot's defined size

// ─────────────────────────────────────────────────────────────────────────────
//  Compile-time guards
// ─────────────────────────────────────────────────────────────────────────────

// Catch count mismatches: enum entry added but no matching SAVE_SLOTS[] entry (or vice versa).
static_assert(
    static_cast<uint8_t>(SAVE_SLOT_COUNT) == sizeof(SAVE_SLOTS) / sizeof(SAVE_SLOTS[0]),
    "WE_SaveSettings: SAVE_SLOTS array size must match SAVE_SLOT_COUNT"
);

// Catch slots with zero size — usually means a SAVE_SLOTS[] entry was forgotten.
namespace _WE_SaveGuards {
    constexpr bool allSlotsHaveSize() {
        for (uint8_t i = 0; i < static_cast<uint8_t>(SAVE_SLOT_COUNT); i++)
            if (SAVE_SLOTS[i].size == 0) return false;
        return true;
    }

    // Returns total bytes consumed on a given chip by all assigned slots (header included).
    constexpr uint32_t totalBytesOnChip(uint8_t chipIdx) {
        constexpr uint16_t HEADER = WE_SAVE_INTEGRITY ? 4 : 0;
        uint32_t total = 0;
        for (uint8_t i = 0; i < static_cast<uint8_t>(SAVE_SLOT_COUNT); i++)
            if (SAVE_SLOTS[i].eepromIndex == chipIdx)
                total += HEADER + SAVE_SLOTS[i].size;
        return total;
    }

    constexpr bool allChipsFit() {
        for (uint8_t c = 0; c < WE_SAVE_EEPROM_COUNT; c++)
            if (totalBytesOnChip(c) > WE_GetEEPROMCapacity(WE_SAVE_EEPROMS[c].type))
                return false;
        return true;
    }
}

static_assert(_WE_SaveGuards::allSlotsHaveSize(),
    "WE_SaveSettings: one or more SAVE_SLOTS[] entries have size = 0 (missing definition?)");

static_assert(_WE_SaveGuards::allChipsFit(),
    "WE_SaveSettings: slot assignments exceed EEPROM capacity on one or more chips");

// ─────────────────────────────────────────────────────────────────────────────
//  Driver placement-new buffer size
//  ENGINE MAINTAINERS ONLY: add sizeof for each new concrete driver inside
//  the lambda. The max is selected automatically; intermediate values are
//  scoped inside the lambda and disappear after compile time.
// ─────────────────────────────────────────────────────────────────────────────
constexpr size_t WE_EEPROM_DRIVER_BUF_SIZE = []() constexpr -> size_t {
    size_t sizes[] = {
        sizeof(EEPROM24LC512),
        // sizeof(WE_EEPROM24LC256),   // uncomment when that driver exists
    };
    size_t max = 0;
    for (size_t s : sizes) if (s > max) max = s;
    return max;
}();

// ─────────────────────────────────────────────────────────────────────────────
//  WE_SaveManager
// ─────────────────────────────────────────────────────────────────────────────
class WolfEngine;


class WE_SaveManager : public TModule<WE_SaveManager, 0> {
public:
    WE_SaveManager() : TModule<WE_SaveManager, 0>("SaveManager") {}

    // ── Write ─────────────────────────────────────────────────────────────────
    // Serializes T as raw bytes and writes it to the given slot.
    // T must be trivially copyable (plain structs — no vtables, no owning pointers).
    // Blocks for 5–20 ms per 128-byte EEPROM page written.
    // Call only at safe moments (between levels, pause screen) — NOT every frame.
    template<typename T>
    esp_err_t write(SaveSlot slot, const T& data) {
        static_assert(std::is_trivially_copyable<T>::value,
            "WE_SaveManager::write<T>: T must be trivially copyable");

        if (static_cast<uint8_t>(slot) >= static_cast<uint8_t>(SAVE_SLOT_COUNT))
            return ESP_ERR_INVALID_ARG;

        if (sizeof(T) > SAVE_SLOTS[slot].size)
            return WE_ERR_SAVE_OVERFLOW;

        uint16_t addr = getSlotAddress(slot);
        uint8_t  eidx = SAVE_SLOTS[slot].eepromIndex;

        if constexpr (WE_SAVE_INTEGRITY) {
            constexpr size_t TOTAL = 4 + sizeof(T);
            uint8_t buf[TOTAL];
            buf[0] = static_cast<uint8_t>(WE_SAVE_MAGIC & 0xFF);
            buf[1] = static_cast<uint8_t>(WE_SAVE_MAGIC >> 8);
            buf[2] = WE_SAVE_VERSION;
            memcpy(buf + 4, &data, sizeof(T));
            buf[3] = crc8(buf + 4, sizeof(T));
            return m_eeproms[eidx]->writeBytes(addr, buf, TOTAL);
        } else {
            return m_eeproms[eidx]->writeBytes(
                addr, reinterpret_cast<const uint8_t*>(&data), sizeof(T));
        }
    }

    // ── Read ──────────────────────────────────────────────────────────────────
    // Reads the slot and deserializes bytes into outData.
    // Returns:
    //   ESP_OK              — success, outData is valid
    //   WE_ERR_SAVE_EMPTY   — slot never written (first boot); outData unchanged
    //   WE_ERR_SAVE_VERSION — WE_SAVE_VERSION changed; outData unchanged
    //   WE_ERR_SAVE_CORRUPT — CRC mismatch; outData unchanged
    //   WE_ERR_SAVE_OVERFLOW— sizeof(T) > slot size; nothing read
    template<typename T>
    esp_err_t read(SaveSlot slot, T& outData) {
        static_assert(std::is_trivially_copyable<T>::value,
            "WE_SaveManager::read<T>: T must be trivially copyable");

        if (static_cast<uint8_t>(slot) >= static_cast<uint8_t>(SAVE_SLOT_COUNT))
            return ESP_ERR_INVALID_ARG;

        if (sizeof(T) > SAVE_SLOTS[slot].size)
            return WE_ERR_SAVE_OVERFLOW;

        uint16_t addr = getSlotAddress(slot);
        uint8_t  eidx = SAVE_SLOTS[slot].eepromIndex;

        if constexpr (WE_SAVE_INTEGRITY) {
            constexpr size_t TOTAL = 4 + sizeof(T);
            uint8_t buf[TOTAL];

            esp_err_t err = m_eeproms[eidx]->readBytes(addr, buf, TOTAL);
            if (err != ESP_OK) return err;

            // Magic check
            uint16_t magic = static_cast<uint16_t>(buf[0]) | (static_cast<uint16_t>(buf[1]) << 8);
            if (magic != WE_SAVE_MAGIC) return WE_ERR_SAVE_EMPTY;

            // Version check
            if (buf[2] != WE_SAVE_VERSION) return WE_ERR_SAVE_VERSION;

            // CRC8 check
            if (buf[3] != crc8(buf + 4, sizeof(T))) return WE_ERR_SAVE_CORRUPT;

            memcpy(&outData, buf + 4, sizeof(T));
            return ESP_OK;
        } else {
            return m_eeproms[eidx]->readBytes(
                addr, reinterpret_cast<uint8_t*>(&outData), sizeof(T));
        }
    }

    // Fill one slot's region on its EEPROM with 0xFF.
    // After erasing, read() will return WE_ERR_SAVE_EMPTY for that slot.
    esp_err_t erase(SaveSlot slot);

    // Erase every chip entirely (all 0xFF). Very slow — never call during gameplay.
    esp_err_t eraseAll();

    // Returns the byte address of a slot's start within its chip.
    // Accounts for header size and all earlier slots assigned to the same chip.
    uint16_t getSlotAddress(SaveSlot slot) const;

private:          
    friend class WolfEngine;
    friend class ModuleSystem;

    void OnInit() override;

    // Placement-new buffers — sized to fit the largest concrete EEPROM driver.
    uint8_t           m_driverBufs[WE_SAVE_EEPROM_COUNT][WE_EEPROM_DRIVER_BUF_SIZE];
    WE_IEEPROMDriver* m_eeproms   [WE_SAVE_EEPROM_COUNT] = {};

    // CRC-8/SMBUS (polynomial 0x07). Used only when WE_SAVE_INTEGRITY = true.
    static uint8_t crc8(const uint8_t* data, size_t len) {
        uint8_t crc = 0;
        for (size_t i = 0; i < len; i++) {
            crc ^= data[i];
            for (int b = 0; b < 8; b++)
                crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : crc << 1;
        }
        return crc;
    }

};