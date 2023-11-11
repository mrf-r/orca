#include "i2c_proc.h"

int i2cMasterTransaction(I2CTransaction* wtr, I2CTransaction* rtr);

static I2CTransaction* i2c_tx_request;
static I2CTransaction* i2c_rx_request;

void i2cSeqTransaction(I2CTransaction* tx, I2CTransaction* rx, bool is_blocked)
{
    i2c_tx_request = tx, i2c_rx_request = rx;
    if (is_blocked) {
        while (true) {
            criticalLoop();
            if (rx) {
                if (rx->status & (ITS_COMPLETE | ITS_NACK | ITS_TIMEOUT))
                    return;
            } else {
                if (tx)
                    if (tx->status & (ITS_COMPLETE | ITS_NACK | ITS_TIMEOUT))
                        return;
            }
        }
    }
}

// to be called when bus is empty
// returns true if transaction was initiated
bool i2cSeqMain()
{
    if (i2c_tx_request || i2c_rx_request) {
        i2cMasterTransaction(i2c_tx_request, i2c_rx_request);
        i2c_tx_request = 0;
        i2c_rx_request = 0;
        return true;
    }
    return false;
}