#ifndef _CONTROLS_H
#define _CONTROLS_H

#include "NUC123.h"

#define BUTTON_MINUS
#define BUTTON_PLUS
#define BUTTON_MENU_TRANSPOSE
#define BUTTON_SHIFT
#define BUTTON_ARP
#define BUTTON_CHORD
#define BUTTON_PLAY
#define BUTTON_RECORD
#define BUTTON_PLAY_ALL
#define BUTTON_STOP_SOLO_MUTE
#define BUTTON_PEDAL

#define TOTAL_ROWS 10

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
	0x0000FFFE  // 00 - next will be start
};

uint8_t buttons_state[10];

static inline void controls_tick(uint32_t sr) {
	uint32_t pos = sr & 0xF;
	// read current
	uint32_t row = PB->PIN >> 4;
	// switch to next
	PC->DOUT = rows[pos];
	
	//process
	if (pos < TOTAL_ROWS) {	
		row = (row & 0x7F) | ((row & 0x400) >> 3);
		buttons_state[pos] = row;
	}
}

#endif // _CONTROLS_H