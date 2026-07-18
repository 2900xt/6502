#pragma once
#include "../W65C02S/W65C02S.h"
#include <stdint.h>

// Sentinel for an AT28C256 pin that is NOT wired to the Arduino (power/ground).
#define NC 255

namespace AT28C256 {

// AT28C256 28-pin DIP pinout (fixed by the package -- do NOT edit these names).
// The number is the physical pin on the chip; the name is the signal.
enum ChipPin : uint8_t {
    CP_A14 = 1,
    CP_A12 = 2,
    CP_A7 = 3,
    CP_A6 = 4,
    CP_A5 = 5,
    CP_A4 = 6,
    CP_A3 = 7,
    CP_A2 = 8,
    CP_A1 = 9,
    CP_A0 = 10,
    CP_IO0 = 11,
    CP_IO1 = 12,
    CP_IO2 = 13,
    CP_GND = 14,
    CP_IO3 = 15,
    CP_IO4 = 16,
    CP_IO5 = 17,
    CP_IO6 = 18,
    CP_IO7 = 19,
    CP_CE = 20,
    CP_A10 = 21,
    CP_OE = 22,
    CP_A11 = 23,
    CP_A9 = 24,
    CP_A8 = 25,
    CP_A13 = 26,
    CP_WE = 27,
    CP_VCC = 28,
};

// Index with the chip pin number; index 0 is unused. Power/ground (14, 28) =
// NC.
const uint8_t CHIP_TO_ARDUINO[29] = {
    /*  0 (unused) */ NC,
    /*  1 A14      */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A14],
    /*  2 A12      */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A12],
    /*  3 A7       */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A7],
    /*  4 A6       */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A6],
    /*  5 A5       */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A5],
    /*  6 A4       */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A4],
    /*  7 A3       */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A3],
    /*  8 A2       */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A2],
    /*  9 A1       */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A1],
    /* 10 A0       */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A0],
    /* 11 I/O0     */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_D0],
    /* 12 I/O1     */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_D1],
    /* 13 I/O2     */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_D2],
    /* 14 GND      */ NC, // ground  -- keep NC
    /* 15 I/O3     */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_D3],
    /* 16 I/O4     */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_D4],
    /* 17 I/O5     */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_D5],
    /* 18 I/O6     */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_D6],
    /* 19 I/O7     */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_D7],
    /* 20 CE#      */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A15], // chip enable,   active low
    /* 21 A10      */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A10],
    /* 22 OE#      */ 9, // output enable, active low
    /* 23 A11      */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A11],
    /* 24 A9       */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A9],
    /* 25 A8       */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A8],
    /* 26 A13      */ W65C02S::CHIP_TO_ARDUINO[W65C02S::CP_A13],
    /* 27 WE#      */ 10, // write enable,  active low
    /* 28 VCC      */ NC,  // +5V     -- keep NC
};

// Bit-ordered views derived from the chip map above (single source of truth).
// Data bit n uses DATA_PINS[n]. Address bit n uses ADDR_PINS[n].
const uint8_t DATA_PINS[8] = {
    CHIP_TO_ARDUINO[CP_IO0], CHIP_TO_ARDUINO[CP_IO1], CHIP_TO_ARDUINO[CP_IO2],
    CHIP_TO_ARDUINO[CP_IO3], CHIP_TO_ARDUINO[CP_IO4], CHIP_TO_ARDUINO[CP_IO5],
    CHIP_TO_ARDUINO[CP_IO6], CHIP_TO_ARDUINO[CP_IO7],
};
const uint8_t ADDR_PINS[15] = {
    CHIP_TO_ARDUINO[CP_A0],  CHIP_TO_ARDUINO[CP_A1],  CHIP_TO_ARDUINO[CP_A2],
    CHIP_TO_ARDUINO[CP_A3],  CHIP_TO_ARDUINO[CP_A4],  CHIP_TO_ARDUINO[CP_A5],
    CHIP_TO_ARDUINO[CP_A6],  CHIP_TO_ARDUINO[CP_A7],  CHIP_TO_ARDUINO[CP_A8],
    CHIP_TO_ARDUINO[CP_A9],  CHIP_TO_ARDUINO[CP_A10], CHIP_TO_ARDUINO[CP_A11],
    CHIP_TO_ARDUINO[CP_A12], CHIP_TO_ARDUINO[CP_A13], CHIP_TO_ARDUINO[CP_A14],
};

const uint8_t PIN_CE = CHIP_TO_ARDUINO[CP_CE]; // active low
const uint8_t PIN_OE = CHIP_TO_ARDUINO[CP_OE]; // active low
const uint8_t PIN_WE = CHIP_TO_ARDUINO[CP_WE]; // active low

constexpr uint32_t EEPROM_SIZE = 32768UL;

bool setup();
void loop();

} // namespace AT28C256
