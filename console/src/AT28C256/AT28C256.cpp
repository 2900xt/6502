#include "AT28C256.h"
#include <Arduino.h>
#include <stdlib.h>
#include <sys/types.h>

namespace AT28C256 {

static bool g_OE, g_WE, g_IO;

inline void latchOE(bool state) {
    if (g_OE != state) {
        digitalWrite(PIN_OE, state);
        g_OE = state;
    }
}

inline void latchWE(bool state) {
    if (g_WE != state) {
        digitalWrite(PIN_WE, state);
        g_WE = state;
    }
}

inline void latchIO(bool state) {
    if (g_IO != state) {
        g_IO = state;
        for (uint8_t i = 0; i < 8; i++) {
            pinMode(DATA_PINS[i], state);
        }
    }
}

uint8_t read_byte(uint16_t addr) {
    // no writes, enable outputs
    latchWE(HIGH);
    latchOE(LOW);
    latchIO(INPUT);

    for (uint8_t i = 0; i <= 14; i++) {
        uint16_t mask = (1 << i);
        digitalWrite(ADDR_PINS[i], (addr & mask) != 0);
    }

    // enable output
    digitalWrite(PIN_OE, LOW);

    uint8_t byte = 0;
    for (uint8_t i = 0; i <= 7; i++) {
        uint8_t mask = (1 << i);
        bool active = digitalRead(DATA_PINS[i]);
        if (active)
            byte += mask;
    }

    return byte;
}

bool write_timeout(uint16_t addr, uint8_t byte) {
    uint32_t ms_start = millis();
    while ((read_byte(addr) & 0x80) != (byte & 0x80)) {
        uint32_t elapsed = millis() - ms_start;
        if (elapsed > 10) {
            // 10 ms timeout
            Serial.println("ERR");
            return false;
        }
    }

    return true;
}

void write_byte(uint16_t addr, uint8_t byte) {
    latchOE(HIGH);
    latchIO(OUTPUT);

    for (uint8_t i = 0; i <= 14; i++) {
        uint16_t mask = (1 << i);
        digitalWrite(ADDR_PINS[i], (addr & mask) != 0);
    }

    for (uint8_t i = 0; i < 8; i++) {
        uint8_t mask = (1 << i);
        digitalWrite(DATA_PINS[i], (byte & mask) != 0);
    }

    latchWE(LOW);
    delayMicroseconds(1);
    latchWE(HIGH);

    // poll IO7
    uint32_t ms_start = millis();
    while ((read_byte(addr) & 0x80) != (byte & 0x80)) {
        uint32_t elapsed = millis() - ms_start;
        if (elapsed > 10) {
            // 10 ms timeout
            Serial.println("WRITE ERR");
            return;
        }
    }
}

#define PAGE_SIZE (1 << 6)
static uint8_t page_write_data[PAGE_SIZE];

void write_page(uint16_t addr) {
    if (((addr >> 6) << 6) != addr) {
        Serial.println("ERR");
        return;
    }

    latchOE(HIGH);
    latchIO(OUTPUT);

    for (uint8_t i = 6; i <= 14; i++) {
        uint16_t mask = (1 << i);
        digitalWrite(ADDR_PINS[i], (addr & mask) != 0);
    }

    for (uint16_t offset = 0; offset < PAGE_SIZE; offset++) {
        for (uint8_t i = 0; i < 6; i++) {
            uint8_t mask = (1 << i);
            digitalWrite(ADDR_PINS[i], (offset & mask) != 0);
        }

        for (uint8_t i = 0; i < 8; i++) {
            uint8_t mask = (1 << i);
            digitalWrite(DATA_PINS[i], (page_write_data[offset] & mask) != 0);
        }

        latchWE(LOW);

        delayMicroseconds(1);

        latchWE(HIGH);
    }

    uint16_t last_addr = addr + PAGE_SIZE - 1;
    // poll IO7
    uint32_t ms_start = millis();
    while ((read_byte(last_addr) & 0x80) !=
           (page_write_data[PAGE_SIZE - 1] & 0x80)) {
        uint32_t elapsed = millis() - ms_start;
        if (elapsed > 10) {
            // 10 ms timeout
            Serial.println("ERR");
            return;
        }
    }
}

void verify_write_page(uint16_t addr) {
    if (((addr >> 6) << 6) != addr) {
        Serial.println("ERR");
        return;
    }

    for (uint16_t cur = addr; cur < addr + PAGE_SIZE; cur++) {
        uint8_t byte = read_byte(cur);
        if (byte != page_write_data[cur - addr]) {
            Serial.print("VERIFY ERR at 0x");
            Serial.print(cur, HEX);
            Serial.print(": EXP 0x");
            Serial.print(page_write_data[cur - addr]);
            Serial.print(" GOT 0x");
            Serial.println(byte, HEX);
            return;
        }
    }

    Serial.println("OK");
}

bool self_test() {
    srand(micros());
    for (uint8_t i = 0; i < 32; i++) {
        uint16_t addr = rand() % EEPROM_SIZE;
        uint8_t old_byte = read_byte(addr);

        uint8_t bytes[] = {
            0xAA, 0x55, 0x20, 0x40, 0x60, 0x00, 0xFF,
        };

        for (int i = 0; i < 7; i++) {
            write_byte(addr, bytes[i]);
            uint8_t a = read_byte(addr);
            if (a != bytes[i]) {
                Serial.print("SELF TEST FAIL: ");
                Serial.print(addr, HEX);
                Serial.print(" act: ");
                Serial.print(a, HEX);
                Serial.print(" exp: ");
                Serial.print(bytes[i], HEX);
                Serial.println();
                return false;
            }
        }

        write_byte(addr, old_byte);
        if (read_byte(addr) != old_byte) {
            Serial.print("SELF TEST FAIL: 0x");
            Serial.print(addr, HEX);
            Serial.println();
            return false;
        }
    }

    Serial.println("SELF TEST OK");

    return true;
}

bool setup() {
    Serial.println("--- AT28C256 INIT BEGIN ---");

    // Set CE, OE, and WE as outputs

    pinMode(PIN_CE, OUTPUT);
    pinMode(PIN_OE, OUTPUT);
    pinMode(PIN_WE, OUTPUT);

    g_IO = INPUT;
    g_WE = HIGH;
    g_OE = HIGH;

    digitalWrite(PIN_WE, HIGH);
    digitalWrite(PIN_OE, HIGH);

    // Set ADDR pins to output

    for (uint8_t i = 0; i < 15; i++) {
        pinMode(ADDR_PINS[i], OUTPUT);
    }

    // enable the chip
    digitalWrite(PIN_CE, LOW);

    if (!self_test()) {
        return false;
    }

    Serial.println("--- AT28C256 INIT OK    ---");
    return true;
}

char input_buffer[64];
uint8_t input_ptr = 0;

void loop() {
    Serial.print(">");
    input_ptr = 0;
    memset(input_buffer, 0, 64);

    // get command

    while (true) {
        while (!Serial.available())
            ;
        char cur = Serial.read();

        if (cur == '\b') {
            if (input_ptr != 0) {
                input_ptr--;
                input_buffer[input_ptr] = 0;
                Serial.print("\b \b");
            }
            continue;
        }

        if (cur == '\r')
            continue;

        if (cur == '\n') {
            Serial.println();
            break;
        }

        input_buffer[input_ptr++] = cur;

        if (input_ptr == 63) {
            input_ptr--;
            Serial.print("\b \b");
        }

        Serial.print(cur);
    }

    // process command
    if (input_buffer[0] == '?') {
        // help
        Serial.println("HELP         '?'");

        Serial.println("WRITE BYTE   'w <addr> <byte>'");
        Serial.println("             'w FFFC A5' ~> '(OK, ERR)");

        Serial.println("READ BYTE    'r <addr>'");
        Serial.println("             'r FFFC' ~> '(A5, ERR)'");

        Serial.println("RESET        'R'");

        Serial.println(
            "WRITE PAGE   'P <addr>' -> binary data, 64 bytes + 'OK");
        Serial.println(
            "             'P FF00' -> '\x54\x32\x67...' -> OK ~> (OK, ERR)");

        Serial.println("SELF TEST    'S'");
    } else if (input_buffer[0] == 'w') {
        // write byte

        if (input_ptr != 9)
            goto invalid_cmd;

        char addr_hex[5] = {}, byte_hex[3] = {};
        strncpy(addr_hex, input_buffer + 2, 4);
        strncpy(byte_hex, input_buffer + 7, 2);

        char *end_ptr;
        uint16_t addr = strtol(addr_hex, &end_ptr, HEX);
        if (*end_ptr != '\0')
            goto invalid_cmd;

        uint16_t byte = strtol(byte_hex, &end_ptr, HEX);
        if (*end_ptr != '\0')
            goto invalid_cmd;

        write_byte(addr, byte);
    } else if (input_buffer[0] == 'r') {
        if (input_ptr != 6)
            goto invalid_cmd;

        char addr_hex[5] = {};
        strncpy(addr_hex, input_buffer + 2, 4);

        char *end_ptr;
        uint16_t addr = strtol(addr_hex, &end_ptr, HEX);
        if (*end_ptr != '\0')
            goto invalid_cmd;

        uint8_t byte = read_byte(addr);
        Serial.println(byte, HEX);
    } else if (input_buffer[0] == 'R') {
        void (*reset)(void) = 0x0000;
        reset();
    } else if (input_buffer[0] == 'P') {
        char addr_hex[5] = {};
        strncpy(addr_hex, input_buffer + 2, 4);

        char *end_ptr;
        uint16_t addr = strtol(addr_hex, &end_ptr, HEX);
        if (*end_ptr != '\0')
            goto invalid_cmd;

        char OK[3] = {};
        Serial.readBytes(page_write_data, PAGE_SIZE);
        Serial.readBytes(OK, 2);
        if (strcmp("OK", OK) != 0) {
            goto invalid_cmd;
        }

        write_page(addr);
        verify_write_page(addr);
    } else if (input_buffer[0] == 'S') {
        self_test();
    } else
        goto invalid_cmd;

    return;

invalid_cmd:
    Serial.println("ERR: INVALID COMMAND. Use '?' for help.");
}

} // namespace AT28C256
