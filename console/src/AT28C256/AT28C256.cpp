#include "AT28C256.h"
#include "HardwareSerial.h"
#include "Print.h"
#include <Arduino.h>
#include <stdlib.h>
#include <time.h>

namespace AT28C256 
{

static bool g_OE, g_WE, g_IO;

inline void latchOE(bool state) 
{
    if(g_OE != state)
    {
        digitalWrite(PIN_OE, state);
        g_OE = state;
    }
}

inline void latchWE(bool state) 
{
    if(g_WE != state)
    {
        digitalWrite(PIN_WE, state);
        g_WE = state;
    }
}

inline void latchIO(bool state)
{
    if(g_IO != state)
    {
        g_IO = state;
        for(int i = 0; i < 8; i++)
        {
            pinMode(DATA_PINS[i], state);
        }
    }
}

uint8_t read_byte(uint16_t addr)
{
    // no writes, enable outputs
    latchWE(HIGH);
    latchOE(LOW);
    latchIO(INPUT);

    for(int i = 0; i <= 14; i++)
    {
        uint16_t mask = (1 << i);
        digitalWrite(ADDR_PINS[i], (addr & mask) != 0);
    }

    // enable output
    digitalWrite(PIN_OE, LOW);

    uint8_t byte = 0;
    for(int i = 0; i <= 7; i++)
    {
        uint8_t mask = (1 << i);
        bool active = digitalRead(DATA_PINS[i]);
        if (active) byte += mask;
    }
    
    return byte;
}

void write_byte(uint16_t addr, uint8_t byte)
{
    latchOE(HIGH);
    latchIO(OUTPUT);

    for(int i = 0; i <= 14; i++)
    {
        uint16_t mask = (1 << i);
        digitalWrite(ADDR_PINS[i], (addr & mask) != 0);
    }

    latchWE(LOW);

    for(int i = 0; i < 8; i++)
    {
        uint8_t mask = (1 << i);
        digitalWrite(DATA_PINS[i], (byte & mask) != 0);
    }
    
    latchWE(HIGH);

    delay(1);
}

bool self_test()
{
    srand(time(NULL));
    for(int i = 0; i < 32; i++)
    {
        uint16_t addr = rand() % EEPROM_SIZE;
        uint8_t old_byte = read_byte(addr);
        uint8_t new_byte = rand();
        write_byte(addr, new_byte);
        if(read_byte(addr) != new_byte) 
        {
            Serial.print("SELF TEST FAIL: 0x");
            Serial.print(addr, HEX);
            return false;
        }

        write_byte(addr, old_byte);
        if(read_byte(addr) != old_byte) 
        {
            Serial.print("SELF TEST FAIL: 0x");
            Serial.print(addr, HEX);
            return false;
        }
    }

    return true;
}

bool setup()
{
    Serial.println("--- AT28C256 INIT BEGIN ---");

    // Set CE, OE, and WE as outputs

    pinMode(PIN_CE, OUTPUT);
    pinMode(PIN_OE, OUTPUT);
    pinMode(PIN_WE, OUTPUT);

    // Set ADDR pins to output

    for(int i = 0; i < 15; i++)
    {
        pinMode(ADDR_PINS[i], OUTPUT);
    }

    // enable the chip
    digitalWrite(PIN_CE, LOW);

    self_test();

    Serial.println("--- AT28C256 INIT OK    ---");
    return true;
}

char input_buffer[64];
uint8_t input_ptr = 0;

void loop()
{
    Serial.print("\n>");
    input_ptr = 0;
    memset(input_buffer, 0, 64);

    // get command

    while(true) 
    {
        while(!Serial.available());
        char cur = Serial.read();

        if(cur == '\b')
        {
            if(input_ptr != 0) 
            {
                input_ptr--;
                input_buffer[input_ptr] = 0;
            }
            continue;
        }

        if(cur == '\n') break;

        input_buffer[input_ptr++] = cur;

        if(input_ptr == 64)
        {
            input_ptr--;
            Serial.print('\b');
        }

        Serial.print(cur);
    }

    // process command

    if(!strcmp(input_buffer, "?"))
    {
        // help
        Serial.print("HELP         '?'");

        Serial.print("WRITE BYTE   'w <addr> <byte>'");
        Serial.print("             'w FFFC A5' -> 'OK");

        Serial.print("READ BYTE    'r <addr>'");
        Serial.print("             'r FFFC' -> 'A5'");
    }
    else if(!strcmp(input_buffer, "w"))
    {
        // write byte

        if(input_ptr != 9) goto invalid_cmd;

        char addr_hex[5], byte_hex[3];
        strncpy(addr_hex, input_buffer + 2, 4);
        strncpy(byte_hex, input_buffer + 7, 2);

        char* end_ptr;
        uint16_t addr = strtol(addr_hex, &end_ptr, HEX);
        if(*end_ptr != '\0') goto invalid_cmd;

        uint16_t byte = strtol(byte_hex, &end_ptr, HEX);
        if(*end_ptr != '\0') goto invalid_cmd;

        write_byte(addr, byte);
        Serial.print("OK");
    }
    else if(!strcmp(input_buffer, "r"))
    {
        if(input_ptr != 6) goto invalid_cmd;

        char addr_hex[5];
        strncpy(addr_hex, input_buffer + 2, 4);

        char* end_ptr;
        uint16_t addr = strtol(addr_hex, &end_ptr, HEX);
        if(*end_ptr != '\0') goto invalid_cmd;

        uint8_t byte = read_byte(addr);
        Serial.print(byte, HEX);
    }
    else goto invalid_cmd;

    return;

invalid_cmd: 
    Serial.print("INVALID COMMAND\nUse '?' for help.");
}

}