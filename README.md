# 6502 Code Monorepo

This is all the code I used to debug, test, and build my 6502 breadbaord computer. `./console` is a debugger for the 6502 cpu, while the actual OS on the EEPROM is in `./6502-os`.

# Memory Map

- 0000 to 3FFF is RAM (16K)
- 3FFF to 5FFF is unmapped
- 6000 to 600F is I/O
- 6010 to 7FFF is reserved
- 8000 to FFFF is EEPROM (32K)

# Clock speed
I'm using a 555 astable with a potentiometer to run this computer, clock speed can vary from 0.5hz to ~10khz in testing.