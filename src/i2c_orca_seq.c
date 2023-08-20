#include "orca.h"

// midi.h
#define I2C_MIDI_BUF_SIZE 64
static uint32_t midi_buffer[I2C_MIDI_BUF_SIZE];
static uint8_t midi_buffer_wp;
static uint8_t midi_buffer_rp;


uint16_t i2cMidiReceiveCallback(uint32_t message) {
	(void)message;
}


typedef struct {
    uint8_t status;
    const uint8_t address;
    const uint16_t len;
    const uint8_t* data;
} I2CTransaction;

static I2CTransaction 

void i2cSeqTap() {

}




