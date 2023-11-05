#include "NUC123.h"

#include "keyboard.h"
#include "orca.h"

#define TOTAL_ROWS 10

#define KBDSTATE_RELEASED 0x0
#define KBDSTATE_PRESSING 0x4
#define KBDSTATE_PRESSED 0x8
#define KBDSTATE_RELEASING 0xC

typedef struct
{
    uint8_t state;
    uint8_t note;
    uint16_t counter;
} key_state_t;

uint8_t contacts[TOTAL_ROWS];
static key_state_t kbd_state[KBD_KEYCOUNT];
static uint8_t kbd_octave; // 2nd octave
static uint8_t kbd_noteshift; // 2nd octave

uint8_t kbdOctaveGet()
{
    return kbd_octave;
}
void kbdOctaveSet(uint8_t octave)
{
    ASSERT(octave < 9);
    kbd_octave = octave;
    kbd_noteshift = kbd_octave * 12;
}
void kbdInit()
{
    kbdOctaveSet(2);
}

/*
one point hyperbola
only saturation can be set
minimal timer value for max velocity
KBD_REC_SCALE is fixed at 16002
to keep all possible velocity levels
*/

#define KBD_REC_SCALE (126 * 127)
#define KBD_MIN_VELO_TIMER (16)
#define KBD_START_VALUE (126 - KBD_MIN_VELO_TIMER)

#if DEBUG == 1
#include "system_dbgout.h"
uint8_t lastkey;
uint8_t lastvelo;
uint16_t kbd_rawvelo;
uint16_t buttons;
uint32_t keys_function;
uint16_t buttons_finc_bmp = (1 << BUTTON_SHIFT);
#endif

__attribute__((weak)) void keyPress(uint8_t key, uint8_t velocity)
{
    lastkey = key;
    lastvelo = velocity;
    print_s(NEWLINE "key prs: ");
    print_d8(key);
    print_s(" ");
    print_d8(velocity);
}
__attribute__((weak)) void keyRelease(uint8_t key, uint8_t velocity)
{
    lastkey = key;
    lastvelo = velocity;
    print_s(NEWLINE "key rel: ");
    print_d8(key);
    print_s(" ");
    print_d8(velocity);
}
__attribute__((weak)) void keyButton(uint8_t button, uint8_t press)
{
    ASSERT(button < 16);
    if (press) {
        buttons |= 1 << button;
        print_s(NEWLINE "but prs: ");
    } else {
        buttons &= ~(1 << button);
        print_s(NEWLINE "but rel: ");
    }
    print_d8(button);
}

__attribute__((weak)) void keyFunc(uint8_t func, uint8_t press)
{
    ASSERT(func < 25);
    if (press) {
        print_s(NEWLINE "but prs: ");
    } else {
        print_s(NEWLINE "but rel: ");
    }
    print_d8(func);
}
// TODO: unglobal

#define CODEBLOCK_kbdnoteon(key)                                    \
    {                                                               \
        if (buttons & buttons_finc_bmp) {                           \
            keys_function |= 1 << key;                              \
            keyFunc(key, 1);                                        \
        } else {                                                    \
            int32_t vrawv = KBD_REC_SCALE / kbd_state[key].counter; \
            uint8_t velocity = vrawv > 127 ? 127 : vrawv;           \
            keyPress(kbd_state[key].note, velocity);                \
        }                                                           \
    }

#define CODEBLOCK_kbdnoteoff(key)                                   \
    {                                                               \
        if (keys_function & (1 << key)) {                           \
            keys_function &= ~(1 << key);                           \
            keyFunc(key, 0);                                        \
        } else {                                                    \
            int32_t vrawv = KBD_REC_SCALE / kbd_state[key].counter; \
            uint8_t velocity = vrawv > 127 ? 127 : vrawv;           \
            keyRelease(kbd_state[key].note, velocity);              \
        }                                                           \
    }

// set state
static void ssfw(uint8_t key)
{ // wait
    (void)key;
}
static void ssfc(uint8_t key)
{ // count
    if (kbd_state[key].counter < 0xFFFF)
        kbd_state[key].counter++;
}
static void ssfps(uint8_t key)
{ // press start
    kbd_state[key].note = key + kbd_noteshift;
    kbd_state[key].counter = KBD_START_VALUE;
    kbd_state[key].state = KBDSTATE_PRESSING;
}
static void ssfpu(uint8_t key)
{ // press unstart
    kbd_state[key].state = KBDSTATE_RELEASED;
}
static void ssfpe(uint8_t key)
{ // press end
    kbd_state[key].state = KBDSTATE_PRESSED;
#if DEBUG == 1
    kbd_rawvelo = kbd_state[key].counter;
#endif
    CODEBLOCK_kbdnoteon(key);
}
static void ssfpp(uint8_t key)
{ // press phaseskip
    kbd_state[key].note = key + kbd_noteshift;
    kbd_state[key].counter = KBD_START_VALUE;
    kbd_state[key].state = KBDSTATE_PRESSED;
#if DEBUG == 1
    kbd_rawvelo = kbd_state[key].counter;
#endif
    CODEBLOCK_kbdnoteon(key);
}
static void ssfrs(uint8_t key)
{ // release start
    kbd_state[key].state = KBDSTATE_RELEASING;
    kbd_state[key].counter = KBD_START_VALUE;
}
static void ssfru(uint8_t key)
{ // release unstart
    kbd_state[key].state = KBDSTATE_PRESSED;
}
static void ssfre(uint8_t key)
{ // release end
    kbd_state[key].state = KBDSTATE_RELEASED;
#if DEBUG == 1
    kbd_rawvelo = kbd_state[key].counter;
#endif
    CODEBLOCK_kbdnoteoff(key);
}
static void ssfrr(uint8_t key)
{ // release phaseskip
    kbd_state[key].state = KBDSTATE_RELEASED;
    kbd_state[key].counter = KBD_START_VALUE;
#if DEBUG == 1
    kbd_rawvelo = kbd_state[key].counter;
#endif
    CODEBLOCK_kbdnoteoff(key);
}

/*
1 - no contact (released)
0 - contact (pressed)

BA 11->10 start counter
BA 10->00 note on evnt with velo, keep octave
BA 00->10 start counter
AA 10->11 note off event with velo

     ||                /\
     ||                ||
A >  || start counter  || noteoff
     ||                ||
     ||                ||
B >  || noteon         || start counter
     ||                ||
     \/                ||

*/

// #define KBDSTATE_RELEASED 0x0
// #define KBDSTATE_PRESSING 0x4
// #define KBDSTATE_PRESSED 0x8
// #define KBDSTATE_RELEASING 0xC

static void (*const ssf[16])(uint8_t) = {
    // state : button_second : button_first
    // released [11]
    ssfpp, // 00    00 - too fast press
    ssfw, //  00    01 - impossible
    ssfps, // 00    10 - start pressing
    ssfw, //  00    11 - o
    // pressing [10]
    ssfpe, // 01    00 - end pressing with noteon
    ssfc, //  01    01 - impossibility squared
    ssfc, //  01    10 - o
    ssfpu, // 01    11 - back to released, change to released
    // pressed [00]
    ssfw, //  10    00 - o
    ssfw, //  10    01 - impossible
    ssfrs, // 10    10 - start releasing
    ssfrr, // 10    11 - too fast release
    // releasing [01]
    ssfru, // 11    00 - back to press state, change to pressed
    ssfc, //  11    01 - impossible
    ssfc, //  11    10 - o
    ssfre, // 11    11 - end releasing
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const uint32_t rows[16] = {
    0x0000FFFD, // 01
    0x0000FFFB, // 02
    0x0000FFF7, // 03
    0x0000FFEF, // 04
    0x0000FEFF, // 08
    0x0000FDFF, // 09
    0x0000FBFF, // 10
    0x0000F7FF, // 11
    0x0000EFFF, // 12
    0x0000FFFF, // now calm
    0x0000FFFF,
    0x0000FFFF,
    0x0000FFFF,
    0x0000FFFF,
    0x0000FFFF,
    0x0000FFFE // 00 - next will be start
};

static inline void hkKeyProc(const uint8_t key)
{
    // custom
    uint8_t pos = (key / 8) * 2;
    uint8_t spos = key & 7;
    uint8_t a = (contacts[pos] >> spos) & 1;
    uint8_t b = (contacts[pos + 1] >> spos) & 1;
    // common
    uint8_t act = kbd_state[key].state | (b << 1) | a;
    ASSERT(act < 16);
    ssf[act](key);
}
/*
static inline uint32_t hkRowRead(const uint pos)
{
    uint32_t row = PB->PIN >> 4;
    row = (row & 0x7F) | ((row & 0x400) >> 3);
    PC->DOUT = rows[pos];
}

#define HK_ROW_MID(NUM)               \
    static void hkRow##NUM()          \
    {                                 \
        hkRowRead(NUM);               \
        hkKeyProc((NUM - 2) * 2);     \
        hkKeyProc((NUM - 2) * 2 + 1); \
    }

static void hkRow0()
{
    hkRowRead(0);
}
static void hkRow1()
{
    hkRowRead(1);
}
HK_ROW_MID(2)
HK_ROW_MID(3)
HK_ROW_MID(4)
HK_ROW_MID(5)
HK_ROW_MID(6)
HK_ROW_MID(7)
static void hkRow8()
{
    hkRowRead(8);
    hkKeyProc(25);
}

static void (*const hkrr[16])() = {
    hkRow0, // R 0..7    P BUT 4..7
    hkRow1, // R 0..7    P BUT 8..10
    hkRow2, // R 8..15   P 0 1
    hkRow3, // R 8..15   P 2 3
    hkRow4, // R 16..23  P 4 5
    hkRow5, // R 16..23  P 6 7
    hkRow6, // R 24      P 8 9
    hkRow7, // R 24      P 10 11
    hkRow8, // R but     P 12 13
    hkRow9, // R but     P 14 15
    hkRow10, // R -      P 16 17
    hkRow11, // R -      P 18 19
    hkRow12, // R -      P 20 21
    hkRow13, // R -      P 22 23
    hkRow14, // R -      P 24
    hkRow15, // R -      P BUT 0..3
}
*/
void kbdSrTap(uint32_t sr)
{
    uint32_t pos = sr & 0xF;
    // read current
    uint32_t row = PB->PIN >> 4;
    // switch to next
    PC->DOUT = rows[pos];

    // process
    if (pos < TOTAL_ROWS) {
        row = (row & 0x7F) | ((row & 0x400) >> 3);
        contacts[pos] = row;
    }

    // A and B sensors are in different rows
    // we need to handle dedicated key outside that rows
    // let's just shift everything by 2 steps:
    // 1 - read key A sensor
    // 2 - read key B sensor
    // 3 - analyze key A and B sensors (and read next key's A)
    if ((pos >= 2) && (pos < 14)) {
        hkKeyProc((pos - 2) * 2);
        hkKeyProc((pos - 2) * 2 + 1);
    } else if (pos == 14) {
        hkKeyProc(24);
    } else if (pos == 0) {
        // buttons
        static uint16_t state_prev = 0;
        uint16_t state = ~(contacts[8] | (contacts[9] << 8));
        uint16_t change = state ^ state_prev;
        state_prev = state;
        for (int i = 0; i < BUTTONS_TOTAL; i++) {
            if (change & (1 << i)) {
                keyButton(i, (state >> i) & 1);
            }
        }
    }
}
