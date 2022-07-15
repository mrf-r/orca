#ifndef _CLI_H
#define _CLI_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CLI_CSILENGTH 4
#define CLI_MAXINPUTSTRINGLENGTH 32
#define CLI_HISTORYDEPTH 5
#define CLI_NEWLINE "\r\n"

// stringify
#define NAME(x) #x
// d for define, example: #define NUMBER (45)
#define DNAME(x) #x
#define DVALUE(x) DNAME x

typedef struct {
    char csibuf[4];
    uint8_t csipos;
    char historybuffer[CLI_HISTORYDEPTH][CLI_MAXINPUTSTRINGLENGTH];
    uint8_t historypos;
    char mainbuffer[CLI_MAXINPUTSTRINGLENGTH];
    char parsebuffer[CLI_MAXINPUTSTRINGLENGTH];
    uint32_t status;
    uint16_t cursor_position; // also used for parsing
} CliHandleData;

typedef const struct CliInstS CliInst;

typedef const struct CliCommandS {
    const char* name;
    void (*hdl)(char* arg, void (*const printfunction)(const char*, ...));
    void (*hdl_ex)(const struct CliInstS* cli);
    const char* help;
} CliCommand;

#define CLI_COMMAND_NORMAL(cmd_str, handler, help_str)                 \
    {                                                                  \
        .name = cmd_str, .hdl = handler, .hdl_ex = 0, .help = help_str \
    }
#define CLI_COMMAND_EXTENDED(cmd_str, handler, help_str)               \
    {                                                                  \
        .name = cmd_str, .hdl = 0, .hdl_ex = handler, .help = help_str \
    }

typedef enum {
    CLI_VARTYPE_S8,
    CLI_VARTYPE_S16,
    CLI_VARTYPE_S32,
    CLI_VARTYPE_FLOAT,
    CLI_VARTYPE_S8_READONLY,
    CLI_VARTYPE_S16_READONLY,
    CLI_VARTYPE_S32_READONLY,
    CLI_VARTYPE_FLOAT_READONLY,
    CLI_VARTYPE_TOTAL
} CliVariableTypeEn;

typedef const struct CliVariableS {
    const char* name;
    CliVariableTypeEn type;
    void* addr;
    // int32_t min;
    // int32_t max;
    union {
        int32_t s32;
        float f32;
    } min;
    union {
        int32_t s32;
        float f32;
    } max;
} CliVariable;

#define CLI_VAR_RO32(var)                                                                                     \
    {                                                                                                         \
        .name = #var, .type = CLI_VARTYPE_S32_READONLY, .addr = &var, .min = { .s32 = 0 }, .max = {.s32 = 0 } \
    }
#define CLI_VAR_RO16(var)                                                                                     \
    {                                                                                                         \
        .name = #var, .type = CLI_VARTYPE_S16_READONLY, .addr = &var, .min = { .s32 = 0 }, .max = {.s32 = 0 } \
    }
#define CLI_VAR_RO8(var)                                                                                     \
    {                                                                                                        \
        .name = #var, .type = CLI_VARTYPE_S8_READONLY, .addr = &var, .min = { .s32 = 0 }, .max = {.s32 = 0 } \
    }
#define CLI_VAR_ROFLOAT(var)                                                                                    \
    {                                                                                                           \
        .name = #var, .type = CLI_VARTYPE_FLOAT_READONLY, .addr = &var, .min = { .f32 = 0 }, .max = {.f32 = 0 } \
    }

#define CLI_VAR_RW32(var, vmin, vmax)                                                                      \
    {                                                                                                      \
        .name = #var, .type = CLI_VARTYPE_S32, .addr = &var, .min = { .s32 = vmin }, .max = {.s32 = vmax } \
    }
#define CLI_VAR_RW16(var, vmin, vmax)                                                                      \
    {                                                                                                      \
        .name = #var, .type = CLI_VARTYPE_S16, .addr = &var, .min = { .s32 = vmin }, .max = {.s32 = vmax } \
    }
#define CLI_VAR_RW8(var, vmin, vmax)                                                                      \
    {                                                                                                     \
        .name = #var, .type = CLI_VARTYPE_S8, .addr = &var, .min = { .s32 = vmin }, .max = {.s32 = vmax } \
    }
#define CLI_VAR_RWFLOAT(var, vmin, vmax)                                                                     \
    {                                                                                                        \
        .name = #var, .type = CLI_VARTYPE_FLOAT, .addr = &var, .min = { .f32 = vmin }, .max = {.f32 = vmax } \
    }

struct CliInstS {
    void (*print)(const char* str, ...);
    // prompt string (ex. "device>"), should probably be replaced by str get
    // method
    const char* prompt;
    CliHandleData* chd; // buffers
    // CliCommand *cmdlist_internal;  // CSI terminal handling
    // uint16_t cmdcount_internal;
    CliCommand* cmdlist; // application
    uint16_t cmdcount;
    CliVariable* varlist;
    uint16_t varcount;
};

void cliParseChar(CliInst* cli, char ch);

#ifdef __cplusplus
}
#endif

#endif // _CLI_H