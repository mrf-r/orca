#include "NUC123.h"

#include "usb_midi.h"

#define ALIGN4
//#define ALIGN4 __attribute__((aligned(4)))

//                      LSB,MSB
//#define USB_ID_VENDOR  0x75,0x1C //microbrute TODO: change?
//#define USB_ID_PRODUCT 0x02,0x04
#define USB_ID_VENDOR 0x75, 0x1C // pid.codes test pid 8
#define USB_ID_PRODUCT 0x04, 0x02
#define USB_BCD_DEVICE 0x00, 0x02 // kinda internal revision and can be anything? now it's like v01.00

ALIGN4 const uint8_t usbdesc_device[] = {

    // device descriptor
    0x12,
    0x01, // Device Descriptor
    0x00, 0x02, // bcdUSB
    0x00, // bDeviceClass
    0x00, // bDeviceSubClass
    0x01, // bDeviceProtocol
    USB_EP0_MAXPACKETSIZE, // bMaxPacketSize
    USB_ID_VENDOR, // idVendor
    USB_ID_PRODUCT, // idProduct
    USB_BCD_DEVICE, // bcdDevice
    0x01, // iManufacturer
    0x02, // iProduct
    0x00, // iSerialNumber
    0x01 // bNumConfigurations
};

ALIGN4 const uint8_t usbdesc_langid[4] = {
    0x04, 0x03, // String Descriptor
    0x09, 0x04 // wLANGID
};

ALIGN4 const uint8_t usbdesc_manufacturer[38] = {
    38, 3,
    'U', 0, 'R', 0, 'A', 0, 'L', 0, ' ', 0, 'S', 0, 'o', 0, 'u', 0,
    'n', 0, 'd', 0, ' ', 0, 'S', 0, 'o', 0, 'u', 0, 'r', 0, 'c', 0,
    'e', 0, 's', 0
};

ALIGN4 const uint8_t usbdesc_product[24] = {
    24, 3,
    'O', 0, 'r', 0, 'c', 0, 'a', 0, ' ', 0, 'm', 0, 'i', 0, 'n', 0,
    'i', 0, '2', 0, '5', 0
};

ALIGN4 const uint8_t usbdesc_conf[] = {
    // device qualifier not needed since we run at one speed only - FS
    // configuration descriptor
    0x09,
    0x02, // Configuration Descriptor
    0x65, 0x00, // wTotalLength                       !check this twice
    0x02, // bNumInterfaces
    0x01, // bConfigurationValue
    0x00, // iConfiguration
    0x80, // bmAttributes   (Self-powered Device) 0x80 - bus
    0xFA, // bMaxPower   (500 mA)

    0x09,
    0x04, // Interface Descriptor 0
    0x00, // bInterfaceNumber
    0x00, // bAlternateSetting
    0x00, // bNumEndPoints
    0x01, // bInterfaceClass   (Audio Device Class)
    0x01, // bInterfaceSubClass   (Audio Control Interface)
    0x00, // bInterfaceProtocol
    0x00, // iInterface   (product)

    0x09,
    0x24, // AC Interface Header Descriptor
    0x01,
    0x00, 0x01, // bcdADC
    0x09, 0x00, // wTotalLength
    0x01, // bInCollection
    0x01, // baInterfaceNr(1)

    0x09,
    0x04, // Interface Descriptor 0
    0x01, // bInterfaceNumber
    0x00, // bAlternateSetting
    0x02, // bNumEndPoints
    0x01, // bInterfaceClass   (Audio Device Class)
    0x03, // bInterfaceSubClass   (MIDI Streaming Interface)
    0x00, // bInterfaceProtocol
    0x00, // iInterface   (product)

    0x07,
    0x24, // MS Interface Header Descriptor:
    0x01, // bDescriptorSubtype
    0x00, 0x01, // bcdADC
    0x41, 0x00, // wTotalLength

    0x06,
    0x24, // MS MIDI IN Jack Descriptor:
    0x02, // bDescriptorSubtype
    0x01, // bJackType
    0x01, // bJackID
    0x00, // iJack

    0x06,
    0x24, // MS MIDI IN Jack Descriptor:
    0x02, // bDescriptorSubtype
    0x02, // bJackType
    0x02, // bJackID
    0x00, // iJack

    0x09,
    0x24, // MS MIDI OUT Jack Descriptor:
    0x03, // bDescriptorSubtype
    0x01, // bJackType
    0x03, // bJackID
    0x01, // bNrInputPins
    0x02, // baSourceID(1)
    0x01, // baSourcePin(1)
    0x00, // iJack

    0x09,
    0x24, // MS MIDI OUT Jack Descriptor:
    0x03, // bDescriptorSubtype
    0x02, // bJackType
    0x04, // bJackID
    0x01, // bNrInputPins
    0x01, // baSourceID(1)
    0x01, // baSourcePin(1)
    0x00, // iJack

    0x09,
    0x05, // Endpoint Descriptor (Audio/MIDI):
    USB_MIDI_EP_OUT, // bEndpointAddress   (OUT Endpoint)
    0x02, // bmAttributes	(Transfer: Bulk / Synch: None / Usage: Data)
    USB_MIDI_EP_OUT_MAXPACKETSIZE, 0x00, // wMaxPacketSize   (64 Bytes)
    0x00, // bInterval
    0x00, // bRefresh
    0x00, // bSynchAddress

    0x05,
    0x25, // MS Bulk Data Endpoint Descriptor:
    0x01, // bDescriptorSubtype
    0x01, // bNumEmbMIDIJack
    0x01, // baAssocJackID

    0x09,
    0x05, // Endpoint Descriptor (Audio/MIDI):
    USB_MIDI_EP_IN, // bEndpointAddress   (IN Endpoint)
    0x02, // bmAttributes	(Transfer: Bulk / Synch: None / Usage: Data)
    USB_MIDI_EP_IN_MAXPACKETSIZE, 0x00, // wMaxPacketSize   (64 Bytes)
    0x00, // bInterval
    0x00, // bRefresh
    0x00, // bSynchAddress

    0x05,
    0x25, // MS Bulk Data Endpoint Descriptor:
    0x01, // bDescriptorSubtype
    0x01, // bNumEmbMIDIJack
    0x03, // baAssocJackID
};

const uint8_t* usbdesc_str[4] = {
    usbdesc_langid,
    usbdesc_manufacturer,
    usbdesc_product
};

const S_USBD_INFO_T usb_descriptors = {
    usbdesc_device,
    usbdesc_conf,
    usbdesc_str,
    NULL
};
