
#include "NUC123.h"
#include "system_dbgout.h"
#include "usb_midi.h"

#include "orca.h"
#include "keyboard.h"

volatile uint32_t timeslot;
volatile uint32_t timeslot_max;

// basically all async MIDI processing
void criticalLoop()
{
    // DEBUG interrupts
    srVirtInterrupt();
    uint32_t delta = tim1GetDelta();
    ASSERT(delta > (72000000 / SAMPLE_RATE - 2));
    ASSERT(delta < (72000000 / SAMPLE_RATE * 2));
    uartVirtInterrupt();
    usbVirtInterrupt();
    i2cVirtInterrupt();

    // async midi application functionality
    usbMainTap();
    cc_write(adc_knob[0]);
    usbDubegLoopback();
}

static inline void mainLoop()
{
    criticalLoop();

    // TODO: should we process MIDI in CR?? (with ADC and probably something else)

    // totally async functionality
    ledIntensitySet(adc_knob[1] / 64);
    lcd_scan_tick();
    // uint32_t lp = counter_sr & 0x7;
    static uint32_t next_upd = 0;
    if (counter_sr >= next_upd) {
        static uint32_t led_toggle = 0;
        if (led_toggle) {
            ledSet(18, rgb2c(0, 0, 0));
            led_toggle = 0;
        } else {
            ledSet(18, rgb2c(255, 0, 0));
            led_toggle = 1;
        }
        next_upd += SAMPLE_RATE;
    }

    // timeslot = timerSrGetValue();
    // if (timeslot > timeslot_max)
    //     timeslot_max = timeslot;
    // ASSERT(!(TIMER0->TISR & TIMER_TISR_TIF_Msk));

    // uint32_t lp = counter_sr & 0x7;
    // ledSet(lp, hsv2c(adc_knob[lp] / 4, adc_modwheel / 32, adc_pitchwheel / 32));
    // timerSrWainNext();

    /*
    for(uint32_t i = 0; i< 8; i++)
    {
            ledSet(i, hsv2c(hue + (i<<5),255,16));
            ledSet(i+8, hsv2c(hue + (i<<5) + 16,255,16));
            ledSet(i+16, hsv2c(25*i,255,16));
    }
    */
}

int main(void)
{
    print_s(NEWLINE "+-------------------------------------------+");
    print_s(NEWLINE "|          NUC123 Orca mini25               |");
    print_s(NEWLINE "+-------------------------------------------+");

    tim1Init();
    timerSrStart();
    i2cStart();
    ledInit();
    adcInit();
    uartStart();
    kbdInit();
    usbStart();

    lcd_start();

    for (uint32_t i = 0; i < 26; i++) {
        // ledSet(i, rgb2c(8, 8, 8));
        ledSet(i, hsv2c(i * 8, 255, 64));
    }
    // ledSet(18, rgb2c(8, 0, 0));
    // ledSet(25, rgb2c(8, 0, 0));

    while (1) {
        mainLoop();
    }
}
