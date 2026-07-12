#include "W65C02S.h"
#include <Arduino.h>

namespace W65C02S {

bool setup() {
    Serial.println("--- W65C02S INIT BEGIN ---");

    Serial.println("--- W65C02S INIT OK    ---");
    return true;
}

void loop() {}

} // namespace W65C02S
