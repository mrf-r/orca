#include "orca.h"
#include "NUC123.h"
#include "i2c.h"
/*
400 kHz or 1 MHz

we need multimaster capabilities
1 master transmit - lcd
2 master transmit-rs-receive - eeprom

3 master transmit - midi panel
4 slave receive - midi panel
4 slave transmit -

NAK leads to another aatempt and then switching to scan mode
After first slave transmit transaction master will be disabled!


*/
#define I2CMIDI_MESSAGE_SIZE 4
#define I2CMIDI_OWNADDRESS 0x92
#define I2CMIDI_MAX_BULK 4 // TODO : messages per transaction
#define I2C_SLAVEONLY_SUPPORT 0
#define I2C_ATOMIC_START() __disable_irq()
#define I2C_ATOMIC_END() __enable_irq()
extern volatile uint32_t counter_sr;
#define I2C_TIME_GET() (counter_sr)
#define I2C_TIMEOUT ((400000 / 10 + SAMPLE_RATE - 1) / SAMPLE_RATE + 2)

// returns 0 in case of buffer overflow, send nack
__attribute__((weak)) uint32_t i2cMidiReceiveCallback(void* message)
{
    (void)message;
    return 0;
}

// this will be called on transmit to read midi buffer
// returns 0 AND writes message with 0 in case nothing to send
__attribute__((weak)) uint32_t i2cMidiTransmitCallback(void* message)
{
    // BYTES ACCESS (MAY BE UNALIGNED)
    for (int i = 0; i < I2CMIDI_MESSAGE_SIZE; i++)
        ((uint8_t*)message)[i] = 0;
    return 0;
}

void i2cStart()
{
    I2C0->I2CLK = 17; // 17 - set to 1 MHz or 44 - to 400kHz, 89 for something slow
    I2C0->I2CON = I2C_I2CON_ENS1_Msk;

    I2C0->I2CADDR0 = I2CMIDI_OWNADDRESS;
    I2C0->I2CTOC = I2C_I2CTOC_ENTI_Msk;
    I2C0->I2CON |= I2C_I2CON_EI_Msk;
    NVIC_SetPriority(I2C0_IRQn, IRQ_PRIORITY_I2C);
    // NVIC_EnableIRQ(I2C0_IRQn);
}

static uint8_t its_address;
static uint16_t its_length_w;
static uint16_t its_length_r;
static uint8_t its_firstbyte_w; // dumb a f, but LCD needs that crap
static uint8_t* its_data_w;
static uint8_t* its_data_r;

// common data
static uint16_t its_position;
static uint8_t its_status;
static uint8_t its_timeout;
static its_sbuf[I2CMIDI_MESSAGE_SIZE];
static uint8_t its_midi_bulk;

#define ITSM_NACK 0x01
#define ITSM_ARLO 0x02
#define ITSM_TXC 0x04
#define ITSM_OFF 0x08
#define ITSR_NACK 0x10
#define ITSR_RXC 0x20
#define ITS_MIDI 0x40
#define ITS_BUSY 0x80

#define ITS_INIT (ITSM_OFF)

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// master transmit
static uint8_t ista_08_10() // start or restart set
{
    if (its_status & ITS_MIDI) {
        its_midi_bulk = 0;
    }
    if (its_length_w) {
        I2C0->I2CDAT = its_address;
        return I2C_I2CON_SI;
    } else if (its_length_r) {
        I2C0->I2CDAT = its_address & 0x01;
        return I2C_I2CON_SI;
    } else {
        its_status &= ~ITS_BUSY;
        return I2C_I2CON_STO_SI_AA;
    }
}
static uint8_t ista_18() {
    // TODO: lcd needs firstbyte
}

static uint8_t ista_18_28() // addr or data ack
{
    ASSERT(its_data_w);
    if (its_length_w) {
        I2C0->I2CDAT = *its_data_w;
        its_data_w++;
        its_length_w--;
        return I2C_I2CON_SI;
    } else {
        its_status |= ITSM_TXC;
        if ((its_status & ITS_MIDI) & (its_midi_bulk < I2CMIDI_MAX_BULK)) {
            its_midi_bulk++;
            if (i2cMidiTransmitCallback(its_sbuf)) {
                I2C0->I2CDAT = *its_sbuf;
                its_data_w = its_sbuf + 1;
                its_length_w = I2CMIDI_MESSAGE_SIZE - 1;
                return I2C_I2CON_SI;
            }
        }
        if (its_length_r) {
            return I2C_I2CON_STA_STO_SI;
        } else {
            its_status &= ~ITS_BUSY;
            return I2C_I2CON_STO_SI_AA;
        }
        // its_status &= ~ITS_BUSY;
        // return its_length_r ? I2C_I2CON_STA_STO_SI : I2C_I2CON_STO_SI_AA;
    }
}
static uint8_t ista_20_30_48() // addr or data nack
{
    // TODO: midi message will be lost
    its_status |= ITSM_NACK;
    its_status &= ~ITS_BUSY;
    return I2C_I2CON_STO_SI_AA;
}
static uint8_t ista_38() // arlo
{
    its_status |= ITSM_ARLO;
    its_status &= ~ITS_BUSY;
    return I2C_I2CON_STO_SI_AA;
}
///////////////////////////////////////////////////////////////////////////////////////////
// master receive
static uint8_t ista_40() // addr ack on receive, do nothng, wait for data
{
    its_status |= ITS_BUSY;
    its_position = 0;
    return I2C_I2CON_SI_AA;
}
static uint8_t ista_50() // data ack
{
    ASSERT(its_length_r > 0);
    ASSERT(its_data_r);
    *its_data_r = I2C0->I2CDAT;
    its_data_r++;
    its_length_r--;
    return its_length_r == 1 ? I2C_I2CON_SI_AA : I2C_I2CON_SI;
    // TODO: next midi message receive ? define polling rules
}
static uint8_t ista_58() // data nack
{
    ASSERT(its_length_r > 0);
    ASSERT(its_data_r);
    *its_data_r = I2C0->I2CDAT;
    its_data_r++;
    its_length_r--;
    its_status |= ITSR_RXC;
    its_status &= ~ITS_BUSY;
    return I2C_I2CON_STO_SI_AA;
}
///////////////////////////////////////////////////////////////////////////////////////////
// slave receive
static uint8_t ista_60_68_70_78() // addr ack / nack (from master)
{
    its_status |= ITS_BUSY;
    its_position = 0;
    its_midi_bulk = 0;
    return I2C_I2CON_SI_AA;
}
static uint8_t ista_68_78() // addr arlo (from master)
{
    its_status |= ITSM_ARLO;
    return ista_60_68_70_78();
}
static uint8_t ista_80_90() // data ack
{
    its_sbuf[its_position] = I2C0->I2CDAT;
    its_position++;
    if (its_position == I2CMIDI_MESSAGE_SIZE) {
        its_position = 0;
        its_midi_bulk++;
        if (i2cMidiReceiveCallback((void*)its_sbuf) && (its_midi_bulk < I2CMIDI_MAX_BULK))
            return I2C_I2CON_SI_AA;
        else
            return I2C_I2CON_SI;
    }
    return I2C_I2CON_SI_AA;
}
static uint8_t ista_88_98() // data nack
{
    uint8_t purge = I2C0->I2CDAT;
    return I2C_I2CON_SI;
    (void)purge;
}
///////////////////////////////////////////////////////////////////////////////////////////
// slave transmit
static uint8_t ista_A0() // stop or restart
{
    return I2C_I2CON_SI_AA;
    // TODO: we don't know whether it stop or restart
    // so we need some timeout
}
static uint8_t ista_A8() // addr ack
{
    // reception means that we are kindly asked to not initiate master anymore
#if I2C_SLAVEONLY_SUPPORT == 1
    its_status |= ITSM_OFF;
    ASSERT(0);
    // we need full support
    // and full define. is it MIDI only??
#endif
    i2cMidiTransmitCallback(its_sbuf);
    I2C0->I2CDAT = its_sbuf[0];
    its_position = 1;
    return I2C_I2CON_SI_AA;
}
static uint8_t ista_B0() // addr nack (from master)
{
    its_status = ITSM_ARLO;
    return ista_A8();
}
static uint8_t ista_B8_C0() // data ack or nack
{
    if (its_position == I2CMIDI_MESSAGE_SIZE) {
        its_position = 0;
        i2cMidiTransmitCallback(its_sbuf);
    }
    I2C0->I2CDAT = its_sbuf[its_position];
    its_position++;
    return I2C_I2CON_SI_AA;
}
///////////////////////////////////////////////////////////////////////////////////////////
// common events
// TODO: in both cases probably the better way is to force stop condition with GPIO
static uint8_t ista_00() // bus error
{
    ASSERT(0);
    return I2C_I2CON_STO_SI_AA;
}
static uint8_t ista_F8() // probably timer due to stretching or just free bus
{
    if (its_status & ITS_BUSY) {
        ASSERT(1);
    }
    return I2C_I2CON_SI_AA;
}

static uint8_t (*const ista[32])(void) = {
    /* 00 bus error     */ ista_00,
    /* 08 M  start      */ ista_08_10,
    /* 10 M  restart    */ ista_08_10,
    /* 18 MT addr ack   */ ista_18_28,
    /* 20 MT addr nack  */ ista_20_30_48,
    /* 28 MT data ack   */ ista_18_28,
    /* 30 MT data nack  */ ista_20_30_48,
    /* 38 arlo          */ ista_38,
    /* 40 MR addr ack   */ ista_40,
    /* 48 MR addr nack  */ ista_20_30_48,
    /* 50 MR data ack   */ ista_50,
    /* 58 MR data nack  */ ista_58,
    /* 60 SR addr ack   */ ista_60_68_70_78,
    /* 68 SR addr arlo  */ ista_60_68_70_78,
    /* 70 SRG addr ack  */ ista_60_68_70_78,
    /* 78 SRG addr arlo */ ista_60_68_70_78,
    /* 80 SR data ack   */ ista_80_90,
    /* 88 SR data nack  */ ista_88_98,
    /* 90 SRG data ack  */ ista_80_90,
    /* 98 SRG data nack */ ista_88_98,
    /* A0 S stop restart*/ ista_A0,
    /* A8 ST addr ack   */ ista_A8,
    /* B0 ST addr arlo  */ ista_B0,
    /* B8 ST data ack   */ ista_B8_C0,
    /* C0 ST data nack  */ ista_B8_C0,
    /* C8   undef       */ ista_00,
    /* D0   undef       */ ista_00,
    /* D8   undef       */ ista_00,
    /* E0   undef       */ ista_00,
    /* E8   undef       */ ista_00,
    /* F0   undef       */ ista_00,
    /* F8 bus free      */ ista_F8,
};

void I2C0_IRQHandler()
{
    // if (NVIC_GetPendingIRQ(I2C0_IRQn)) {
    if (I2C0->I2CON & I2C_I2CON_SI) {
        uint8_t status = I2C0->I2CSTATUS;
#if DEBUG == 1
        static volatile uint32_t i2c_sta_flags = 0;
        i2c_sta_flags |= 1 << (status >> 3);
#endif
        // leave IRQ and EN bits as is
        uint32_t ctrl = I2C0->I2CON & 0xFFFFFFC3;
        ctrl |= ista[status >> 3]();
        I2C0->I2CON = ctrl;
        its_timeout = I2C_TIME_GET();
    }
}

void i2cVirtInterrupt()
{
    if (NVIC_GetPendingIRQ(I2C0_IRQn)) {
        I2C0_IRQHandler();
    }
    ASSERT(NVIC_GetPendingIRQ(I2C0_IRQn) == 0);
}

uint32_t i2cLcdTransaction(uint8_t addr, uint8_t firstbyte, uint8_t* tx_buffer, uint16_t tx_len)
{
    I2C_ATOMIC_START();
    if (its_status & ITS_BUSY)
        return;

    if ((I2C0->I2CSTATUS & 0xF8) == 0xF8) {
        its_address = addr;
        its_data_w = tx_buffer;
        its_length_w = tx_len;
        its_data_r = 0;
        its_length_r = 0;
        // start
        I2C0->I2CON |= I2C_I2CON_STA_SI;
        return 0;
    }
    I2C_ATOMIC_END();
}

uint32_t i2cTransaction(uint8_t addr, uint8_t* tx_buffer, uint16_t tx_len, uint8_t* rx_buffer, uint16_t rx_len)
{
    I2C_ATOMIC_START();
    if (its_status & ITS_BUSY)
        return;

    if ((I2C0->I2CSTATUS & 0xF8) == 0xF8) {
        its_address = addr;
        its_data_w = tx_buffer;
        its_length_w = tx_len;
        its_data_r = rx_buffer;
        its_length_r = rx_len;
        // start
        I2C0->I2CON |= I2C_I2CON_STA_SI;
        return 0;
    }
    I2C_ATOMIC_END();
}

lcd_send__(uint8_t* buf, uint32_t length, uint8_t lcd_mode)
{
    // start
    I2C0->I2CON = 0x68; // EN + STA + SI
    while (!(I2C0->I2CON & I2C_I2CON_SI))
        ; // wait
    // address + write
    if (I2C0->I2CSTATUS == 0x08)
        I2C0->I2CDAT = SSD1306_I2C_ADDRESS, I2C0->I2CON = 0x48;
    else
        goto err;
    while (!(I2C0->I2CON & I2C_I2CON_SI))
        ; // wait
    // data 1
    if (I2C0->I2CSTATUS == 0x18)
        I2C0->I2CDAT = lcd_mode, I2C0->I2CON = 0x48;
    else
        goto err;
    // data 2
    for (uint32_t i = 0; i < length; i++) {
        while (!(I2C0->I2CON & I2C_I2CON_SI))
            ; // wait
        if (I2C0->I2CSTATUS == 0x28)
            I2C0->I2CDAT = buf[i], I2C0->I2CON = 0x48;
        else
            goto err;
    }
    while (!(I2C0->I2CON & I2C_I2CON_SI))
        ; // wait
    // stop
    I2C0->I2CON = 0x58;
    return 0;
err:
    I2C0->I2CON = 0x58;
    return 1;
}
