
#include "NUC123.h"
#include "system_dbgout.h"
#include "usb_midi.h"

#include "orca.h"
#include "keyboard.h"

volatile uint32_t sr_counter;
volatile uint32_t timeslot;
volatile uint32_t timeslot_max;

static inline void mainLoop()
{
    static uint32_t next_upd = 0;
    static uint32_t led_toggle = 0;
    // sync
    wait_next_48k_tick();
    // inc
    sr_counter++;
    adc_tick(sr_counter);
    lcd_scan_tick(sr_counter);

    // controls_tick(sr_counter);
    kbdTap(sr_counter);
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

    timeslot = timer_get_value();
    if (timeslot > timeslot_max)
        timeslot_max = timeslot;

    usbmidiTap();
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

int main(void)
{
    print_s(NEWLINE "+-------------------------------------------+");
    print_s(NEWLINE "|          NUC123 Orca mini25               |");
    print_s(NEWLINE "+-------------------------------------------+");

    tim0_start_sr();
    lcd_start();
    led_init();
    adc_start();
    midi_start();
    kbdInit();

    usbmidiStart();
    
    for (uint32_t i = 0; i < 26; i++) {
        // led_set(i, rgb2c(8, 8, 8));
        led_set(i, hsv2c(i * 8, 192, 4));
    }
    // led_set(18, rgb2c(8, 0, 0));
    // led_set(25, rgb2c(8, 0, 0));


    while (1) {
        mainLoop();
    }
}
