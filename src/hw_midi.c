#include "NUC123.h"
#include "orca.h"
#define MIDI_BUFFER_SIZE 128
uint8_t midi_buffer[MIDI_BUFFER_SIZE];
uint8_t midi_buffer_rp = 0;
uint8_t midi_buffer_wp = 0;

void midi_write(uint8_t byte)
{
    midi_buffer[midi_buffer_wp] = byte;
    midi_buffer_wp = (midi_buffer_wp + 1) & (MIDI_BUFFER_SIZE - 1);
}

void cc_write(uint16_t cc)
{
    static uint16_t cc_old = 0;
    if (cc_old != cc) {
        cc_old = cc;
        if (((midi_buffer_rp - midi_buffer_wp - 1) & (MIDI_BUFFER_SIZE - 1)) > MIDI_BUFFER_SIZE / 2) {
            // midi_write(0x55);
            // midi_write(0x00);
            // return;
            midi_write(0xb0);
            midi_write(0x10);
            midi_write((cc >> 7) & 0x7f);
            midi_write(0x30);
            midi_write(cc & 0x7f);
        }
    }
}

void midi_test_check()
{
    if (midi_buffer_rp != midi_buffer_wp) {
        if (UART0->FSR & UART_FSR_TX_EMPTY_Msk) {
            UART0->THR = midi_buffer[midi_buffer_rp];
            midi_buffer_rp = (midi_buffer_rp + 1) & (MIDI_BUFFER_SIZE - 1);
        }
    }
}

void midi_start()
{
    UART0->LCR = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
    UART0->BAUD = 142;
    /*
    UART0->IER = UART_IER_THRE_IEN_Msk;
    NVIC_SetPriority(UART0_IRQn, IRQ_PRIORITY_UART_MIDI);
    NVIC_EnableIRQ(UART0_IRQn);
    */
}

void uart_enable()
{
    UART0->IER = UART_IER_THRE_IEN_Msk;
}
void uart_disable()
{
    UART0->IER = 0;
}
void uart_write_byte(uint8_t b)
{
    UART0->THR = b;
}

void UART0_IRQHandler()
{
    ;
}