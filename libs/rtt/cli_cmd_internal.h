#ifndef _CLI_CMD_INTERNAL_H
#define _CLI_CMD_INTERNAL_H

// do not include
// not to be used by app
// only for cli module itself

#include "cli.h"
static void cliNewCommand(CliInst* cli);
// ccn - cli command name
// cch - cli command handler
// ccd - cli command description

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void cch_help(CliInst* cli);
static const char ccd_help[] = "list available commands and variables with description";

void cli_gitversion(char* arg, void (*const pr)(const char*, ...));
void cli_reset(char* arg, void (*const pr)(const char*, ...));

static void cch_set(CliInst* cli);
static void cch_get(CliInst* cli);

static const CliCommand cli_commands_internal[] = {
    CLI_COMMAND_EXTENDED("help", cch_help, ccd_help),
    CLI_COMMAND_EXTENDED("?", cch_help, ccd_help),
    CLI_COMMAND_NORMAL("reset", cli_reset, 0),
    CLI_COMMAND_NORMAL("git", cli_gitversion, 0),
    CLI_COMMAND_NORMAL("build", cli_gitversion, 0),
    CLI_COMMAND_NORMAL("version", cli_gitversion, 0),
    CLI_COMMAND_EXTENDED("set", cch_set, 0),
    CLI_COMMAND_EXTENDED("get", cch_get, 0),
};
static const uint16_t cli_commands_internal_count = sizeof(cli_commands_internal) / sizeof(CliCommand);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cch_help(CliInst* cli)
{
    CliHandleData* chd = cli->chd;
    int charcount;
    if (sscanf(&chd->mainbuffer[chd->cursor_position], "%s%n", chd->parsebuffer, &charcount) != 1) {
        chd->parsebuffer[0] = '\0';
        charcount = 0;
    }
    chd->cursor_position += charcount;
    // internal
    for (uint16_t i = 0; i < cli_commands_internal_count; i++) {
        if (strstr(cli_commands_internal[i].name, chd->parsebuffer)) {
            CliCommand* cc = &cli_commands_internal[i];
            cli->print(CLI_NEWLINE " - internal command \'%s\'", cc->name);
            if (cc->help)
                cli->print(CLI_NEWLINE "%s", cc->help);
        }
    }
    // application
    for (uint16_t i = 0; i < cli->cmdcount; i++) {
        if (strstr(cli->cmdlist[i].name, chd->parsebuffer)) {
            CliCommand* cc = &cli->cmdlist[i];
            cli->print(CLI_NEWLINE " - command \'%s\'", cc->name);
            if (cc->help)
                cli->print(CLI_NEWLINE "%s", cc->help);
        }
    }
    // set/get variables
    for (uint16_t i = 0; i < cli->varcount; i++) {
        if (strstr(cli->cmdlist[i].name, chd->parsebuffer)) {
            CliVariable* cv = &cli->varlist[i];
            switch (cv->type) {
            default:
                cli->print(CLI_NEWLINE " - unknown type variable \'%s\'", cv->name);
                break;
            case CLI_VARTYPE_FLOAT:
                cli->print(CLI_NEWLINE " - variable float \'%s\' (%f - %f)", cv->name,
                    cv->min.f32, cv->max.f32);
                break;
            case CLI_VARTYPE_S32:
                cli->print(CLI_NEWLINE " - variable int32 \'%s\' (%d - %d)", cv->name,
                    cv->min.s32, cv->max.s32);
                break;
            case CLI_VARTYPE_S16:
                cli->print(CLI_NEWLINE " - variable int16 \'%s\' (%d - %d)", cv->name,
                    cv->min.s32, cv->max.s32);
                break;
            case CLI_VARTYPE_S8:
                cli->print(CLI_NEWLINE " - variable int8 \'%s\' (%d - %d)", cv->name,
                    cv->min.s32, cv->max.s32);
                break;
            case CLI_VARTYPE_FLOAT_READONLY:
                cli->print(CLI_NEWLINE " - variable float \'%s\'", cv->name);
                break;
            case CLI_VARTYPE_S32_READONLY:
                cli->print(CLI_NEWLINE " - variable int32 \'%s\'", cv->name);
                break;
            case CLI_VARTYPE_S16_READONLY:
                cli->print(CLI_NEWLINE " - variable int16 \'%s\'", cv->name);
                break;
            case CLI_VARTYPE_S8_READONLY:
                cli->print(CLI_NEWLINE " - variable int8 \'%s\'", cv->name);
                break;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// on parse need to consume name and whitespace, so next one will get either arg
// or 0

static CliVariable* cliVariableFind(CliInst* cli)
{
    CliHandleData* chd = cli->chd;
    while ((chd->mainbuffer[chd->cursor_position] == ' ') || (chd->mainbuffer[chd->cursor_position] == '\t')) {
        chd->cursor_position++;
    }
    for (uint16_t i = 0; i < cli->varcount; i++) {
        uint16_t namelen = strlen(cli->varlist[i].name);
        if (strncmp(&chd->mainbuffer[chd->cursor_position], cli->varlist[i].name,
                namelen)
            == 0) {
            char next = chd->mainbuffer[chd->cursor_position + namelen];
            // whitespace - set variable
            // end string - get value
            if ((next == ' ') || (next = '\0')) {
                chd->cursor_position += namelen;
                return &cli->varlist[i];
            }
        }
    }
    return 0;
}

static void cch_set(CliInst* cli)
{
    CliHandleData* chd = cli->chd;
    CliVariable* cv = cliVariableFind(cli);
    if (cv) {
        switch (cv->type) {
        default:
            cli->print(CLI_NEWLINE "variables table error: bad type of var %s", cv->name);
            break;
        case CLI_VARTYPE_S8:
        case CLI_VARTYPE_S16:
        case CLI_VARTYPE_S32: {
            int val, vchars;
            int32_t num = sscanf(&chd->mainbuffer[chd->cursor_position], "%d%n",
                &val, &vchars);
            if (num == EOF) {
                // print value
                int32_t read = *(int32_t*)cv->addr;
                cli->print(CLI_NEWLINE "read %s = %d", cv->name, read);
            } else if (num == 1) {
                // write value
                int32_t read = *(int32_t*)cv->addr;
                *(int32_t*)cv->addr = val;
                cli->print(CLI_NEWLINE "write %s = %d (was %d)", cv->name, val, read);
            } else {
                cli->print(CLI_NEWLINE "bad input");
                cliNewCommand(cli);
            }
        } break;
            /*
            case CLI_VARTYPE_FLOAT: {
              float val;
              int vchars;
              int32_t num =
                  sscanf(&chd->mainbuffer[chd->cursor_position], "%f%n", &val, &vchars);
              if (num == EOF) {
                // print value
                float read = *(float *)cv->addr;
                cli->print(CLI_NEWLINE "read %s = %f", cv->name, read);
              } else if (num == 1) {
                // write value
                float read = *(float *)cv->addr;
                *(float *)cv->addr = val;
                cli->print(CLI_NEWLINE "write %s = %f (was %f)", cv->name, val, read);
              } else {
                cliError(cli, "bad input");
              }
            } break;
            */
            // TODO: terminate processing, return to prompt
        }
    }
}
static void cch_get(CliInst* cli) { cli->print("soon it will work"); }

#endif // _CLI_CMD_INTERNAL_H