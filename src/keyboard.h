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


uint8_t kbdOctaveGet(void);
void kbdOctaveSet(uint8_t octave);
void kbdInit(void);
void kbdTap(uint32_t sr);

#endif // _KEYBOARD_H
