#include <stdint.h>

#define IO_PORT (*(volatile uint8_t *)0x6000u)
#define IO_DDR  (*(volatile uint8_t *)0x6002u)

int main(void)
{
    IO_DDR = 0xffu;
    IO_PORT = 0x55u;

    // Halt
    for(;;);
}
