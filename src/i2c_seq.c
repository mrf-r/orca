#include "i2c_proc.h"
#include "orca.h"

/*
400kHz - 25 uS
10 bytes - after master
5 bytes - after slave

transaction types:
- midi: transfer from buffer with max bulk size limitation
- regular access - display / adc / dac
    those are performed by the sequencer
- out of order: direct access to bus devices that is triggered by user
    those performed by direct call of transfer init blocking function.
    normally it should wait for BUSFREE period and access bus

by the time of sequencer run everything should be initialized
probably we should have sequencer block capability to reset bus in case something is wrong or it should be a part of the sequencer
oh and yes, it is in critical loop

MIDI probably should have some kind of handshake ?? state machine??
===================================================
lcd: 1024 bytes / frame
full frame time 0.0256 sec
max fps 39.0625

@ 25 fps
    bus allocation is 64 %
    36 % for midi - 14400 bytes
    3600 midi messages / second
    hw midi is
        about 3125 bytes / second
        1250 messages / second
    keep in mind, that midi is full duplex, while i2c is half-duplex
    so basically @ 25 fps we have almost the same throughput
*/


#define I2C_TIMEOUT(bytes) ((SAMPLE_RATE * bytes + 400000 / 10) / (400000 / 10))

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// LCD
bool i2cSeqMain(void);
bool i2cSeqMidi(void);
bool i2cSeqLcd(void);
