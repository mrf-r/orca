#include "orca.h"
#include "monochrome_graphic_lib.h"
#define lcd_framebuffer mgl_framebuffer

#include "NUC123.h"
#define SSD1306_I2C_ADDRESS 0x78

#define SSD1306_I2CMODE_SINGLE_CMD 0x80
#define SSD1306_I2CMODE_STREAM_CMD 0x00
#define SSD1306_I2CMODE_SINGLE_DAT 0xC0
#define SSD1306_I2CMODE_STREAM_DAT 0x40

static const uint8_t lcd_init_sequence[] = {
    0xAE, // SSD1306_DISPLAYOFF
    0xD5, 0x80, // SSD1306_SETDISPLAYCLOCKDIV 80
    0xA8, 0x3F, // SSD1306_SETMULTIPLEX 3F
    0xD3, 0x00, // SSD1306_SETDISPLAYOFFSET
    0x8D, 0x15, // SSD1306_CHARGEPUMP
    0x40, // SSD1306_SETSTARTLINE
    0xA6, // SSD1306_NORMALDISPLAY
    0xA4, // SSD1306_DISPLAYALLON_RESUME

    0xA1, // SSD1306_SEGREMAP (FLIP) A0
    0xC8, // SSD1306_COMSCANDEC C0
    // 0x20, 0x00, // SSD1306_MEMORYMODE

    0xDA, 0x12, // SSD1306_SETCOMPINS
    0x81, 0x3F, // SSD1306_SETCONTRAST
    0xD9, 0x77, // SSD1306_SETPRECHARGE F1
    0xDB, 0x20, // SSD1306_SETVCOMDETECT 40?
    0xAF // SSD1306_DISPLAYON
};

// static uint8_t lcd_mode;

// static uint8_t lcd_framebuffer[128 * 64 / 8];

uint32_t lcd_send(uint8_t* buf, uint32_t length, uint8_t lcd_mode)
{
    // start
    I2C0->I2CON = 0x68; // EN + STA + SI
    while (!(I2C0->I2CON & I2C_I2CON_SI))
        ; // wait
    // address + write
    if (I2C0->I2CSTATUS == 0x08)
        I2C0->I2CDAT = SSD1306_I2C_ADDRESS, I2C0->I2CON = 0x48;
    else
        goto err;
    while (!(I2C0->I2CON & I2C_I2CON_SI))
        ; // wait
    // data 1
    if (I2C0->I2CSTATUS == 0x18)
        I2C0->I2CDAT = lcd_mode, I2C0->I2CON = 0x48;
    else
        goto err;
    // data 2
    for (uint32_t i = 0; i < length; i++) {
        while (!(I2C0->I2CON & I2C_I2CON_SI))
            ; // wait
        if (I2C0->I2CSTATUS == 0x28)
            I2C0->I2CDAT = buf[i], I2C0->I2CON = 0x48;
        else
            goto err;
    }
    while (!(I2C0->I2CON & I2C_I2CON_SI))
        ; // wait
    // stop
    I2C0->I2CON = 0x58;
    return 0;
err:
    I2C0->I2CON = 0x58;
    return 1;
}

void lcd_update()
{
    uint8_t setadr[3] = { 0x80, 0x00, 0x10 };
    for (uint32_t i = 0; i < 8; i++) {
        setadr[0] = 0xB0 + i;
        // lcd_mode = SSD1306_I2CMODE_STREAM_CMD;
        lcd_send(setadr, 3, SSD1306_I2CMODE_STREAM_CMD);
        // lcd_mode = SSD1306_I2CMODE_STREAM_DAT;
        lcd_send(&lcd_framebuffer[128 * i], 128, SSD1306_I2CMODE_STREAM_DAT);
    }
}

void lcd_update_line(uint32_t sr)
{
    uint32_t line = (sr >> 8) & 0x7;
    uint8_t setadr[3] = { 0xB0 + (line & 0x7), 0x00, 0x10 };
    // lcd_mode = SSD1306_I2CMODE_STREAM_CMD;
    lcd_send(setadr, 3, SSD1306_I2CMODE_STREAM_CMD);
    // lcd_mode = SSD1306_I2CMODE_STREAM_DAT;
    lcd_send(&lcd_framebuffer[128 * line], 128, SSD1306_I2CMODE_STREAM_DAT);
}

void lcdUpdate()
{
    uint8_t setadr[4] = { SSD1306_I2CMODE_STREAM_CMD, 0xB0, 0x00, 0x10 };
    for (int i = 0; i < 8; i++) {
        setadr[0] = 0xB0 | i;
        i2cTransaction(SSD1306_I2C_ADDRESS, setadr, 3, 0, 0);
        i2cTransaction(SSD1306_I2C_ADDRESS, lcd_framebuffer, 3, 0, 0);
    }

    // lcd_mode = SSD1306_I2CMODE_STREAM_CMD;
    lcd_send(setadr, 3, SSD1306_I2CMODE_STREAM_CMD);
    lcd_send(setadr, 3, SSD1306_I2CMODE_STREAM_CMD);
    lcd_send(setadr, 3, SSD1306_I2CMODE_STREAM_CMD);
    // lcd_mode = SSD1306_I2CMODE_STREAM_DAT;
    lcd_send(&lcd_framebuffer[128 * line], 128, SSD1306_I2CMODE_STREAM_DAT);
    i2cTransaction(SSD1306_I2C_ADDRESS,
        se)
}

// pseudo-update
extern uint8_t buttons_state[10];
extern volatile int32_t adc_pitchwheel;
extern volatile int32_t adc_modwheel;
extern volatile uint16_t adc_knob[8];
extern volatile uint16_t adc_pad[16];

extern volatile uint32_t timeslot;
extern volatile uint32_t timeslot_max;

void lcd_scan_tick__(uint32_t sr)
// 1st pad draw scope
{
    if ((sr & 0xFF) == 0) {
        static const uint8_t bmp = 0x1;
        uint16_t pos = (sr >> 8) & 0x7F;
        mgl_setworkingarea(pos, 0, 1, MGL_DISPLAY_HEIGHT);
        mgl_fill(MGL_COLOR_LOW);
        mgl_setcursor(pos, 0x3FF - adc_pad[0]);
        mgl_drawbmp(&bmp, 1, 1);

        lcd_update_line(sr);
    }
}
void lcd_scan_tick_(uint32_t sr)
// all adc capture
{
    if ((sr & 0xFF) == 0) {
        /*
        // 1st row - buttons
        for (int i = 0; i < 10; i++) {
            lcd_framebuffer[i] = buttons_state[i];
        }
        // 2nd and 3rd rows - adc
        for (int i = 0; i < 8; i++) {
            lcd_framebuffer[i + 128] = adc_knob[i];
            lcd_framebuffer[i + 128 * 2] = adc_knob[i] >> 8;
        }
        for (int i = 0; i < 16; i++) {
            lcd_framebuffer[i + 128 + 16] = adc_pad[i];
            lcd_framebuffer[i + 128 * 2 + 16] = adc_pad[i] >> 8;
        }
        lcd_framebuffer[128 + 64] = adc_pitchwheel;
        lcd_framebuffer[128 * 2 + 64] = adc_pitchwheel >> 8;
        lcd_framebuffer[128 + 65] = adc_modwheel;
        lcd_framebuffer[128 * 2 + 65] = adc_modwheel >> 8;
        */
        mgl_fill(MGL_COLOR_LOW);
        for (int i = 0; i < 8; i++) {
            mgl_setcursor(64, 8 * i);
            if (adc_knob[i])
                mgl_hexvalue16(adc_knob[i]);
            mgl_setcursor(0, 8 * i);
            if (adc_pad[i])
                mgl_hexvalue16(adc_pad[i]);
            mgl_setcursor(32, 8 * i);
            if (adc_pad[i + 8])
                mgl_hexvalue16(adc_pad[i + 8]);
        }
        mgl_setcursor(96, 8);
        mgl_hexvalue32(adc_pitchwheel);
        mgl_setcursor(96, 16);
        mgl_hexvalue16(adc_pitchwheel);
        mgl_setcursor(96, 32);
        mgl_hexvalue32(adc_modwheel);
        mgl_setcursor(96, 40);
        mgl_hexvalue16(adc_modwheel);

        mgl_setcursor(96, 48);
        mgl_hexvalue16(timeslot);
        mgl_setcursor(96, 56);
        mgl_hexvalue16(timeslot_max--);

        lcd_update_line(sr);
    }
}

extern uint8_t buttons_state[10];

extern uint8_t lastkey;
extern uint8_t lastvelo;
extern uint16_t kbd_rawvelo;
extern uint16_t buttons;

void lcd_scan_tick()
// matrix scan
{
    mgl_fill(MGL_COLOR_LOW);

    for (int i = 0; i < 8; i++) {
        mgl_setcursor(0, 8 * i);
        if (buttons_state[i] != 0xFF)
            mgl_hexvalue16(buttons_state[i]);
    }
    mgl_setcursor(32, 0);
    if (buttons_state[8] != 0xFF)
        mgl_hexvalue16(buttons_state[8]);
    mgl_setcursor(32, 8);
    if (buttons_state[9] != 0xFF)
        mgl_hexvalue16(buttons_state[9]);

    mgl_setcursor(32, 16);
    mgl_hexvalue16(lastkey);
    mgl_setcursor(32, 24);
    mgl_hexvalue16(lastvelo);
    mgl_setcursor(32, 32);
    mgl_hexvalue16(kbd_rawvelo);
    mgl_setcursor(32, 40);
    mgl_hexvalue16(buttons);

    mgl_setcursor(96, 48);
    mgl_hexvalue16(timeslot);
    mgl_setcursor(96, 56);
    mgl_hexvalue16(timeslot_max--);
    lcd_update_line(sr);
}

void lcd_start()
{

    // init i2c interface
    // I2C0->I2CLK = 17; // 17 - set to 1 MHz or 44 - to 400kHz, 89 for something slow
    // I2C0->I2CON = I2C_I2CON_ENS1_Msk;
    //
    // lcd_mode = SSD1306_I2CMODE_STREAM_CMD;
    lcd_send((uint8_t*)lcd_init_sequence, sizeof(lcd_init_sequence), SSD1306_I2CMODE_STREAM_CMD);

    /*
    // fill framebuffer
    // clear
    for (uint32_t i = 0; i < sizeof(lcd_framebuffer); i++)
        lcd_framebuffer[i] = 0;
    //
    for (uint32_t x = 0; x < 128; x++) {
        lcd_framebuffer[0 + x] = 0x05;
        lcd_framebuffer[128 * 7 + x] = 0xA0;
    }
    for (uint32_t y = 1; y < 7; y++) {
        lcd_framebuffer[128 * y] = 0xFF;
        lcd_framebuffer[128 * y + 127] = 0xFF;
        lcd_framebuffer[128 * y + 2] = 0xFF;
        lcd_framebuffer[128 * y + 125] = 0xFF;
    }
    */
    mgl_init();

    // show image
    // lcd_update();
}
