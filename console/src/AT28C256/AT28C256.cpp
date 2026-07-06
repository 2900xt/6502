#include "AT28C256.h"
#include <Arduino.h>

namespace AT28C256 
{

static bool g_OE, g_WE;

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

uint8_t read_byte(uint16_t addr)
{
    // no writes, enable outputs
    latchWE(HIGH);
    latchOE(LOW);

    for(int i = 0; i <= 15; i++)
    {
        uint16_t mask = (1 << i);
        digitalWrite(ADDR_PINS[i], (addr & mask) != 0);
    }

    // set DATA pins to input
    for(int i = 0; i < 8; i++)
    {
        pinMode(DATA_PINS[i], INPUT);
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
    // set DATA pins to output and write byte onto pin

    for(int i = 0; i < 8; i++)
    {
        uint8_t mask = (1 << i);
        pinMode(DATA_PINS[i], OUTPUT);
        digitalWrite(DATA_PINS[i], (byte & mask) != 0);
    }
}

bool setup()
{
    Serial.println();
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

    Serial.println("--- AT28C256 INIT OK    ---");
    return true;
}

void loop()
{
    
}

}