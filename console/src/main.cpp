#include "AT28C256/AT28C256.h"
#include "W65C02S/W65C02S.h"
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println(" --- AR6502-console INIT BEGIN ---");

    // Set all pins as input to avoid logic disruption (except for TX/RX)
    for (int i = 2; i < NUM_DIGITAL_PINS; i++) {
        pinMode(i, INPUT);
    }

    Serial.println(" --- AR6502-console INIT OK    ---");
}

void loop() {
    Serial.print("CHIP SELECT\n [0] W6502S Monitor\n [1] AT28C256 Monitor\n>");
    while (!Serial.available()) {
        delay(10);
    }

    uint8_t chip_num = (uint8_t)Serial.read();
    Serial.print((char)chip_num);
    Serial.println();

    switch (chip_num) {
    case '0': {
        W65C02S::setup();
        while (true)
            W65C02S::loop();
        break;
    }
    case '1': {
        if (AT28C256::setup()) {
            while (true)
                AT28C256::loop();
        }
        break;
    }
    }

    Serial.print("ERR: Invalid Chip Number: ");
    Serial.println((char)chip_num);
}