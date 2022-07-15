#ifndef _USB_MIDI_H
#define _USB_MIDI_H

#include "NUC123.h"
extern const S_USBD_INFO_T usb_descriptors;
void midi_endpoints_init(void);

#define USB_MIDI_EP_OUT 01
#define USB_MIDI_EP_IN 82
#define USB_MIDI_EP_OUT_MAXPACKETSIZE 64
#define USB_MIDI_EP_IN_MAXPACKETSIZE 64
#define USB_EP0_MAXPACKETSIZE 64


#endif // _USB_MIDI_H
