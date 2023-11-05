
#include "NUC123.h"
#include "i2c_proc.h"
#include "keyboard.h"
#include "orca.h"
#include "system_dbgout.h"
#include "usb_midi.h"
#include "../obj/signature.h"

volatile uint32_t timeslot;
volatile uint32_t timeslot_max;

// basically all async MIDI processing
void criticalLoop()
{
#if IRQ_DISABLE
    srVirtInterrupt();
    uint32_t delta = measurementTimerDelta();
    // ASSERT(delta > (72000000 / SAMPLE_RATE - 2));
    // ASSERT(delta < (72000000 / SAMPLE_RATE * 2));
    uartVirtInterrupt();
    // usbVirtInterrupt();
    i2cVirtInterrupt();
#endif

    // async midi application functionality
    // TODO: main control loop is here
    i2cSeqTap();
    // usbMainTap();
    cc_write(adc_knob[0]);
    // usbDubegLoopback();
}

void delayMs(uint32_t ms)
{
    uint32_t end_sr = counter_sr + ms * 1000 / SAMPLE_RATE;
    while (counter_sr > end_sr) {
        criticalLoop();
    }
    while (counter_sr < end_sr) {
        criticalLoop();
    }
}

static inline void mainLoop()
{
    criticalLoop();

    // TODO: should we process MIDI in CR?? (with ADC and probably something else)

    // totally async functionality
    ledIntensitySet(adc_knob[1] / 64);

    void lcd_scan_tick(void); // TODO DEBUG
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
    print_s(NEWLINE "--<<< NUC123 Orca mini25 >>>--");
    print_s(NEWLINE GIT_DESCRIBE);
    print_s(NEWLINE ENV_DATE_LOCAL);
    print_s(NEWLINE ENV_TIME_LOCAL);
    // ASSERT(0); // test assert

    measurementTimerStart();
    timerSrStart();
    i2cStart(I2C_MIDI_OWN_ADDRESS);
    ledInit();
    adcInit();
    uartStart();
    kbdInit();
    // usbStart();
    lcdStart();

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
