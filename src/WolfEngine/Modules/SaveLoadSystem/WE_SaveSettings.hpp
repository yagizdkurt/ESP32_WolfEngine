#pragma once
#include <stdint.h>

/*
============================================================================================
WOLF ENGINE — SAVE/LOAD SYSTEM SETTINGS
Edit this file to configure persistent storage for your game.

Quick setup:
  1. Set WE_SAVE_INTEGRITY = false to disable header/CRC overhead (optional).
  2. Add your EEPROM chip(s) to WE_SAVE_EEPROMS[]. Terminate with { .i2cAddr = 0x00 }.
  3. Add your save slot names to the SaveSlot enum.
  4. Add matching entries to SAVE_SLOTS[] with the size of your save struct.

ENGINE MAINTAINERS: Sections marked "ENGINE MAINTAINERS ONLY" control the driver
registry and must only be changed when adding new EEPROM driver support.
============================================================================================
*/


// ── Integrity ─────────────────────────────────────────────────────────────────
// When true (default): each slot is prefixed with a 4-byte header containing
// a magic number, version byte, and CRC8 checksum. Detects first boot,
// version changes, and bit-flip corruption.
//
// When false: raw bytes only — no header written, no checks on read.
// All integrity code is eliminated at compile time via if constexpr (zero overhead).
constexpr bool WE_SAVE_INTEGRITY = true;

// Magic number written into every slot header. Change only if you need to
// invalidate all existing saves (e.g. after a breaking save format change).
constexpr uint16_t WE_SAVE_MAGIC = 0xB0EF;

// Version byte stored in every slot header. Bump this when you change the
// layout of any save struct — old saves will return WE_ERR_SAVE_VERSION on read.
constexpr uint8_t WE_SAVE_VERSION = 1;


// ── EEPROM driver registry ────────────────────────────────────────────────────
// ENGINE MAINTAINERS ONLY:
//   - Add new driver types to EEPROMDriverType. Do NOT reorder existing entries
//     (the enum value is the index into WE_EEPROM_CAPACITIES[]).
//   - Add the matching capacity in bytes to WE_EEPROM_CAPACITIES[] at the same index.
// ─────────────────────────────────────────────────────────────────────────────
enum class EEPROMDriverType : uint8_t {
    EEPROM_NONE = 0,        // reserved value for "no chip"
    EEPROM_24LC512 = 1,    // 64 KB, 128-byte pages, 5 ms write cycle
    // EEPROM_24LC256 = 2, // 32 KB,  64-byte pages, 5 ms write cycle  — add driver first
    // EEPROM_AT24C32 = 3, //  4 KB,  32-byte pages, 10 ms write cycle — add driver first
};

// ENGINE MAINTAINERS ONLY: indexed 1:1 with EEPROMDriverType above.
// These are the authoritative capacity values — users must not change them.
constexpr uint32_t WE_EEPROM_CAPACITIES[] = {
    /* EEPROM_NONE */ 0,
    /* EEPROM_24LC512 */ 65536,
    // /* EEPROM_24LC256 */ 32768,
    // /* EEPROM_AT24C32 */  4096,
};

// Returns the capacity in bytes for a given driver type.
// Called automatically by SaveManager's compile-time overflow guards.
constexpr uint32_t WE_GetEEPROMCapacity(EEPROMDriverType t) {
    return WE_EEPROM_CAPACITIES[static_cast<uint8_t>(t)];
}


// ── EEPROM chips ──────────────────────────────────────────────────────────────
// USER: list every EEPROM chip on your board here.
// Terminate the array with an entry where i2cAddr = 0x00.
// Valid I²C addresses for 24-series EEPROMs: 0x50–0x57.
// Capacity is derived automatically from the driver type — do not specify it.
// ─────────────────────────────────────────────────────────────────────────────
struct EEPROMConfig {
    uint8_t          i2cAddr = 0x00;   // 0x00 = list terminator
    EEPROMDriverType type = EEPROMDriverType::EEPROM_24LC512;
};

constexpr EEPROMConfig WE_SAVE_EEPROMS[] = {
    { .i2cAddr = 0x50, .type = EEPROMDriverType::EEPROM_24LC512 },
    // { .i2cAddr = 0x51, .type = EEPROMDriverType::EEPROM_24LC512 }, // second chip example
    { .i2cAddr = 0x00 }, // terminator — must be last
};

// Chip count deduced at compile time from the null-terminated array above.
constexpr uint8_t WE_SAVE_EEPROM_COUNT = []() constexpr -> uint8_t {
    uint8_t n = 0;
    while (WE_SAVE_EEPROMS[n].i2cAddr != 0x00) n++;
    return n;
}();


// ── Slot definitions ─────────────────────────────────────────────────────────
// USER:
//   1. Add a named entry to the SaveSlot enum for each save slot you need.
//      Keep SAVE_SLOT_COUNT as the last entry — it is used as the array size.
//   2. Add a matching SaveSlotDef entry to SAVE_SLOTS[] in the same order.
//      .name        — debug label (printed in log output)
//      .size        — maximum bytes for this slot; sizeof(YourStruct) recommended
//      .eepromIndex — which chip this slot lives on (0-based index into WE_SAVE_EEPROMS)
//
// IMPORTANT: The enum order and SAVE_SLOTS[] order must match.
//            A compile-time assert in WE_SaveManager.hpp will catch count mismatches,
//            and another will catch slots with size = 0.
// ─────────────────────────────────────────────────────────────────────────────
struct SaveSlotDef {
    const char* name;        // debug label — printed in SaveManager log output
    uint16_t    size;        // max data bytes for this slot (sizeof your struct)
    uint8_t     eepromIndex; // which chip (0-based index into WE_SAVE_EEPROMS[])
};

enum SaveSlot : uint8_t {
    SAVE_SLOT_0 = 0,
    // SAVE_SLOT_1,
    // SAVE_SLOT_2,
    SAVE_SLOT_COUNT  // always last
};

constexpr SaveSlotDef SAVE_SLOTS[SAVE_SLOT_COUNT] = {
    /* SAVE_SLOT_0 */ { .name = "Slot0", .size = 64, .eepromIndex = 0 },
    // /* SAVE_SLOT_1 */ { .name = "Slot1", .size = 128, .eepromIndex = 0 },
};
