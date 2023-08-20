#ifndef _I2C_PROC_H
#define _I2C_PROC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t status;
    const uint8_t address;
    const uint16_t len;
    const uint8_t* data;
} I2CTransaction;

typedef I2CTransaction* (*I2cCallback)(void);

typedef enum {
    ITM_FREE = 0,
    ITM_MASTER_TRANSMITTER,
    ITM_MASTER_RECEIVER,
    ITM_SLAVE_TRANSMITER,
    ITM_SLAVE_RECEIVER,
    ITM_UNCERTAIN
} ItmModeEn;

#define ITS_STARTED 0x01
#define ITS_COMPLETE 0x01
#define ITS_NACK 0x01 // only possible for receivers
#define ITS_ARLO 0x01 // only possible for masters

#define I2C_OK 0
#define I2C_BUSY 1

void i2cStart(uint8_t own_address);
int i2cMasterTransaction(I2CTransaction *wtr, I2CTransaction *rtr, I2cCallback wcb, I2cCallback rcb);
int i2cSlaveSetup(I2CTransaction *rtr, I2cCallback rcb, I2CTransaction *wtr, I2cCallback wcb);

#endif // _I2C_PROC_H
