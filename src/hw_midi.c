#include "NUC123.h"
#include "orca.h"
#include "mbwmidi.h"

// #define MIDI_BUFFER_SIZE 128
// uint8_t m_buffer[MIDI_BUFFER_SIZE];
// uint8_t midi_buffer_rp = 0;
// uint8_t midi_buffer_wp = 0;

MidiOutPortContextT orca_uart_port;
static MidiOutUartContextT orca_uart_cx;

static void ouSendByte(uint8_t b)
{
    UART0->IER = UART_IER_THRE_IEN_Msk;
    UART0->THR = b;
}
static void ouStopSend()
{
    UART0->IER = 0;
}
static MidiRet ouIsBusy()
{
    return (UART0->IER & UART_IER_THRE_IEN_Msk) ? MIDI_RET_OK : MIDI_RET_FAIL;
}

MidiOutUartPortT orca_uart = {
    .port = &orca_uart_port,
    .context = &orca_uart_cx,
    .sendByte = ouSendByte,
    .stopSend = ouStopSend,
    .isBusy = ouIsBusy
};

void uartStart()
{
    midiOutUartInit(&orca_uart);

    UART0->LCR = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
    UART0->BAUD = 142;
    // UART0->IER = UART_IER_THRE_IEN_Msk;
    NVIC_SetPriority(UART0_IRQn, IRQ_PRIORITY_UART_MIDI);
#if IRQ_DISABLE == 0
    NVIC_EnableIRQ(UART0_IRQn);
#endif
}

// void uartTxIrqEnable()
// {
//     UART0->IER = UART_IER_THRE_IEN_Msk;
// }
// void uartTxIrqDisable()
// {
//     UART0->IER = 0;
// }
// void uartTxWrite(uint8_t b)
// {
//     UART0->THR = b;
// }

void UART0_IRQHandler()
{
    ASSERT(UART0->FSR & UART_FSR_TX_EMPTY_Msk);
    midiOutUartTranmissionCompleteCallback(&orca_uart);
    // if (midi_buffer_rp != midi_buffer_wp) {
    //     uartTxWrite(m_buffer[midi_buffer_rp]);
    //     midi_buffer_rp = (midi_buffer_rp + 1) & (MIDI_BUFFER_SIZE - 1);
    // } else {
    //     uartTxIrqDisable();
    // }
}

#if IRQ_DISABLE == 1
void uartVirtInterrupt()
{
    if (NVIC_GetPendingIRQ(UART0_IRQn)) {
        // if (UART0->FSR & UART_FSR_TX_EMPTY_Msk) {
        UART0_IRQHandler();
        NVIC_ClearPendingIRQ(UART0_IRQn);
    }
    // ASSERT(NVIC_GetPendingIRQ(UART0_IRQn) == 0);
}
#endif

// static inline void midi_write(uint8_t byte)
// {
//     m_buffer[midi_buffer_wp] = byte;
//     midi_buffer_wp = (midi_buffer_wp + 1) & (MIDI_BUFFER_SIZE - 1);
// }

// void cc_write(uint16_t cc)
// {
//     static uint16_t cc_old = 0;
//     if (cc_old != cc) {
//         cc_old = cc;
//         if (((midi_buffer_rp - midi_buffer_wp - 1) & (MIDI_BUFFER_SIZE - 1)) > MIDI_BUFFER_SIZE / 2) {
//             // midi_write(0x55);
//             // midi_write(0x00);
//             // return;
//             midi_write(0xb0);
//             midi_write(0x10);
//             midi_write((cc >> 7) & 0x7f);
//             midi_write(0x30);
//             midi_write(cc & 0x7f);
//             uartTxIrqEnable();
//         }
//     }
// }