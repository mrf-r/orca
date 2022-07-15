
#include "NUC123.h"
#include "orca.h"
#define BOOTLOADER_ENTER_KEY

#define TIM_DIVIDE 64 // 1.125MHz up to 14.9 Seconds @ 24bit
#define TIM_CLK_HZ (72000000 / TIM_DIVIDE)

static inline void delayOneMs()
{
    uint16_t tim_next = TIMER0->TDR + (TIM_CLK_HZ / 1000);
    while (tim_next < ((uint16_t)TIMER0->TDR))
        ;
    while (tim_next > ((uint16_t)TIMER0->TDR))
        ;
}

void delayMS(uint32_t ms)
{
    while (ms--) {
        delayOneMs();
    }
}

int main()
{
    __disable_irq();

    TIMER0->TCSR = (3 << TIMER_TCSR_MODE_Pos) | (TIM_DIVIDE - 1);
    TIMER0->TCSR |= TIMER_TCSR_CEN_Msk;

    // check checksum
    uint32_t firmware_checksum_error = 0;
    
    // check buttons
    uint32_t user_request = 0;

    if (firmware_checksum_error | user_request) {
        // start usb-midi "bootloader" with no exit
        while (firmware_checksum_error || user_request) {
        }

        // add delay 5 seconds for USB to reinit
        // deinit USB
    }

    // start main firmware
    SYS->REGWRPROT = 0x59;
    SYS->REGWRPROT = 0x16;
    SYS->REGWRPROT = 0x88;
    FMC->ISPCON &= ~FMC_ISPCON_BS_Msk;
    SYS->IPRSTC1 = SYS_IPRSTC1_CHIP_RST_Msk;
    while(1);
}