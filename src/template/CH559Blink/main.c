#include <stdint.h>
#include "ch559.h"

static inline void delay()
{
    uint32_t i;
    for (i = 0; i < (120000UL); i++)
    {
        __asm__("nop");
    }
}

void main()
{
    PORT_CFG = 0b00101101;
    P1_DIR = 0b11110000;
    P1 = 0x00;

    AIN4 = 0; // P1_4
    AIN5 = 1; // P1_5
    AIN6 = 0; // P1_6
    AIN7 = 1; // P1_7

    while (1)
    {
        delay();
        AIN4 = !AIN4;
        AIN5 = !AIN5;

        delay();
        AIN4 = !AIN4;
        AIN5 = !AIN5;
        AIN6 = !AIN6;
        AIN7 = !AIN7;
    }
}