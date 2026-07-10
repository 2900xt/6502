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
// >>> EDIT ME <<<  Chip pin  ->  Arduino Mega digital pin.
//
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
    /*  1 VPB      */ 19,
    /*  2 RDY      */ 18,
    /*  3 PHI1O    */ 17,
    /*  4 IRQB     */ 16,
    /*  5 MLB      */ 15,
    /*  6 NMIB     */ 14,
    /*  7 SYNC     */ 12,
    /*  8 VDD      */ 23, // +5V power  -- keep NC
    /*  9 A0       */ 25,
    /* 10 A1       */ 27,
    /* 11 A2       */ 29,
    /* 12 A3       */ 31,
    /* 13 A4       */ 33,
    /* 14 A5       */ 35,
    /* 15 A6       */ 37,
    /* 16 A7       */ 39,
    /* 17 A8       */ 41,
    /* 18 A9       */ 43,
    /* 19 A10      */ 45,
    /* 20 A11      */ 47,
    /* 21 VSS      */ 46, // ground     -- keep NC
    /* 22 A12      */ 44,
    /* 23 A13      */ 42,
    /* 24 A14      */ 40,
    /* 25 A15      */ 38,
    /* 26 D7       */ 36,
    /* 27 D6       */ 34,
    /* 28 D5       */ 32,
    /* 29 D4       */ 30,
    /* 30 D3       */ 28,
    /* 31 D2       */ 26,
    /* 32 D1       */ 24,
    /* 33 D0       */ 22,
    /* 34 RWB      */ 7,
    /* 35 NC       */ 6, // no connect -- keep NC
    /* 36 BE       */ 5,
    /* 37 PHI2     */ 4, // master clock reference (interrupt-capable pin)
    /* 38 SOB      */ 3,
    /* 39 PHI2O    */ 2,
    /* 40 RESB     */ 13,
};

// -----------------------------------------------------------------------------
// Sampling behaviour.
// -----------------------------------------------------------------------------
// The clock reference for sampling is PHI2O (chip pin 39), the buffered
// in-phase phase-2 clock output. The 6502 puts a valid address on the bus
// during PHI2-low and latches/presents data around the PHI2 rising edge; the
// whole cycle's bus contents are valid and stable while PHI2 is HIGH. We
// therefore sample one snapshot per clock cycle on the RISING edge of PHI2O.
// Set to FALLING if your build prefers it. PHI2O (pin 39) must be on an
// interrupt-capable Mega pin (2, 3, 18, 19, 20, 21).
#define SAMPLE_EDGE RISING

void setup();
void loop();

} // namespace W65C02S