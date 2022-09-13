#include "SEGGER_RTT.h"
#include "string.h"

void print_s(const char* s)
{
    SEGGER_RTT_WriteNoLock(0, s, strlen(s));
}

void print_d32(const unsigned s)
{
    const char toh[17] = "0123456789ABCDEF";
    for (unsigned i = 0; i < 8; i++) {
        unsigned dig = (s >> (28 - (i * 4))) & 0xF;
        SEGGER_RTT_WriteNoLock(0, &toh[dig], 1);
    }
}
