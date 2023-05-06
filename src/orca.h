#ifndef ORCA_H_
#define ORCA_H_

#include <stdint.h>

// oversimplified assertion
#if DEBUG == 1
#define ASSERT(some)  \
    {                 \
        if (!(some))    \
            while (1) \
                ;     \
    }
#else
#define ASSERT(...)
#endif

/*
нужно инитить стек при загрузке в ОЗУ
нужно понять как там правильно грузить в флеш
автоматизировать файл


режим клавиатуры:
просто 16 клипов
на сколько там хватит памяти
в клип можно писать любую инфу, но по сути только сс и ноты
клипы по дефолту ваншотные, то есть просто пишем аккорд к примеру, а потом играем клипами-пэдами
также у клипов можно сделать несколько режимов - типа просто аккорд залаченый, и в группе, чтобы можно было класть аккорды

кароч нужно чето очень простое без менюшек типа как сёркит и на 16 миди каналов.
для каждого из 16 миди каналов крутилки свои сс имеют, также свои настройки октавы

ну по-хорошему наверно все-таки лучше иметь режим сохранения всего этого говна типа 4 слота памяти

режим и2с контроллера
*/

/*


31 - PA15 - LED2 - PWM3
32 - PA14 - ADSEL2
33 - PA13 - ADSEL1
34 - PA12 - ADSEL0
04 - PA11 - ?
05 - PA10 - x

06 - PB4 - scanI 9    : ?     : rec  - pressed 0, released 1
07 - PB5 - scanI 10   : -     : >
08 - PB6 - scanI 13   : +     : stop
09 - PB7 - scanI 14   : menu  : ?
02 - PB8 - scanI 15   : shift : ?
24 - PB9 - scanI 16   : arp   : ?
23 - PB10 - scanI 12  : chord : ?
03 - PB14 - scanI 11  : play  : ?

25 - PC13 - SPI1_MOSI/PWM3 - WS2812B
26 - PC12 - scanO - CONTROL BUTTONS 2nd - running zero
27 - PC11 - scanO CONTROL BUTTONS 1st
28 - PC10 - scanO KB 4th 1 key 2nd contact
29 - PC9 - scanO KB 4th 1 key 1st contact
30 - PC8 - scanO KB 3rd 8 keys 2nd contact
17 - PC5 - UART0-TXD - MIDI out
18 - PC4 - scanO KB 3rd 8 keys 1st contact
19 - PC3 - scanO KB 2st 8 keys 2nd contact
20 - PC2 - scanO KB 2nd 8 keys 1st contact
21 - PC1 - scanO KB 1st 8 keys 2nd contact
22 - PC0 - scanO KB 1st 8 keys 1st contact

38 - PD0 - ADC0 - Pitch
39 - PD1 - ADC1 - Mod
40 - PD2 - ADC2 - top pads 1..8
41 - PD3 - ADC3 - pots 1..8
42 - PD4 - ADC4 - bottom pads 9..16
43 - PD5 - x

47 - PF2 - I2C0_SDA - display
48 - PF3 - I2C0_SCL - display

*/

#define IRQ_PRIORITY_TIMER_SR 0x0
#define IRQ_PRIORITY_UART_MIDI 0x1
#define IRQ_PRIORITY_USB 0x2
#define IRQ_PRIORITY_I2C 0x3

#define SAMPLE_RATE 96000

typedef struct
{
    uint8_t green;
    uint8_t red;
    uint8_t blue;
} color_t;

void led_set(uint32_t led, color_t color);
void led_init(void);
void led_scan_tick(uint32_t src);
color_t hsv2c(uint8_t hue, uint8_t saturation, uint8_t value);
color_t rgb2c(uint8_t red, uint8_t green, uint8_t blue);

// main timer
void tim0_start_sr(void);
void wait_next_48k_tick(void);
void wait_one_sec(void);
int32_t timer_get_value(void);

// display
void lcd_start(void);
void lcd_scan_tick(uint32_t sr);

// adc
void adc_start(void);
void adc_tick(uint32_t sr);
extern volatile uint16_t adc_pitchwheel;
extern volatile uint16_t adc_modwheel;
extern volatile uint16_t adc_knob[8];
extern volatile uint16_t adc_pad[16];

// keys
void controls_tick(uint32_t sr);

// midi
void cc_write(uint16_t cc);
void midi_test_check(void);
void midi_start(void);

// usb
void usbmidiTap(void);

#endif // ORCA_H_
