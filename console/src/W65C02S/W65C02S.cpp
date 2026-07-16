#include "W65C02S.h"
#include <Arduino.h>

namespace W65C02S {

void print_state();

bool setup() {
    Serial.println("--- W65C02S INIT BEGIN ---");

    pinMode(CHIP_TO_ARDUINO[CP_PHI2O], INPUT);

    Serial.println("--- W65C02S INIT OK    ---");
    return true;
}

void print_state() {

    bool reset = digitalRead(CHIP_TO_ARDUINO[CP_RESB]);
    if (!reset) {
        Serial.println("---------------- RESET -----------------");
        return;
    }

    uint16_t addr = 0;
    for (int i = 0; i < 16; i++) {
        if (digitalRead(CHIP_TO_ARDUINO[ADDR_PINS[i]])) {
            addr |= (1 << i);
        }
    }

    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        if (digitalRead(CHIP_TO_ARDUINO[DATA_PINS[i]])) {
            data |= (1 << i);
        }
    }

    bool rw = digitalRead(CHIP_TO_ARDUINO[CP_RWB]);
    bool vpb = !digitalRead(CHIP_TO_ARDUINO[CP_VPB]);
    bool rdy = !digitalRead(CHIP_TO_ARDUINO[CP_RDY]);
    bool mlb = !digitalRead(CHIP_TO_ARDUINO[CP_MLB]);
    bool sync = digitalRead(CHIP_TO_ARDUINO[CP_SYNC]);

    char flags[8];
    memset(flags, 0, 8);
    if (vpb) {
        strcat(flags, "V");
    }

    if (rdy) {
        strcat(flags, "R");
    }

    if (mlb) {
        strcat(flags, "M");
    }

    if (sync) {
        strcat(flags, "S");
    }

    char buffer[64];
    memset(buffer, 0, 64);
    snprintf(buffer, 64, "0x%04X 0x%02X %c %s\n", addr, data, rw ? 'r' : 'W',
             flags);
    Serial.print(buffer);
}

static bool phi2o_prev = false;

void loop() {
    bool phi2o = digitalRead(CHIP_TO_ARDUINO[CP_PHI2O]);
    if (phi2o == HIGH && phi2o_prev == LOW) {
        // Rising Edge
        print_state();
    }

    phi2o_prev = phi2o;
}

} // namespace W65C02S
