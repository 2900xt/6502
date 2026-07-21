#include <types.h>

void _start(void) {

  volatile byte_t *PORTB = (volatile byte_t *)0x6000;
  volatile byte_t *DDRB = (volatile byte_t *)0x6002;
  *DDRB = 0xFF;
  *PORTB = 0xFF;
  // Halt
  for (;;)
    ;
}
