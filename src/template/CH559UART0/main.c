#include <stdio.h>
#include "ch559.h"

int UART0Receive()
{
    while (!RI)
        ;
    RI = 0;
    return SBUF;
}

int UART0Send(int c)
{
    while (!TI)
        ;
    TI = 0;
    SBUF = c & 0xFF;
    return c;
}

int getchar()
{
    return UART0Receive();
}

int putchar(int c)
{
    return UART0Send(c);
}

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
    unsigned long x;
    uint32_t baud = 115200;

    // Clock Setting
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    CLOCK_CFG &= ~MASK_SYS_CK_DIV;
    CLOCK_CFG |= 6;
    PLL_CFG = ((24 << 0) | (6 << 5)) & 255;
    SAFE_MOD = 0xFF;
    delay();

    // UART0 Setting
    PORT_CFG |= bP0_OC;
    P0_DIR |= bTXD_;
    P0_PU |= bTXD_ | bRXD_;
    PIN_FUNC |= bUART0_PIN_X;

    SM0 = 0;
    SM1 = 1;
    SM2 = 0;
    REN = 1;
    PCON |= SMOD;
    x = (((unsigned long)FREQ_SYS / 8) / baud + 1) / 2;

    TMOD = TMOD & ~bT1_GATE & ~bT1_CT & ~MASK_T1_MOD | bT1_M1;
    T2MOD = T2MOD | bTMR_CLK | bT1_CLK;
    TH1 = (256 - x) & 255;
    TR1 = 1;
    TI = 1;

    while (1)
    {
        printf("Test build=%s, baud=%ld\n", __TIME__, baud);
        delay();
    }
}