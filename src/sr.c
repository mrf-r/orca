
#include "NUC123.h"
#include "keyboard.h"
#include "orca.h"

#define SR_TIMER_PERIOD (72000000 / SAMPLE_RATE)

volatile uint32_t counter_sr;
static int32_t lcg;

void TMR0_IRQHandler()
{
    // clear IRQ
    TIMER0->TISR = TIMER_TISR_TIF_Msk;

    counter_sr++;
    lcg = lcg * 0x41C64E6D + 0x3039;
    adcSrTap(counter_sr, lcg);
    ledSrTap(counter_sr, lcg);
    kbdSrTap(counter_sr);
}

// #if SAMPLE_RATE == 96000
// // #define SR_TIMER_PERIOD 375
// #define SR_TIMER_PERIOD 750
// #elif SAMPLE_RATE == 48000
// #define SR_TIMER_PERIOD 1500
// #else
// #error "SAMPLE RATE CONFIG ERROR"
// #endif

void timerSrStart()
{
    //
    TIMER0->TCSR = TIMER_TCSR_IE_Msk | (1 << TIMER_TCSR_MODE_Pos);
    // TIMER0->TCSR = 1 << TIMER_TCSR_MODE_Pos;
    TIMER0->TCMPR = SR_TIMER_PERIOD - 1; // 750 - 96k 1500 - 48k
    TIMER0->TCSR |= TIMER_TCSR_CEN_Msk;
    NVIC_SetPriority(TMR0_IRQn, IRQ_PRIORITY_TIMER_SR);
#if IRQ_DISABLE == 0
    NVIC_EnableIRQ(TMR0_IRQn);
#endif
}

#if IRQ_DISABLE == 1
void srVirtInterrupt()
{
    // while (!(TIMER0->TISR & TIMER_TISR_TIF_Msk))
    //     ;
    // TIMER0->TISR = TIMER_TISR_TIF_Msk;
    // ASSERT(NVIC_GetPendingIRQ(TMR0_IRQn));
    if (NVIC_GetPendingIRQ(TMR0_IRQn)) {
        NVIC_ClearPendingIRQ(TMR0_IRQn);
        TMR0_IRQHandler();
    }
    // for some reason it fails
    // ASSERT(NVIC_GetPendingIRQ(TMR0_IRQn) == 0);
}
#endif

int32_t timerSrGetValue()
{
    return TIMER0->TDR;
}

void measurementTimerStart()
{
    TIMER1->TCSR = 3 << TIMER_TCSR_MODE_Pos;
    TIMER1->TCSR |= TIMER_TCSR_CEN_Msk;
}

// 24bits!!! from the last call
uint32_t measurementTimerDelta()
{
    static uint32_t prev;
    uint32_t current = TIMER1->TDR;
    uint32_t delta = (current - prev) & 0xFFFFFF;
    prev = current;
    return delta;
}