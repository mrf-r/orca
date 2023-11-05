#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <stdint.h>

#define KBD_KEYCOUNT 25

typedef enum {
    BUTTON_PEDAL = 0,
    BUTTON_MINUS,
    BUTTON_PLUS,
    BUTTON_MENU_TRANSPOSE,
    BUTTON_SHIFT,
    BUTTON_ARP,
    BUTTON_CHORD,
    BUTTON_PLAY,
    BUTTON_RECORD,
    BUTTON_PLAY_ALL,
    BUTTON_STOP_SOLO_MUTE,
    BUTTONS_TOTAL,
} ButtonsEn;

typedef enum {
    KEYFUNC_ARP_UP = 0,
    KEYFUNC_ARP_DWN,
    KEYFUNC_ARP_UPDWN,
    KEYFUNC_ARP_PLAYED,
    KEYFUNC_ARP_CHORD,
    KEYFUNC_ARP_U1,
    KEYFUNC_ARP_U2,
    KEYFUNC_RATE_4TH,
    KEYFUNC_RATE_8TH,
    KEYFUNC_RATE_16TH,
    KEYFUNC_RATE_32TH,
    KEYFUNC_RATE_U1,
    KEYFUNC_OCT_1,
    KEYFUNC_OCT_2,
    KEYFUNC_OCT_3,
    KEYFUNC_OCT_4,
    KEYFUNC_RHYTHM_O,
    KEYFUNC_RHYTHM_OXO,
    KEYFUNC_RHYTHM_OXXO,
    KEYFUNC_RHYTHM_U1,
    KEYFUNC_RHYTHM_U2,
    KEYFUNC_LATCH,
    KEYFUNC_UNDEF1,
    KEYFUNC_UNDEF2,
    KEYFUNC_UNDEF3,
    KEYFUNCS_TOTAL,
} KeyFuncEn;

uint8_t kbdOctaveGet(void);
void kbdOctaveSet(uint8_t octave);
void kbdInit(void);
void kbdSrTap(uint32_t sr);

// TODO: callbacks
// void keyPress(uint8_t key, uint8_t velocity);
// void keyRelease(uint8_t key, uint8_t velocity);
// void keyButton(uint8_t button, uint8_t press);
// void keyFunc(uint8_t func, uint8_t press);


#endif // _KEYBOARD_H
