#pragma once
#include <stdint.h>
#define NC 255

namespace W65C02S {

// -----------------------------------------------------------------------------
// W65C02S 40-pin PDIP pinout (fixed by the package — do NOT edit these names).
// The number is the physical pin on the chip; the comment is the signal.
// -----------------------------------------------------------------------------
enum ChipPin : uint8_t {
    CP_VPB = 1,   // Vector Pull      (out, active low)
    CP_RDY = 2,   // Ready            (in/out, active low when held)
    CP_PHI1O = 3, // Phase 1 clock    (out)
    CP_IRQB = 4,  // Interrupt Req.   (in,  active low)
    CP_MLB = 5,   // Memory Lock      (out, active low)
    CP_NMIB = 6,  // Non-Maskable Int (in,  active low, edge)
    CP_SYNC = 7,  // Opcode fetch     (out, active high)
    CP_VDD = 8,   // +5V  -- POWER, do not wire to Arduino
    CP_A0 = 9,
    CP_A1 = 10,
    CP_A2 = 11,
    CP_A3 = 12,
    CP_A4 = 13,
    CP_A5 = 14,
    CP_A6 = 15,
    CP_A7 = 16,
    CP_A8 = 17,
    CP_A9 = 18,
    CP_A10 = 19,
    CP_A11 = 20,
    CP_VSS = 21, // GND  -- POWER, do not wire to Arduino
    CP_A12 = 22,
    CP_A13 = 23,
    CP_A14 = 24,
    CP_A15 = 25,
    CP_D7 = 26,
    CP_D6 = 27,
    CP_D5 = 28,
    CP_D4 = 29,
    CP_D3 = 30,
    CP_D2 = 31,
    CP_D1 = 32,
    CP_D0 = 33,
    CP_RWB = 34,   // Read/Write       (out, high = read, low = write)
    CP_NC35 = 35,  // No connect
    CP_BE = 36,    // Bus Enable       (in,  active high)
    CP_PHI2 = 37,  // Phase 2 clock    (in)  <-- master clock reference
    CP_SOB = 38,   // Set Overflow     (in,  active low)
    CP_PHI2O = 39, // Phase 2 clock    (out)
    CP_RESB = 40,  // Reset            (in,  active low)
};

// -----------------------------------------------------------------------------
// Index into this array with the chip pin number (1-40); index 0 is unused.
// Set an entry to NC if that chip pin is not wired to the Arduino.
//
// Power/ground/no-connect (pins 8, 21, 35) MUST stay NC.
//
// The default layout below uses the Mega's contiguous 22-53 header block for
// the address/data/status lines and the interrupt-capable pins (2, 3, 18-21)
// for the clocks and a couple of extras. Rewire freely — just keep it INPUT-
// only and don't wire a pin twice.
// -----------------------------------------------------------------------------
const uint8_t CHIP_TO_ARDUINO[41] = {
    /*  0 (unused) */ NC,
    /*  1 VPB      */ 23,
    /*  2 RDY      */ 25,
    /*  3 PHI1O    */ 27,
    /*  4 IRQB     */ 29,
    /*  5 MLB      */ 31,
    /*  6 NMIB     */ 33,
    /*  7 SYNC     */ 35,
    /*  8 VDD      */ 37, // +5V power  -- keep NC
    /*  9 A0       */ 39,
    /* 10 A1       */ 41,
    /* 11 A2       */ 43,
    /* 12 A3       */ 45,
    /* 13 A4       */ 47,
    /* 14 A5       */ 19,
    /* 15 A6       */ 18,
    /* 16 A7       */ 17,
    /* 17 A8       */ 16,
    /* 18 A9       */ 15,
    /* 19 A10      */ 14,
    /* 20 A11      */ 8,
    /* 21 VSS      */ NC, // ground     -- keep NC
    /* 22 A12      */ 7,
    /* 23 A13      */ 6,
    /* 24 A14      */ 5,
    /* 25 A15      */ 4,
    /* 26 D7       */ 3,
    /* 27 D6       */ 2,
    /* 28 D5       */ 46,
    /* 29 D4       */ 44,
    /* 30 D3       */ 42,
    /* 31 D2       */ 40,
    /* 32 D1       */ 38,
    /* 33 D0       */ 36,
    /* 34 RWB      */ 34,
    /* 35 NC       */ 32, // no connect -- keep NC
    /* 36 BE       */ 30,
    /* 37 PHI2     */ 28, // master clock reference (interrupt-capable pin)
    /* 38 SOB      */ 26,
    /* 39 PHI2O    */ 24,
    /* 40 RESB     */ 22,
};

const uint8_t ADDR_PINS[] = {CP_A0,  CP_A1,  CP_A2,  CP_A3, CP_A4,  CP_A5,
                             CP_A6,  CP_A7,  CP_A8,  CP_A9, CP_A10, CP_A11,
                             CP_A12, CP_A13, CP_A14, CP_A15};

const uint8_t DATA_PINS[] = {CP_D0, CP_D1, CP_D2, CP_D3,
                             CP_D4, CP_D5, CP_D6, CP_D7};

bool setup();
void loop();

}; // namespace W65C02S
// namespace W65C02S
