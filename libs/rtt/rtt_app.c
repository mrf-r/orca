#include <stdio.h>
#include <stdarg.h>

#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"

#include "cli.h"

extern CliInst monitor;

void rtt_printf(const char* formatstring, ...)
{
    int size = 0;
    char buff[SEGGER_RTT_PRINTF_BUFFER_SIZE];
    // memset(buff, 0, sizeof(buff));
    va_list args;
    va_start(args, formatstring);
    size = vsnprintf(buff, sizeof(buff), formatstring, args);
    SEGGER_RTT_WriteNoLock(0, buff, size);
    va_end(args);
}

void rtt_check()
{
    char c;
    if (SEGGER_RTT_ReadNoLock(0, &c, 1) == 1) {
        cliParseChar(&monitor,c);
        //SEGGER_RTT_WriteNoLock(0, &c, 1);
    }
}

void rtt_init(void)
{
    SEGGER_RTT_Init();
    SEGGER_RTT_WriteString(0, "SEGGER Real-Time-Terminal Sample" CLI_NEWLINE);
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// cli

void test1(char* arg, void (*const p)(const char*, ...));
void test2(char* arg, void (*const p)(const char*, ...));


CliCommand monitor_commands[] = {
  CLI_COMMAND_NORMAL(NAME(test1), test1,     "run test1 func"),
  CLI_COMMAND_NORMAL(NAME(test2), test2,     "run test2 func"),
    // add more commands here
};
const uint16_t monitor_commands_count = sizeof(monitor_commands)/sizeof(CliCommand);

//////////////////////////////////////////////////////////////////////////////////////////

extern uint32_t lcg;
extern uint32_t sr_counter;
extern uint32_t cr_counter;
extern uint32_t fractional_period;

CliVariable monitor_variables[] = {
    CLI_VAR_RO32(lcg),
    CLI_VAR_RO32(sr_counter),
    CLI_VAR_RO32(cr_counter),
    CLI_VAR_RO32(fractional_period),
};
const uint16_t monitor_variables_count = sizeof(monitor_variables)/sizeof(CliVariable);




CliHandleData monitor_data;
CliInst monitor = {/*.print    = */ rtt_printf,
                   /*.prompt   = */ "SL_seq>",
                   /*.chd      = */ &monitor_data,
                   /*.cmdlist  = */ monitor_commands,
                   /*.cmdcount = */ monitor_commands_count,
                   /*.varlist  = */ monitor_variables,
                   /*.varcount = */ monitor_variables_count};

void cliReceiveCallback(char c) { cliParseChar(&monitor, c); }


