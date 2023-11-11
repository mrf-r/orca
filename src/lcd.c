#include "i2c_proc.h"
#include "orca.h"

#include "mgl.h"
// #include "mgl_font5_cut.h"
#define MGL_DISPLAY_HEIGHT DISPLAY_SIZE_Y
#define MGL_COLOR_LOW COLOR_OFF
#define mgl_setworkingarea mgsWorkingArea
#define mgl_fill mgdFill
#define mgl_setcursor mgsCursorAbs
// #define mgl_drawbmp(a,b,c) mgdBitmap(a,b,c)
#define mgl_hexvalue16(a) mgdHex16(a, COLOR_ON)
#define mgl_hexvalue32(a) mgdHex32(a, COLOR_ON)
// extern const MglFont font5;
#ifndef MGL_SINGLEDISPLAY
extern const MglDisplay mgl_display;
#endif

// #include "monochrome_graphic_lib.h"
// #define mgsFont(...)

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
// pseudo-update

// void lcd_scan_tick__(uint32_t sr)
// // 1st pad draw scope
// {
//     if ((sr & 0xFF) == 0) {
//         static const uint8_t bmp = 0x1;
//         uint16_t pos = (sr >> 8) & 0x7F;
//         mgl_setworkingarea(pos, 0, 1, MGL_DISPLAY_HEIGHT);
//         mgl_fill(MGL_COLOR_LOW);
//         mgl_setcursor(pos, 0x3FF - adc_pad[0]);
//         // mgl_drawbmp(&bmp, 1, 1);

//         // lcd_update_line(sr);
//     }
// }

void lcd_scan_tick_(uint32_t sr)
// all adc capture
{
    if (!(lcd_sync & 0x1))
        return;
    lcd_sync &= ~0x1;
    if ((sr & 0xFF) == 0) {
        /*
        // 1st row - buttons
        for (int i = 0; i < 10; i++) {
            lcd_framebuffer[i] = contacts[i];
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

        // lcd_update_line(sr);
    }
}

void lcd_scan_tick()
// matrix scan
{
    static uint8_t syncp = 0;
    if (syncp == lcd_sync)
        return;

    mgsWorkingArea(0, 0, DISPLAY_SIZE_X, DISPLAY_SIZE_Y);
    // // mgdFill(COLOR_OFF);
    for (int i = 0; i < 8; i++) {
        mgsCursorAbs(0, i * 8);
        mgdHex16(adc_knob[i], COLOR_ON);
        mgsCursorAbs(28, i * 8);
        mgdHex16(adc_pad[i], COLOR_ON);
        mgsCursorAbs(56, i * 8);
        mgdHex16(adc_pad[i+8], COLOR_ON);
    }
    mgsCursorAbs(84, 0);
    mgdHex16(adc_pitchwheel, COLOR_ON);
    mgsCursorAbs(84, 8);
    mgdHex16(adc_modwheel, COLOR_ON);
    mgsCursorAbs(84, 16);
    mgdHex16(timeslot, COLOR_ON);
    mgsCursorAbs(84, 24);
    mgdHex16(timeslot_max, COLOR_ON);
    mgsCursorAbs(84, 32);
    mgdHex16(lastkey, COLOR_ON);
    mgsCursorAbs(84, 40);
    mgdHex16(lastvelo, COLOR_ON);
    mgsCursorAbs(84, 48);
    mgdHex16(buttons, COLOR_ON);
    mgsCursorAbs(84, 56);
    mgdHex16(keys_function, COLOR_ON);
    // mgsCursorAbs(84, 48);
    // mgdString("Hello", COLOR_ON);

    // mgdHex16(adc_pad[syncp], COLOR_ON);
    // mgdHex16(adc_pad[syncp+8], COLOR_ON);

    //     mgl_fill(MGL_COLOR_LOW);

    //     for (int i = 0; i < 8; i++) {
    //         mgl_setcursor(0, 8 * i);
    //         if (contacts[i] != 0xFF)
    //             mgl_hexvalue16(contacts[i]);
    //     }
    //     mgl_setcursor(32, 0);
    //     if (contacts[8] != 0xFF)
    //         mgl_hexvalue16(contacts[8]);
    //     mgl_setcursor(32, 8);
    //     if (contacts[9] != 0xFF)
    //         mgl_hexvalue16(contacts[9]);

    //     mgl_setcursor(32, 16);
    //     mgl_hexvalue16(lastkey);
    //     mgl_setcursor(32, 24);
    //     mgl_hexvalue16(lastvelo);
    //     mgl_setcursor(32, 32);
    //     mgl_hexvalue16(kbd_rawvelo);
    //     mgl_setcursor(32, 40);
    //     mgl_hexvalue16(buttons);

    //     mgl_setcursor(96, 48);
    //     mgl_hexvalue16(timeslot);
    //     mgl_setcursor(96, 56);
    //     mgl_hexvalue16(timeslot_max);
    // timeslot_max--;
    // timeslot_max -= (timeslot_max >> 4) + 1;
    // lcd_update_line(sr);

    syncp = lcd_sync;
}
