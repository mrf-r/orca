#include <stdint.h>

/*
__attribute__((at(0x00300000)))
volatile const uint32_t user_conf[2] = {
        //0x80A00046, // WDT disable, BOR reset @ 2.7V, LDROM BOOT, Flash unlocked, dataflash 4kB @ 0x0001F000
        //0x0001F000 // dataflah address as is
        0xFFFFFFBF, // boot from ldrom
        0x0001F000 // dataflah address as is
};
// */

#include "NUC123.h"
//#include "clk.h"
#include "SEGGER_RTT.h"
#include "controls.h"
#include "orca.h"
#include "usb_midi.h"

volatile uint32_t sr_counter;
volatile uint32_t timeslot;
volatile uint32_t timeslot_max;

void mainloop()
{
    static uint32_t next_upd = 0;
    static uint32_t led_toggle = 0;
    // sync
    wait_next_48k_tick();
    // inc
    sr_counter++;
    adc_tick(sr_counter);
    lcd_scan_tick(sr_counter);

    controls_tick(sr_counter);
    cc_write(adc_knob[0]);
    //  update
    led_scan_tick(sr_counter);
    // glow
    // uint32_t lp = sr_counter & 0x7;
    if (sr_counter >= next_upd) {
        if (led_toggle) {
            led_set(18, rgb2c(0, 0, 0));
            led_toggle = 0;
        } else {
            led_set(18, rgb2c(8, 0, 0));
            led_toggle = 1;
        }
        next_upd += 48000;
    }
    midi_test_check();
    
    usb_midi_tap();


    timeslot = timer_get_value();
    if (timeslot > timeslot_max)
        timeslot_max = timeslot;


    // uint32_t lp = sr_counter & 0x7;
    // led_set(lp, hsv2c(adc_knob[lp] / 4, adc_modwheel / 32, adc_pitchwheel / 32));

    /*
    SOMETHING WRONG WITH ADC, PLEASE FIX IT
    */

    /*
    for(uint32_t i = 0; i< 8; i++)
    {
            led_set(i, hsv2c(hue + (i<<5),255,16));
            led_set(i+8, hsv2c(hue + (i<<5) + 16,255,16));
            led_set(i+16, hsv2c(25*i,255,16));
    }
    */
}

volatile uint32_t readcfg;

int main()
{
    // usbflasher_start();

    tim0_start_sr();
    lcd_start();
    led_init();
    adc_start();
    midi_start();

    // rtt in systeminit.c
    // SEGGER_RTT_Init();
    // SEGGER_RTT_Write(0, "hello!\n", 7);


    usbflasher_start();
    /*
    USBD_SET_SE0();
    for (int i = 0; i < 12000; i++)
        wait_next_48k_tick();

    // Open USB controller
    USBD_Open(&usb_descriptors, NULL, NULL);

    // Endpoint configuration
    midi_endpoints_init();
    // Start USB device
    USBD_Start();
    */


    // NVIC_EnableIRQ(USBD_IRQn);
    // NVIC_EnableIRQ(UART_IRQn);

    // uint8_t hue = sr_counter >> 10;
    // color_t c = hsv2c(hue,128,128);
    // led_set(0,c);
    //

    for (uint32_t i = 0; i < 26; i++) {
        // led_set(i, rgb2c(8, 8, 8));
        led_set(i, hsv2c(i * 8, 192, 4));
    }
    // led_set(18, rgb2c(8, 0, 0));
    // led_set(25, rgb2c(8, 0, 0));

    while (1)
        ;//mainloop();

    while (!(SYS->REGWRPROT & 0x1))
        ;

    CLK->PWRCON |= 0x40; //
    CLK->AHBCLK |= 0x40;

    /*
    FMC->ISPCON = 0xFF; // enable fucking everything

    FMC->ISPADR = 0x00300000;
    FMC->ISPDAT = 0xFFFFFFFF;
    FMC->ISPCMD = 0x22;
    FMC->ISPTRG = 1;
    */

    /*
    perfect bootloader:
    start
    clock init
    copy bootloader code to sram
    run usb in polling mode

    */

    // clk init
    // vtor init
    //  there is no VTOR in M0 core. switch to polling mode
    // usb start
    // volatile uint32_t counter = 0;
    while (1) {
        // poll USB interrupt request

        // counter ++;
    }
}
