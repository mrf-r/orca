#include "usb_midi.h"
//#include <string.h>

#define USB_IRQ_DISABLE() __disable_irq()
#define USB_IRQ_ENABLE() __enable_irq()

void EP2_Handler(void);
void EP3_Handler(void);

/*--------------------------------------------------------------------------*/
void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

    //------------------------------------------------------------------
    if (u32IntSts & USBD_INTSTS_FLDET) {
        // Floating detect
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

        if (USBD_IS_ATTACHED()) {
            /* USB Plug In */
            USBD_ENABLE_USB();
            pr("enable\n");
        } else {
            /* USB Un-plug */
            USBD_DISABLE_USB();
            pr("disable\n");
        }
    }

    //------------------------------------------------------------------
    if (u32IntSts & USBD_INTSTS_WAKEUP) {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_WAKEUP);
        pr("wake -- \n");
    }

    //------------------------------------------------------------------
    if (u32IntSts & USBD_INTSTS_BUS) {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if (u32State & USBD_STATE_USBRST) {
            /* Bus reset */
            USBD_ENABLE_USB();
            USBD_SwReset();
            pr("bus reset\n");
        }
        if (u32State & USBD_STATE_SUSPEND) {

            /* Enable USB but disable PHY */
            USBD_DISABLE_PHY();
            pr("phy disable\n");
        }
        if (u32State & USBD_STATE_RESUME) {
            /* Enable USB and enable PHY */
            USBD_ENABLE_USB();
            pr("phy enable\n");
        }
    }

    //------------------------------------------------------------------
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
            pr("setup pack\n");
        }
        // EP events
        if (u32IntSts & USBD_INTSTS_EP0) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);

            // control IN
            USBD_CtrlIn();
            pr("ctrl in\n");
        }
        if (u32IntSts & USBD_INTSTS_EP1) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);

            // control OUT
            USBD_CtrlOut();
            pr("ctrl out\n");
        }
        if (u32IntSts & USBD_INTSTS_EP2) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
            // Bulk OUT
            EP2_Handler();
            pr("ep2\n");
        }

        if (u32IntSts & USBD_INTSTS_EP3) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            // Bulk IN
            EP3_Handler();
            pr("ep3\n");
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

void midi_endpoints_init(void)
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

    /*
    СЕЙЧАС ГЛАВНОЕ ЭТО ЮСБ потому что ты заебал

     + проверить ин аут кто есть кто я хз не помню
     + проверить соответствие кабеля и порта и нахер вообще кабельнумбер нужен
     сделать в первую очередь прием сообщений из хоста
     проверить блокировку, ограничивая размер ендпоинта
     проверить выдачу и надежность выдачи, потери в хосте
     проверить лупбэк с хоста какого-нибудь сисекса нивзъебенных размеров
     и его скорость конечнно-же
    */
}

#define USB_MIDI_IN_BUFFER_SIZE 32
uint32_t usb_midi_in_buffer[USB_MIDI_IN_BUFFER_SIZE];
uint32_t usb_midi_in_buffer_wp = 0;
uint32_t usb_midi_in_buffer_rp = 0;
uint32_t usb_midi_in_endpoint_unprocessed = 0;

#define USB_MIDI_OUT_BUFFER_SIZE 32
uint32_t usb_midi_out_buffer[USB_MIDI_OUT_BUFFER_SIZE];
uint32_t usb_midi_out_buffer_wp = 0;
uint32_t usb_midi_out_buffer_rp = 0;
uint32_t usb_midi_out_endpoint_payload = 0;

void usb_midi_cc_send(uint16_t control)
{
    if (((usb_midi_out_buffer_rp - usb_midi_out_buffer_wp - 1) & (USB_MIDI_OUT_BUFFER_SIZE - 1)) > USB_MIDI_OUT_BUFFER_SIZE / 2) {
        usb_midi_out_buffer[usb_midi_out_buffer_wp] = 0x00B01000 | (control >> 7);
        usb_midi_out_buffer_wp = (usb_midi_out_buffer_wp + 1) & (USB_MIDI_OUT_BUFFER_SIZE - 1);
        usb_midi_out_buffer[usb_midi_out_buffer_wp] = 0x00B03000 | (control & 0x7F);
        usb_midi_out_buffer_wp = (usb_midi_out_buffer_wp + 1) & (USB_MIDI_OUT_BUFFER_SIZE - 1);
    }
}

// 0x01 HOST OUT -> DEVICE IN
void EP2_Handler(void)
{
    usb_midi_in_endpoint_unprocessed = USBD_GET_PAYLOAD_LEN(2) / 4;
    if (((usb_midi_in_buffer_rp - usb_midi_in_buffer_wp - 1) & (USB_MIDI_IN_BUFFER_SIZE - 1)) > usb_midi_in_endpoint_unprocessed) {
        // read from endpoint to input buffer
        uint32_t* src = (uint32_t*)(USBD_BUF_BASE + EP2_BUF_BASE);
        for (int i = 0; i < usb_midi_in_endpoint_unprocessed; i++) {
            usb_midi_in_buffer[usb_midi_in_buffer_wp] = src[i];
            usb_midi_out_buffer_wp = (usb_midi_out_buffer_wp + 1) & (USB_MIDI_OUT_BUFFER_SIZE - 1);
        }
        // prepare for next data reception
        USBD_SET_PAYLOAD_LEN(2, USB_MIDI_EP_IN_MAXPACKETSIZE);
    } // else do nothing, keep ep ram untouched
}

// 0x82 DEVICE OUT -> HOST IN
void EP3_Handler(void)
{
    uint32_t* dst = (uint32_t*)(USBD_BUF_BASE + EP3_BUF_BASE);
    usb_midi_out_endpoint_payload = 0;
    while ((usb_midi_out_buffer_rp != usb_midi_out_buffer_wp)
        && (usb_midi_out_endpoint_payload < (USB_MIDI_EP_OUT_MAXPACKETSIZE / 4))) {
        *dst = usb_midi_out_buffer[usb_midi_out_buffer_rp];
        usb_midi_out_buffer_rp = (usb_midi_out_buffer_rp + 1) & (USB_MIDI_OUT_BUFFER_SIZE - 1);
        dst++;
        usb_midi_out_endpoint_payload += 4;
    }
    // validate EP
    if (usb_midi_out_endpoint_payload)
        USBD_SET_PAYLOAD_LEN(3, usb_midi_out_endpoint_payload);
}

void usb_midi_tap()
{
    if (NVIC_GetPendingIRQ(USBD_IRQn)) {
        NVIC_ClearPendingIRQ(USBD_IRQn);
        pr("+");
    }
    USBD_IRQHandler_();
    return;

    // in check
    if (usb_midi_in_endpoint_unprocessed) {
        USB_IRQ_DISABLE();
        if (((usb_midi_in_buffer_rp - usb_midi_in_buffer_wp - 1) & (USB_MIDI_IN_BUFFER_SIZE - 1)) > usb_midi_in_endpoint_unprocessed) {
            // read from endpoint to input buffer
            uint32_t* src = (uint32_t*)(USBD_BUF_BASE + EP2_BUF_BASE);
            for (int i = 0; i < usb_midi_in_endpoint_unprocessed; i++) {
                usb_midi_in_buffer[usb_midi_in_buffer_wp++] = src[i];
            }
            // prepare for next data reception
            USBD_SET_PAYLOAD_LEN(2, USB_MIDI_EP_IN_MAXPACKETSIZE);

            // validate ep
            usb_midi_in_endpoint_unprocessed = 0;
        }
        USB_IRQ_ENABLE();
    }
    // out check
    if (usb_midi_out_endpoint_payload == 0) {
        uint32_t* dst = (uint32_t*)(USBD_BUF_BASE + EP3_BUF_BASE);
        USB_IRQ_DISABLE();
        // load to ep
        while ((usb_midi_out_buffer_rp != usb_midi_out_buffer_wp)
            && (usb_midi_out_endpoint_payload < (USB_MIDI_EP_OUT_MAXPACKETSIZE / 4))) {
            *dst = usb_midi_out_buffer[usb_midi_out_buffer_rp];
            usb_midi_out_buffer_rp = (usb_midi_out_buffer_rp + 1) & (USB_MIDI_OUT_BUFFER_SIZE - 1);
            dst++;
            usb_midi_out_endpoint_payload += 4;
        }
        // validate EP
        if (usb_midi_out_endpoint_payload)
            USBD_SET_PAYLOAD_LEN(3, usb_midi_out_endpoint_payload);
        USB_IRQ_ENABLE();
    }
    // nothing??
}

uint32_t usb_midi_send(uint32_t message)
{
    uint32_t res = -1;
    USB_IRQ_DISABLE();
    if (((usb_midi_out_buffer_rp - usb_midi_out_buffer_wp - 1) & (USB_MIDI_OUT_BUFFER_SIZE - 1)) > 1) {
        usb_midi_out_buffer[usb_midi_out_buffer_wp] = message;
        usb_midi_out_buffer_wp = (usb_midi_out_buffer_wp + 1) & (USB_MIDI_OUT_BUFFER_SIZE - 1);
    }
    USB_IRQ_ENABLE();
    return res;
}
