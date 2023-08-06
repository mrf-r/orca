#include "orca.h"
#include "usb_midi.h"
#include "system_dbgout.h"

/*
TODO:
- connection status, empty buffer on disconnect
- host read timeout, empty on inactive receiver
- head tail pointers instead of positions???
*/

#define USB_IRQ_DISABLE() __disable_irq()
#define USB_IRQ_ENABLE() __enable_irq()

volatile unsigned counter_usbd_irq = 0;
volatile unsigned counter_usbd_received = 0;
volatile unsigned counter_usbd_transmitted = 0;

#define SETUP_BUF_BASE 0
#define SETUP_BUF_LEN 8
#define EP0_BUF_BASE (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP0_BUF_LEN USB_EP0_MAXPACKETSIZE
#define EP1_BUF_BASE (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP1_BUF_LEN USB_EP0_MAXPACKETSIZE
#define EP2_BUF_BASE (EP1_BUF_BASE + EP1_BUF_LEN)
#define EP2_BUF_LEN USB_MIDI_EP_OUT_MAXPACKETSIZE
#define EP3_BUF_BASE (EP2_BUF_BASE + EP2_BUF_LEN)
#define EP3_BUF_LEN USB_MIDI_EP_IN_MAXPACKETSIZE

void usbmidiEndpointsInit(void)
{
    /* Init setup packet buffer */
    /* Buffer for setup packet -> [0 ~ 0x7] */
    USBD->STBUFSEG = SETUP_BUF_BASE;

    /*****************************************************/
    /* EP0 ==> control IN endpoint, address 0 */
    USBD_CONFIG_EP(EP0, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    /* Buffer range for EP0 */
    USBD_SET_EP_BUF_ADDR(EP0, EP0_BUF_BASE);

    /* EP1 ==> control OUT endpoint, address 0 */
    USBD_CONFIG_EP(EP1, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    /* Buffer range for EP1 */
    USBD_SET_EP_BUF_ADDR(EP1, EP1_BUF_BASE);

    /*****************************************************/
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_OUT | (USB_MIDI_EP_OUT & 0x7));
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);
    USBD_SET_PAYLOAD_LEN(EP2, USB_MIDI_EP_OUT_MAXPACKETSIZE);

    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_IN | (USB_MIDI_EP_IN & 0x7));
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
    USBD_SET_PAYLOAD_LEN(EP3, USB_MIDI_EP_IN_MAXPACKETSIZE);
}

#define UMI_BUF_SIZE 32
static uint32_t umi_buf[UMI_BUF_SIZE];
static uint32_t umi_wp = 0;
static uint32_t umi_rp = 0;
static uint32_t umi_ep_received = 0;

#define UMO_BUF_SIZE 32
static uint32_t umo_buf[UMO_BUF_SIZE];
static uint32_t umo_wp = 0;
static uint32_t umo_rp = 0;
static uint32_t umo_ep_sent = 0;

// check available buffer space and copy endpoint data or do nothing (wait)
static void usbmidiCopyInFromEp()
{
    if (((umi_rp - umi_wp - 1) & (UMI_BUF_SIZE - 1)) > umi_ep_received) {
        // read from endpoint to input buffer
        uint32_t* src = (uint32_t*)(USBD_BUF_BASE + EP2_BUF_BASE);
        for (int i = 0; i < umi_ep_received; i++) {
            // umi_buf[umi_wp] = src[i];
            USBD_MemCopy((uint8_t*)&umi_buf[umi_wp], (uint8_t*)&src[i], 4);
            umi_wp = (umi_wp + 1) & (UMI_BUF_SIZE - 1);
            counter_usbd_received++;
        }
        // prepare for next data reception
        umi_ep_received = 0;
        USBD_SET_PAYLOAD_LEN(2, USB_MIDI_EP_IN_MAXPACKETSIZE);
    } // else do nothing, keep ep ram untouched
}

static void usbmidiCopyOutToEp()
{
    uint32_t* dst = (uint32_t*)(USBD_BUF_BASE + EP3_BUF_BASE);
    umo_ep_sent = 0;
    while ((umo_rp != umo_wp)
        && (umo_ep_sent < (USB_MIDI_EP_OUT_MAXPACKETSIZE / 4))) {
        // dst[umo_ep_sent] = umo_buf[umo_rp];
        USBD_MemCopy((uint8_t*)&dst[umo_ep_sent], (uint8_t*)&umo_buf[umo_rp], 4);
        umo_rp = (umo_rp + 1) & (UMO_BUF_SIZE - 1);
        umo_ep_sent++;
        counter_usbd_transmitted++;
    }
    // validate EP
    if (umo_ep_sent)
        USBD_SET_PAYLOAD_LEN(3, umo_ep_sent * 4);
}

void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

    // debug stuff
    if (!u32IntSts)
        return;
#warning "check irq count"
    // print_s("\n");
    // print_d32(counter_usbd_irq);
    // print_s(" int:");
    // print_d32(u32IntSts);
    // print_s(" bus:");
    // print_d32(u32State);
    // debug stuff end

    counter_usbd_irq++;

    if (u32IntSts & USBD_INTSTS_FLDET) {
        // Floating detect
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);
        if (USBD_IS_ATTACHED()) {
            /* USB Plug In */
            USBD_ENABLE_USB();
        } else {
            /* USB Un-plug */
            USBD_DISABLE_USB();
        }
    }
    if (u32IntSts & USBD_INTSTS_WAKEUP) {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_WAKEUP);
    }
    if (u32IntSts & USBD_INTSTS_BUS) {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if (u32State & USBD_STATE_USBRST) {
            /* Bus reset */
            USBD_ENABLE_USB();
            USBD_SwReset();
        }
        if (u32State & USBD_STATE_SUSPEND) {
            /* Enable USB but disable PHY */
            USBD_DISABLE_PHY();
        }
        if (u32State & USBD_STATE_RESUME) {
            /* Enable USB and enable PHY */
            USBD_ENABLE_USB();
        }
    }
    if (u32IntSts & USBD_INTSTS_USB) {
        // USB event
        if (u32IntSts & USBD_INTSTS_SETUP) {
            // Setup packet
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);
            /* Clear the data IN/OUT ready flag of control end-points */
            USBD_STOP_TRANSACTION(EP0);
            USBD_STOP_TRANSACTION(EP1);
            USBD_ProcessSetupPacket();
        }
        // EP events
        if (u32IntSts & USBD_INTSTS_EP0) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
            // control IN
            USBD_CtrlIn();
        }
        if (u32IntSts & USBD_INTSTS_EP1) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);
            // control OUT
            USBD_CtrlOut();
        }
        if (u32IntSts & USBD_INTSTS_EP2) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
            // 0x01 HOST OUT -> DEVICE IN
            // EP2_Handler();
            umi_ep_received = USBD_GET_PAYLOAD_LEN(2) / 4;
            usbmidiCopyInFromEp();
        }
        if (u32IntSts & USBD_INTSTS_EP3) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            // 0x82 DEVICE OUT -> HOST IN
            // previous packet has been sent
            usbmidiCopyOutToEp();
        }

        if (u32IntSts & USBD_INTSTS_EP4) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
        }
        if (u32IntSts & USBD_INTSTS_EP5) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP5);
        }
        if (u32IntSts & USBD_INTSTS_EP6) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP6);
        }
        if (u32IntSts & USBD_INTSTS_EP7) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP7);
        }
    }
}

// static void usbmidiClassRequest(void)
// {
//     /* Setup error, stall the device */
//     USBD_SetStall(EP0);
//     USBD_SetStall(EP1);
// }

uint32_t usbmidiOutGetFree()
{
    uint32_t ret = (umo_rp - umo_wp - 1) & (UMO_BUF_SIZE - 1);
    return ret;
}
uint32_t usbmidiOutSend(uint32_t message)
{
    uint32_t ret = 0;
    USB_IRQ_DISABLE();
    if (((umo_rp - umo_wp - 1) & (UMO_BUF_SIZE - 1)) > 1) {
        umo_buf[umo_wp] = message;
        umo_wp = (umo_wp + 1) & (UMO_BUF_SIZE - 1);
        ret = 1;
    }
    USB_IRQ_ENABLE();
    return ret;
}

uint32_t usbmidiInReceive(uint32_t* message)
{
    uint32_t ret = 0;
    if (umi_rp != umi_wp) {
        *message = umi_buf[umi_rp];
        umi_rp = (umi_rp + 1) & (UMI_BUF_SIZE - 1);
        ret = 1;
    }
    return ret;
}
void usbmidiOutCC(uint16_t control)
{
    USB_IRQ_DISABLE();
    if (((umo_rp - umo_wp - 1) & (UMO_BUF_SIZE - 1)) > UMO_BUF_SIZE / 2) {
        umo_buf[umo_wp] = 0x00B01000 | (control >> 7);
        umo_wp = (umo_wp + 1) & (UMO_BUF_SIZE - 1);
        umo_buf[umo_wp] = 0x00B03000 | (control & 0x7F);
        umo_wp = (umo_wp + 1) & (UMO_BUF_SIZE - 1);
    }
    USB_IRQ_ENABLE();
}

void usbVirtInterrupt()
{
    /*
    TODO
     + проверить соответствие кабеля и порта и нахер вообще кабельнумбер нужен
     проверить блокировку, ограничивая размер ендпоинта
     проверить лупбэк с хоста какого-нибудь сисекса нивзъебенных размеров
     и его скорость конечнно-же

    */
    // debug polling
    if (NVIC_GetPendingIRQ(USBD_IRQn))
        USBD_IRQHandler();
    ASSERT(NVIC_GetPendingIRQ(USBD_IRQn) == 0);
}

void usbDubegLoopback()
{
    // debug loopback - one message per iteration??
    static uint8_t not_empty = 0;
    static uint32_t m;
    do {
        if (not_empty) {
            if (usbmidiOutSend(m))
                not_empty = 0;
            else
                return;
        } else {
            not_empty = usbmidiInReceive(&m);
        }
    } while (not_empty);

    // copy_another:
    //     if (umi_rp != umi_wp) {
    //         if (usbmidiOutSend(umi_buf[umi_rp]) == 0) {
    //             umi_rp = (umi_rp + 1) & (UMI_BUF_SIZE - 1);
    //             goto copy_another;
    //         }
    //     }
}

void usbMainTap()
{
    USB_IRQ_DISABLE();
    if (umi_ep_received) {
        usbmidiCopyInFromEp();
    }
    if (umo_ep_sent == 0) {
        usbmidiCopyOutToEp();
    }
    USB_IRQ_ENABLE();
}

void usbStart()
{
    USBD_Open(&usb_descriptors, NULL, NULL);
    usbmidiEndpointsInit();
    USBD_Start();
    NVIC_SetPriority(USBD_IRQn, IRQ_PRIORITY_USB);
    // NVIC_EnableIRQ(USBD_IRQn);
}
