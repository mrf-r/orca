#include "SEGGER_RTT.h"
#include "NUC123.h"

extern unsigned int __data_start__;
extern unsigned int __data_end__;
extern unsigned int __data_init_rom__;
extern unsigned int __bss_start__;
extern unsigned int __bss_end__;

void SystemInit()
{
    // init some hw stuff
    SYS->REGWRPROT = 0x59;
    SYS->REGWRPROT = 0x16;
    SYS->REGWRPROT = 0x88;
    SYS->PORCR = 0x5AA5;

    // init clock
    // Enable XT1_OUT (PF.0) and XT1_IN (PF.1)
    SYS->GPF_MFP |= SYS_GPF_MFP_PF0_XT1_OUT | SYS_GPF_MFP_PF1_XT1_IN;

    // Enable Internal RC 22.1184 MHz clock
    CLK->PWRCON |= CLK_PWRCON_OSC22M_EN_Msk;
    // Waiting for Internal RC clock ready
    while (!(CLK->CLKSTATUS & CLK_CLKSTATUS_OSC22M_STB_Msk))
        ;
    // Switch HCLK clock source to Internal RC and HCLK source divide 1
    // Apply new Divider
    CLK->CLKDIV &= ~CLK_CLKDIV_HCLK_N_Msk;
    // Switch HCLK to HIRC
    CLK->CLKSEL0 |= CLK_CLKSEL0_HCLK_S_HIRC;
    // Enable external XTAL 12 MHz clock
    CLK->PWRCON |= CLK_PWRCON_XTL12M_EN_Msk;
    // Waiting for external XTAL clock ready
    while (!(CLK->CLKSTATUS & CLK_CLKSTATUS_XTL12M_STB_Msk))
        ;
    // Set core clock

    // disable PLL
    CLK->PLLCON = CLK_PLLCON_PD_Msk;
    // set PLL to 144 and div to 2
    // FOUT = FIN * NF / NR / NO
    CLK->PLLCON = CLK_PLLCON_PLL_SRC_HXT // SRC - 12 MHz XT
        | (0x0 << 14) // NO = 2
        | (1 << 9) // NR = 3
        | 34; // NF = 36
    // wait till stable
    while (!(CLK->CLKSTATUS & CLK_CLKSTATUS_PLL_STB_Msk))
        ;
    // set div from 144
    CLK->CLKDIV = 1 // HCLK = PLL / 2 = 72 MHz
        | (2 << 4) // USB = PLL / 3 = 48 MHz
        | (1 << 8) // UART = PLL / 2 = 72 MHz
        | (0 << 16); // ADC = XT / 1 = 12 MHz
    // switch HCLK to PLL, SysTick to HCLK/2
    CLK->CLKSEL0 = (CLK->CLKSEL0
                       & (~(CLK_CLKSEL0_HCLK_S_Msk | CLK_CLKSEL0_STCLK_S_Msk)))
        | CLK_CLKSEL0_HCLK_S_PLL | CLK_CLKSEL0_STCLK_S_HCLK_DIV2;
    //
    CLK->CLKSEL1 = 0x0DAAAAF3;
    CLK->CLKSEL2 = CLK_CLKSEL2_WWDT_S_HCLK_DIV2048
        | CLK_CLKSEL2_PWM23_EXT_HXT | CLK_CLKSEL2_PWM01_EXT_HXT
        | CLK_CLKSEL2_FRQDIV_S_HCLK | CLK_CLKSEL2_I2S_S_HXT;
    // switch off 22M IRC
    CLK->PWRCON &= ~CLK_PWRCON_OSC22M_EN_Msk;

    // Enable peripherals
    CLK->AHBCLK |= CLK_AHBCLK_ISP_EN_Msk | CLK_AHBCLK_PDMA_EN_Msk;
    CLK->APBCLK |= CLK_APBCLK_ADC_EN_Msk | CLK_APBCLK_USBD_EN_Msk
        | CLK_APBCLK_UART0_EN_Msk | CLK_APBCLK_SPI1_EN_Msk | CLK_APBCLK_PWM23_EN_Msk
        | CLK_APBCLK_I2C0_EN_Msk | CLK_APBCLK_TMR0_EN_Msk;

    // init gpio pins
    SYS->ALT_MFP = 0x40000000; // uart tx
    SYS->ALT_MFP1 = 0x0A3F0000; // i2c, adc[5:0]
    SYS->GPA_MFP = 0x00008000; // pwm3
    SYS->GPC_MFP = 0x00002020; // uart tx, mosi1
    SYS->GPD_MFP = 0x0000003F; // adc
    SYS->GPF_MFP = 0x0002000F; // i2c, keep xtal

    PA->PMD = 0xD5FFFFFF;
    PB->PMD = 0xCFC000FF;
    PC->PMD = 0xF555F555;
    PD->PMD = 0xFFFFF000;
    PD->OFFD = 0x003F0000;
    PF->PMD = 0x000000A1;

    /*
    // ISP enable
    FMC->ISPCON = FMC_ISPCON_CFGUEN_Msk | FMC_ISPCON_APUEN_Msk | FMC_ISPCON_ISPEN_Msk;
    // read
    FMC->ISPADR = 0x00300000;
    FMC->ISPDAT = 0xFFFFFFFF;
    FMC->ISPCMD = 0;
    FMC->ISPTRG = 1;
    while (FMC->ISPTRG & 1)
        ;
    if (0) //(FMC->ISPDAT != 0xFFFFFFBF)
    {
        // erase
        FMC->ISPCMD = 0x22;
        FMC->ISPTRG = 1;
        while (FMC->ISPTRG & 1)
            ;
        // write
        FMC->ISPDAT = 0xFFFFFFBF;
        FMC->ISPTRG = 1;
        while (FMC->ISPTRG & 1)
            ;
    }
    */
   
    // close access
    // SYS->REGWRPROT = 0x00;

    // c init memory
    unsigned int* ram;
    unsigned int* rom;

    ram = &__data_start__;
    rom = &__data_init_rom__;
    while (ram < &__data_end__) {
        *ram++ = *rom++;
    }

    ram = &__bss_start__;
    while (ram < &__bss_end__) {
        *ram++ = 0;
    }

    // SEGGER_RTT_Init();
    // SEGGER_RTT_Write(0, "rtt start\n", 10);
}
