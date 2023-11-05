#include "i2c_proc.h"
#include "NUC123.h"
#include "i2c.h"
#include "orca.h"

/*

MIDI HAS VARIABLE LENGTH MESSAGES

 - fully independent in separate thread
    with request_next_callback (iic sceduler call)

    i2c: transaction - timer wait - transaction ...
    midi: highest priority: poll for completion - next
    eeprom: request - poll for completion - continue
    lcd: poll for completion - timer wait - request

lcd frame sync:


400 kHz or 1 MHz

we need multimaster capabilities
1 master transmit - lcd
    - it should have modes
        - normal
        - disabled (full master control) ??
        - midi character emulation (position, cursor, cgram, bitmaps ?)
        - midi graphic emuilation (high level ?)
2 master transmit-rs-receive - eeprom
    - so far not yet soldered

3 master transmit - midi over iic
    should be periodically tried to get NAK
4 slave receive - midi over iic
    does not need anything special


NAK leads to another aatempt and then switching to scan mode
After first slave transmit transaction master will be disabled??

TODO: check all transaction statuses (complete, etc)
TODO: probably change handler

*/

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// local config

// amount of 32bit messages to receive or transmit
#define I2C_MIDI_BLOCK_SIZE 8

#define I2C_ATOMIC_START() __disable_irq()
#define I2C_ATOMIC_END() __enable_irq()

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

static uint8_t i2c_current_mode;
static uint32_t i2c_last_event_time;
static bool i2c_last_role_was_master;

// master transactions
static uint8_t* itsm_wp; // write pointer
static uint8_t* itsm_we; // write end
static uint8_t* itsm_rp; // read pointer
static uint8_t* itsm_re; // read end
static I2CTransaction* itsm_wtr;
static I2CTransaction* itsm_rtr;
// next or previous, depending on a pov
static uint8_t* itsmr_nd = 0; // receiver next data
static uint8_t itsmr_ns; // receiver next status
static uint8_t* itsmr_nsp; // receiver next status pointer

// slave transactions
static uint8_t* itss_wp;
static uint8_t* itss_we;
static uint8_t* itss_re;
static uint8_t* itss_rp;
static I2CTransaction* itss_wtr;
static I2CTransaction* itss_rtr;

// buffer handling
static inline void itsm_w_refresh()
{
    itsm_wp = (uint8_t*)itsm_wtr->data;
    itsm_we = itsm_wp + itsm_wtr->len;
    ASSERT(itsm_wtr->status == 0);
    ASSERT(itsm_wp);
    ASSERT(itsm_we > itsm_wp);
}
static inline void itsm_r_refresh()
{
    itsm_rp = (uint8_t*)itsm_rtr->data;
    itsm_re = itsm_rp + itsm_rtr->len;
    ASSERT(itsm_rtr->status == 0);
    ASSERT(itsm_rp);
    ASSERT(itsm_re > itsm_rp);
}
static inline void itss_w_refresh()
{
    itss_wp = (uint8_t*)itss_wtr->data;
    itss_we = itss_wp + itss_wtr->len;
    ASSERT(itss_wtr->status == 0);
    ASSERT(itss_wp);
    ASSERT(itss_we > itss_wp);
}
static inline void itss_r_refresh()
{
    itss_rp = (uint8_t*)itss_rtr->data;
    itss_re = itss_rp + itss_rtr->len;
    ASSERT(itss_rtr->status == 0);
    ASSERT(itss_rp);
    ASSERT(itss_re > itss_rp);
}

void i2cStart(uint8_t own_address)
{
    I2C0->I2CLK = 17; // 17 - set to 1 MHz or 44 - to 400kHz, 89 for something slow
    I2C0->I2CON = I2C_I2CON_ENS1_Msk;

    I2C0->I2CADDR0 = own_address;
    // I2C0->I2CTOC = I2C_I2CTOC_ENTI_Msk; // timeout ?
    I2C0->I2CON |= I2C_I2CON_EI_Msk;
    NVIC_SetPriority(I2C0_IRQn, IRQ_PRIORITY_I2C);
#if IRQ_DISABLE == 0
    NVIC_EnableIRQ(I2C0_IRQn);
#endif
}

int i2cMasterTransaction(I2CTransaction* wtr, I2CTransaction* rtr)
{
    ASSERT((wtr && rtr) ? (wtr->address == (rtr->address & 1)) : true);
    int ret = I2C_BUSY;
    I2C_ATOMIC_START();
    // TODO return inside atomic
    if (i2c_current_mode == I2C_MODE_FREE) {
        itsm_wtr = wtr;
        itsm_rtr = rtr;
        // start
        I2C0->I2CON |= I2C_I2CON_STA_SI;
        i2c_current_mode = I2C_MODE_UNCERTAIN;
        ret = I2C_OK;
    }
    I2C_ATOMIC_END();
    return ret;
}

int i2cSlaveSetup(I2CTransaction* rtr, I2CTransaction* wtr)
{
    int ret = I2C_BUSY;

    I2C_ATOMIC_START();
    // TODO return inside atomic
    if (i2c_current_mode == I2C_MODE_FREE) {
        itss_rtr = rtr;
        itss_r_refresh();
        itss_wtr = wtr;
        itss_w_refresh();
        ret = I2C_OK;
    }
    I2C_ATOMIC_END();
    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////
// master transmit

// start or restart set
static uint8_t ista_08_10()
{
    i2c_last_role_was_master = true;
    if (itsm_wtr) {
        itsm_w_refresh();
        itsm_wtr->status |= ITS_STARTED;
        I2C0->I2CDAT = itsm_wtr->address;
        i2c_current_mode = I2C_MODE_MASTER_TRANSMITTER;
        return I2C_I2CON_SI_AA;
    } else if (itsm_rtr) {
        itsm_r_refresh();
        I2C0->I2CDAT = itsm_rtr->address;
        itsm_rtr->status |= ITS_STARTED;
        i2c_current_mode = I2C_MODE_MASTER_RECEIVER;
        return I2C_I2CON_SI_AA;
    } else {
        i2c_current_mode &= ~I2C_BUSY;
        return I2C_I2CON_STO_SI_AA;
    }
}

// addr or data ack
static uint8_t ista_18_28()
{
    if (itsm_wp < itsm_we) {
        I2C0->I2CDAT = *itsm_wp;
        itsm_wp++;
        return I2C_I2CON_SI_AA;
    } else {
        itsm_wtr->status |= ITS_COMPLETE;
        if (itsm_wtr->complete_callback) {
            itsm_wtr = itsm_wtr->complete_callback();
            if (itsm_wtr) {
                itsm_w_refresh();
                I2C0->I2CDAT = *itsm_wp;
                itsm_wp++;
                itsm_wtr->status |= ITS_STARTED;
                return I2C_I2CON_SI_AA;
            }
        } else {
            itsm_wtr = 0;
        }
        if (itsm_rtr) {
            return I2C_I2CON_STA_STO_SI_AA;
        } else {
            i2c_current_mode = I2C_MODE_FREE;
            return I2C_I2CON_STO_SI_AA;
        }
    }
}

// addr or data nack
static uint8_t ista_20_30()
{
    itsm_wtr->status |= ITS_NACK;
    i2c_current_mode = I2C_MODE_FREE;
    return I2C_I2CON_STO_SI_AA;
}

// arlo
static uint8_t ista_38()
{
    i2c_last_role_was_master = false;
    if (itsm_wtr) {
        itsm_wtr->status |= ITS_ARLO;
    }
    if (itsm_rtr) {
        itsm_rtr->status |= ITS_ARLO;
    }
    i2c_current_mode = I2C_MODE_FREE;
    return I2C_I2CON_STO_SI_AA;
}

///////////////////////////////////////////////////////////////////////////////////////////
// master receive

// addr ack on receive, do nothng, wait for data
static uint8_t ista_40()
{
    ASSERT(itsm_rtr);
    itsmr_nsp = &itsm_rtr->status;
    itsmr_nd = itsm_rp;
    itsm_rp++;
    if (itsm_rp < itsm_re) {
        itsmr_ns = ITS_STARTED;
    } else {
        itsmr_ns = ITS_COMPLETE;
        if (itsm_rtr->complete_callback) {
            itsm_rtr = itsm_rtr->complete_callback();
            if (itsm_rtr) {
                itsm_r_refresh();
                return I2C_I2CON_SI_AA;
            }
        }
        return I2C_I2CON_SI;
    }
    return I2C_I2CON_SI_AA;
}

// data ack
static uint8_t ista_50()
{
    *itsmr_nd = I2C0->I2CDAT;
    *itsmr_nsp |= itsmr_ns;
    return ista_40();
}

// addr or data nack
static uint8_t ista_48_58()
{
    if (itsmr_ns == ITS_COMPLETE) {
        *itsmr_nd = I2C0->I2CDAT;
        *itsmr_nsp |= itsmr_ns;
    } else {
        *itsmr_nsp |= ITS_NACK;
    }
    i2c_current_mode = I2C_MODE_FREE;
    return I2C_I2CON_STO_SI_AA;
}

///////////////////////////////////////////////////////////////////////////////////////////
// slave receive
uint8_t itssr_ns;
uint8_t* itssr_nsp;
uint8_t* itssr_nd;

// (gc) addr ack
static uint8_t ista_60_70()
{
    i2c_current_mode = I2C_MODE_SLAVE_RECEIVER;
    i2c_last_role_was_master = false;
    if (itss_rtr) {
        itssr_nsp = &itss_rtr->status;
        itssr_nd = itss_rp;
        itss_rp++;
    } else {
        itssr_nd = 0;
        itssr_nsp = 0;
    }
    if (itss_rp < itss_re) {
        itssr_ns = ITS_STARTED;
    } else {
        itssr_ns = ITS_COMPLETE;
        if (itss_rtr->complete_callback) {
            itss_rtr = itss_rtr->complete_callback();
            if (itss_rtr) {
                itss_r_refresh();
                return I2C_I2CON_SI_AA;
            }
        }
        itss_rtr = 0;
        itss_rp = 0;
        itss_re = 0;
        return I2C_I2CON_SI;
    }
    return I2C_I2CON_SI_AA;
}

// (gc) addr arlo (from master mode)
static uint8_t ista_68_78()
{
    i2c_last_role_was_master = false;
    if (itsm_wtr) {
        itsm_wtr->status |= ITS_ARLO;
    }
    if (itsm_rtr) {
        itsm_rtr->status |= ITS_ARLO;
    }
    return ista_60_70();
}

// (gc) data ack
static uint8_t ista_80_90()
{
    if (itssr_nd) {
        *itssr_nd = I2C0->I2CDAT;
        *itssr_nsp |= itssr_ns;
    } else {
        uint8_t dummy = I2C0->I2CDAT;
        (void)dummy;
    }
    return ista_60_70();
}

// (gc) data nack
static uint8_t ista_88_98()
{
    ASSERT(itssr_ns == ITS_COMPLETE);
    if (itssr_nd) {
        *itssr_nd = I2C0->I2CDAT;
        *itssr_nsp |= itssr_ns;
    } else {
        uint8_t dummy = I2C0->I2CDAT;
        (void)dummy;
    }
    i2c_current_mode = I2C_MODE_FREE;
    return I2C_I2CON_SI_AA;
}

///////////////////////////////////////////////////////////////////////////////////////////
// slave transmit

// addr or data ack
static uint8_t ista_A8_B8()
{
    i2c_current_mode = I2C_MODE_SLAVE_TRANSMITER;
    i2c_last_role_was_master = false;
    if (itss_wp < itss_we) {
        I2C0->I2CDAT = *itss_wp;
        itss_wp++;
        itss_wtr->status |= ITS_STARTED;
        return I2C_I2CON_SI_AA;
    } else {
        if (itss_wtr) {
            itss_wtr->status |= ITS_COMPLETE;
        }
        if (itss_wtr->complete_callback) {
            itss_wtr = itss_wtr->complete_callback();
            if (itss_wtr) {
                itss_w_refresh();
                I2C0->I2CDAT = *itss_wp;
                itss_wp++;
                return I2C_I2CON_SI_AA;
            }
        }
        itss_wtr = 0;
        itss_wp = 0;
        itss_we = 0;
        I2C0->I2CDAT = 0;
    }
    return I2C_I2CON_SI_AA;
}

// addr arlo (from master)
static uint8_t ista_B0()
{
    i2c_last_role_was_master = false;
    if (itsm_wtr) {
        itsm_wtr->status |= ITS_ARLO;
    }
    if (itsm_rtr) {
        itsm_rtr->status |= ITS_ARLO;
    }
    return ista_A8_B8();
}

// stop/restart or data nack or last data ack
static uint8_t ista_A0_C0_C8()
{
    // TODO: A0 ???
    i2c_current_mode = I2C_MODE_FREE;
    return I2C_I2CON_SI_AA;
}

///////////////////////////////////////////////////////////////////////////////////////////
// common events
// TODO: in both cases probably the better way is to force stop condition with GPIO
static uint8_t ista_00() // bus error
{
    ASSERT(false);
    I2C0->I2CON = I2C_I2CON_STO_SI_AA;
    // TODO: delay ?
    return I2C_I2CON_SI_AA;
}
static uint8_t ista_F8() // probably timer due to stretching or just free bus
{
    ASSERT(false);
    return I2C_I2CON_SI_AA;
}

static uint8_t (*const ista[32])(void) = {
    /* 00 bus error     */ ista_00,
    /* 08 M  start      */ ista_08_10,
    /* 10 M  restart    */ ista_08_10,
    /* 18 MT addr ack   */ ista_18_28,
    /* 20 MT addr nack  */ ista_20_30,
    /* 28 MT data ack   */ ista_18_28,
    /* 30 MT data nack  */ ista_20_30,
    /* 38 arlo          */ ista_38,
    /* 40 MR addr ack   */ ista_40,
    /* 48 MR addr nack  */ ista_48_58,
    /* 50 MR data ack   */ ista_50,
    /* 58 MR data nack  */ ista_48_58,
    /* 60 SR addr ack   */ ista_60_70,
    /* 68 SR addr arlo  */ ista_68_78,
    /* 70 SRG addr ack  */ ista_60_70,
    /* 78 SRG addr arlo */ ista_68_78,
    /* 80 SR data ack   */ ista_80_90,
    /* 88 SR data nack  */ ista_88_98,
    /* 90 SRG data ack  */ ista_80_90,
    /* 98 SRG data nack */ ista_88_98,
    /* A0 S stop restart*/ ista_A0_C0_C8,
    /* A8 ST addr ack   */ ista_A8_B8,
    /* B0 ST addr arlo  */ ista_B0,
    /* B8 ST data ack   */ ista_A8_B8,
    /* C0 ST data nack  */ ista_A0_C0_C8,
    /* C8 ST last ack   */ ista_A0_C0_C8,
    /* D8   undef       */ ista_F8,
    /* D0   undef       */ ista_F8,
    /* E0   undef       */ ista_F8,
    /* E8   undef       */ ista_F8,
    /* F0   undef       */ ista_F8,
    /* F8 bus free      */ ista_F8,
};

void I2C0_IRQHandler()
{
    ASSERT(I2C0->I2CON & I2C_I2CON_SI)
    uint8_t status = I2C0->I2CSTATUS;
#if DEBUG == 1
    static volatile uint32_t i2c_sta_flags = 0;
    i2c_sta_flags |= 1 << (status >> 3);
#endif
    // leave IRQ and EN bits as is
    uint32_t ctrl = I2C0->I2CON & 0xFFFFFFC3;
    ctrl |= ista[status >> 3]();
    I2C0->I2CON = ctrl;
    i2c_last_event_time = I2C_TIME_GET();
}

#if IRQ_DISABLE == 1
void i2cVirtInterrupt()
{
    while (NVIC_GetPendingIRQ(I2C0_IRQn)) {
        I2C0_IRQHandler();
        NVIC_ClearPendingIRQ(I2C0_IRQn);
    }
    ASSERT(NVIC_GetPendingIRQ(I2C0_IRQn) == 0);
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// sequencer

#define I2C_BUSFREE_AFTER_SLAVE 4
#define I2C_BUSFREE_AFTER_MASTER (I2C_BUSFREE_AFTER_SLAVE * 2)
#define I2C_TIMEOUT (SAMPLE_RATE / 2)

bool i2cSeqMain(void);
bool i2cSeqMidi(void);
bool i2cSeqLcd(void);

static inline void busfree()
{
    if (i2cSeqMain())
        return;
    else if (i2cSeqLcd())
        return;
    else if (i2cSeqMidi())
        return;
}

void i2cMidiReceiveCallback(uint32_t message)
{
    if (message) {
        // todo timestamping in midi
        (void)message;
    }
}

extern bool i2c_last_role_was_master;
extern uint8_t i2c_current_mode;
extern uint32_t i2c_last_event_time;

void i2cSeqTap()
{
    int32_t delta = I2C_TIME_GET() - i2c_last_event_time;

    switch (i2c_current_mode) {
    case I2C_MODE_FREE: {
        int32_t delay = i2c_last_role_was_master ? I2C_BUSFREE_AFTER_MASTER : I2C_BUSFREE_AFTER_SLAVE;
        if (delta > delay) {
            busfree();
        }
    } break;
    case I2C_MODE_MASTER_TRANSMITTER:
    case I2C_MODE_MASTER_RECEIVER:
        // do not stay more than current transaction length
        // fall through
    case I2C_MODE_UNCERTAIN:
        // do not stay more than 2 bytes
        // ASSERT(delta < I2C_TIMEOUT_MASTER);
        // break;

    case I2C_MODE_SLAVE_TRANSMITER:
    case I2C_MODE_SLAVE_RECEIVER:
        // do not stay more than I2C_MIDI_BUF_SIZE / 2
        ASSERT(delta < I2C_TIMEOUT);
        break;
    default:
        ASSERT(FALSE);
        break;
    }
}
