/******************************************************************************
 * @file     main.c
 * @brief
 *           Demonstrate how to implement a USB keyboard device.
 *           It supports to use GPIO to simulate key input.
 * @note
 * Copyright (C) 2014~2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

// #define printf(...)
#include "NUC123.h"
#include "usb_midi.h"
#include "system_dbgout.h"

/*--------------------------------------------------------------------------*/
// uint8_t volatile g_u8EP2Ready = 0;

int main(void)
{
    print_s(NEWLINE "+-----------------------------------------------------+");
    print_s(NEWLINE "|          NuMicro USB MIDI Sample Code               |");
    print_s(NEWLINE "+-----------------------------------------------------+");

    usbmidiStart();
    
    while (1) {

        usbmidiTap();
    }
}
