
#include "NUC123.h"

void tim0_start_sr()
{
    //
    // TIMER0->TCSR = TIMER_TCSR_IE_Msk | (1 << TIMER_TCSR_MODE_Pos);
    TIMER0->TCSR = 2 << TIMER_TCSR_MODE_Pos;
    TIMER0->TCMPR = 750 - 1; // 750 - 96k 1500 - 48k
    TIMER0->TCSR |= TIMER_TCSR_CEN_Msk;
    // NVIC_SetPriority(TMR0_IRQn,2);
    // NVIC_EnableIRQ(TMR0_IRQn);
}
// unused, we can not remap table
void TMR0_IRQHandler()
{
    TIMER0->TCSR = TIMER_TISR_TIF_Msk;
    PA->DOUT |= 1 << 12;
    PA->DOUT &= ~(1 << 12);
}

//~ 21 uS
void wait_next_48k_tick()
{
    while (!(TIMER0->TISR & TIMER_TISR_TIF_Msk))
        ;
    TIMER0->TISR = TIMER_TISR_TIF_Msk;
}

void wait_one_sec()
{
    for (int i = 0; i < 96000; i++) {
        wait_next_48k_tick();
        PA->DOUT |= 1 << 12;
        PA->DOUT &= ~(1 << 12);
    }
}

int32_t timer_get_value()
{
    return TIMER0->TDR;
}