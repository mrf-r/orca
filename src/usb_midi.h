#ifndef _USB_MIDI_H
#define _USB_MIDI_H

#include "NUC123.h"

// descriptors and protocol handler
#define USB_MIDI_EP_OUT 0x01
#define USB_MIDI_EP_IN 0x82
#define USB_MIDI_EP_OUT_MAXPACKETSIZE 64
#define USB_MIDI_EP_IN_MAXPACKETSIZE 64
#define USB_EP0_MAXPACKETSIZE 64
extern const S_USBD_INFO_T usb_descriptors;

void usbStart(void);
void usbMainTap(void);
uint32_t usbmidiOutSend(uint32_t message);
uint32_t usbmidiInReceive(uint32_t* message);
// void usbmidiEndpointsInit(void);

#if DEBUG
void usbDubegLoopback(void);
void usbmidiOutCC(uint16_t control);
#endif

#if IRQ_DISABLE == 0
void usbVirtInterrupt(void);
#endif

#endif // _USB_MIDI_H
