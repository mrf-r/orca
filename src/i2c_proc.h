#ifndef _I2C_PROC_H
#define _I2C_PROC_H

#include <stdint.h>
#include <stdbool.h>
#include "orca.h" // TODO: ASSERT

typedef struct I2CTransaction_{
    uint8_t status;
    uint8_t address;
    uint16_t len;
    const uint8_t* data;
    struct I2CTransaction_* (*complete_callback)(void);
} I2CTransaction;

typedef enum {
    I2C_MODE_FREE = 0,
    I2C_MODE_MASTER_TRANSMITTER,
    I2C_MODE_MASTER_RECEIVER,
    I2C_MODE_SLAVE_TRANSMITER,
    I2C_MODE_SLAVE_RECEIVER,
    I2C_MODE_UNCERTAIN
} ItmModeEn;

// i2c transaction status
#define ITS_STARTED 0x01
#define ITS_COMPLETE 0x02
#define ITS_NACK 0x04 // only possible for receivers
#define ITS_ARLO 0x08 // only possible for masters
#define ITS_TIMEOUT 0x10 // only possible for masters
#define ITS_RESULT_OK (ITS_STARTED | ITS_COMPLETE)

#define I2C_OK 0
#define I2C_BUSY 1

void i2cStart(uint8_t own_address);
void i2cSeqTap(void);
// int i2cMasterTransaction(const I2CTransaction *wtr, const I2CTransaction *rtr);
// int i2cSlaveSetup(I2CTransaction *rtr, I2CTransaction *wtr);
void i2cSeqTransaction(I2CTransaction* tx, I2CTransaction* rx, bool is_blocked);

extern volatile uint32_t counter_sr;
#define I2C_TIME_GET() (counter_sr)

// i2c devices
extern uint8_t lcd_framebuffer[128 * 8];
void lcdStart(void);

#endif // _I2C_PROC_H
