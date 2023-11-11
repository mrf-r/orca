#include "orca.h"
#include "i2c_proc.h"

int i2cMasterTransaction(I2CTransaction* wtr, I2CTransaction* rtr);
__attribute__((weak)) void midiReceive(uint32_t timestamp, uint32_t message)
{
    // TODO
    (void)timestamp;
    (void)message;
}
__attribute__((weak)) uint32_t midiTransmitGetNext()
{
    // TODO
    return 0;
}

#define I2C_MIDI_BUFFER_SIZE 4

static I2CTransaction* midiTxCallback();
static I2CTransaction* midiRxCallback();

static I2CTransaction midi_transmit;
static I2CTransaction midi_receive;
static uint32_t tx_buffer[I2C_MIDI_BUFFER_SIZE];
static uint32_t rx_buffer[I2C_MIDI_BUFFER_SIZE];

#define MIDI_ACTIVE_SENSING_GEN_MS 270
#define MIDI_ACTIVE_SENSING_MESSAGE 0x0000FE0F
#define MIDI_ACTIVE_SENSING_DETACHED_MAX 8
#define MIDI_ACTIVE_SENSING_DETACHED_TRES 4
static uint8_t detached;
static uint32_t next_update;

static I2CTransaction* midiTxCallback()
{
    if (midi_transmit.status == ITS_RESULT_OK) {
        for (unsigned i = 0; i < I2C_MIDI_BUFFER_SIZE; i++) {
            tx_buffer[i] = midiTransmitGetNext();
        }
        if (detached)
            detached--;
    } else {
        next_update = counter_sr;
        if (detached < MIDI_ACTIVE_SENSING_DETACHED_MAX)
            detached++;
    }
    return 0;
}

static I2CTransaction* midiRxCallback()
{
    if (midi_receive.status == ITS_RESULT_OK) {
        for (unsigned i = 0; i < I2C_MIDI_BUFFER_SIZE; i++) {
            if (0 != rx_buffer[i]) {
                midiReceive(counter_sr, rx_buffer[i]);
            }
        }
        if (detached)
            detached--;
    } else {
        next_update = counter_sr;
        if (detached < MIDI_ACTIVE_SENSING_DETACHED_MAX)
            detached++;
    }
    return 0;
}

static I2CTransaction* midiActiveSenceCallback()
{
    if (midi_transmit.status == ITS_RESULT_OK) {
        if (detached < MIDI_ACTIVE_SENSING_DETACHED_MAX)
            detached++;
    } else {
        if (detached)
            detached--;
    }
    return 0;
}

// to be called when bus is empty
// returns true if transaction was initiated
bool i2cSeqMidi()
{
    // in case of connected device, start a transaction to occupy all remaining bandwidth
    // in case of NAK MIDI active sensing )
    if (detached >= MIDI_ACTIVE_SENSING_DETACHED_TRES) {
        if (((int32_t)next_update - (int32_t)counter_sr) < 0) {
            midi_transmit.status = 0;
            midi_transmit.address = I2C_MIDI_REMOTE_ADDRESS;
            midi_transmit.len = 4;
            midi_transmit.data = (const uint8_t*)tx_buffer;
            tx_buffer[0] = MIDI_ACTIVE_SENSING_MESSAGE;
            midi_transmit.complete_callback = midiActiveSenceCallback;
            i2cMasterTransaction(&midi_transmit, 0);
            // update AS
            next_update = counter_sr + (SAMPLE_RATE * MIDI_ACTIVE_SENSING_GEN_MS / 1000);
            return true;
        } else
            return false;
    }
    midi_transmit.status = 0;
    midi_transmit.address = I2C_MIDI_REMOTE_ADDRESS;
    midi_transmit.len = I2C_MIDI_BUFFER_SIZE * 4;
    midi_transmit.data = (const uint8_t*)tx_buffer;
    midi_transmit.complete_callback = midiTxCallback;
    midi_receive.status = 0;
    midi_receive.address = I2C_MIDI_REMOTE_ADDRESS | 1;
    midi_receive.len = I2C_MIDI_BUFFER_SIZE * 4;
    midi_receive.data = (const uint8_t*)rx_buffer;
    midi_receive.complete_callback = midiRxCallback;
    i2cMasterTransaction(&midi_transmit, &midi_receive);
    return true;
}