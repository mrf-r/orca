// simple static CLI
// Evgeny Chernih
// 2022 Triton

/*
использование:
для работы необходимо просто добавить в проект:
cli.c/cpp cli.h cli_cmd_internal.h
в cli_cmd_internal.h при необходимости исправить комманды сброса, GIT

в проекте необходима функция вывода типа void printf(char*, ...) для потока командной строки
в проекте создать массив комманд CliCommand и массив переменных CliVariable
в озу создать объект контекста CliHandleData
создать инстанс CliInst с указанными ранее объектами
в обработчике символа вызывать cliParseChar(CliInst*, char)

в основном списке комманд должны содержаться указатели на функции типа
void command(char*, void (*const p)(const char *, ...))
первым аргументом приходит введенная после команды строка,
вторым аргументом приходит указатель на функцию типа void printf(char*, ...)
соответственно парсинг аргументов индивидуален для каждой команды

*/

#include "cli.h"
#include "cli_cmd_internal.h"

#include <stdarg.h>

#define ASSERT(...)

#define CLI_STATUS_ESCAPE 0x01
#define CLI_STATUS_ESCAPEANSI 0x02

#define CLI_STR_CSI "\x1B["

// CliInst monitor;

static inline void cliStatusSet(CliInst* cli, uint32_t status)
{
    cli->chd->status |= status;
}

static inline void cliStatusClear(CliInst* cli, uint32_t status)
{
    cli->chd->status &= ~status;
}

static void cliRedrawString(CliInst* cli, uint16_t from_pos)
{
    CliHandleData* chd = cli->chd;
    ASSERT(from_pos <= strlen(chd->mainbuffer));
    uint16_t startpos = strlen(cli->prompt);
    if (from_pos == 0) {
        cli->print(CLI_STR_CSI "0G");
        cli->print(cli->prompt);
        cli->print(chd->mainbuffer);
        for (uint16_t i = strlen(chd->mainbuffer); i < CLI_MAXINPUTSTRINGLENGTH; i++) {
            cli->print(" ");
        }
    } else {
        cli->print(CLI_STR_CSI "%dG", startpos + from_pos);
        cli->print(&chd->mainbuffer[chd->cursor_position - 1]);
        cli->print("  "); // in case of delete
    }
    cli->print(CLI_STR_CSI "%dG", startpos + chd->cursor_position + 1);
}

static void cliNewCommand(CliInst* cli)
{
    CliHandleData* chd = cli->chd;
    chd->status = 0;
    chd->cursor_position = 0;
    chd->mainbuffer[0] = '\0';
    chd->historypos = CLI_HISTORYDEPTH - 1;
    cli->print(CLI_NEWLINE);
    cli->print(cli->prompt);
}

static void cliInsertChar(CliInst* cli, char ch)
{
    CliHandleData* chd = cli->chd;
    for (uint16_t i = CLI_MAXINPUTSTRINGLENGTH - 1; i > chd->cursor_position;
         i--) {
        chd->mainbuffer[i] = chd->mainbuffer[i - 1];
    }
    chd->mainbuffer[chd->cursor_position] = ch;
    chd->mainbuffer[CLI_MAXINPUTSTRINGLENGTH - 1] = '\0';
    chd->cursor_position++;
    if (chd->cursor_position >= CLI_MAXINPUTSTRINGLENGTH - 1) {
        cli->print(CLI_NEWLINE "input buffer limit reached" CLI_NEWLINE);
        cliRedrawString(cli, 0);
    } else {
        cliRedrawString(cli, chd->cursor_position);
    }
}

static void cliRemoveChar(CliInst* cli)
{
    CliHandleData* chd = cli->chd;
    chd->mainbuffer[CLI_MAXINPUTSTRINGLENGTH - 1] = '\0';
    for (uint16_t i = chd->cursor_position; i <= CLI_MAXINPUTSTRINGLENGTH - 2;
         i++) {
        chd->mainbuffer[i] = chd->mainbuffer[i + 1];
    }
    cliRedrawString(cli, chd->cursor_position);
}

// save successfully parsed part only (till cursor position)
static void cliHistorySave(CliInst* cli)
{
    CliHandleData* chd = cli->chd;
    // slightly dumb
    for (uint16_t i = 0; i > CLI_HISTORYDEPTH; i++) {
        if (strcmp(chd->historybuffer[i], chd->mainbuffer)) {
            return;
        }
    }
    for (uint16_t i = CLI_HISTORYDEPTH - 1; i > 0; i--) {
        strcpy(chd->historybuffer[i], chd->historybuffer[i - 1]);
    }
    strcpy(chd->historybuffer[0], chd->mainbuffer);
}

static void cliHistoryScroll(CliInst* cli, uint8_t dir_up)
{
    CliHandleData* chd = cli->chd;
    if (dir_up) {
        if (chd->historypos < CLI_HISTORYDEPTH - 1) {
            chd->historypos++;
        } else {
            chd->historypos = 0;
        }
    } else {
        if (chd->historypos) {
            chd->historypos--;
        } else {
            chd->historypos = CLI_HISTORYDEPTH - 1;
        }
    }
    strcpy(chd->mainbuffer, chd->historybuffer[chd->historypos]);
    chd->cursor_position = strlen(chd->mainbuffer);
    cliRedrawString(cli, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cliProcessCommand(CliInst* cli)
{
    CliHandleData* chd = cli->chd;
    int charcount = strlen(chd->mainbuffer);
    if (sscanf(chd->mainbuffer, "%s%n", chd->parsebuffer, &charcount) == 1) {
        cliHistorySave(cli);
        chd->cursor_position = charcount;
        // internal
        for (uint16_t i = 0; i < cli_commands_internal_count; i++) {
            if (strcmp(chd->parsebuffer, cli_commands_internal[i].name) == 0) {
                CliCommand* cc = &cli_commands_internal[i];
                if (cc->hdl) {
                    cc->hdl(&chd->mainbuffer[chd->cursor_position], cli->print);
                    chd->cursor_position += strlen(&chd->mainbuffer[chd->cursor_position]);
                } else {
                    cc->hdl_ex(cli);
                }
                cliNewCommand(cli);
                return;
            }
        }
        // application
        for (uint16_t i = 0; i < cli->cmdcount; i++) {
            if (strcmp(chd->parsebuffer, cli->cmdlist[i].name) == 0) {
                CliCommand* cc = &cli->cmdlist[i];
                if (cc->hdl) {
                    cc->hdl(&chd->mainbuffer[chd->cursor_position], cli->print);
                    chd->cursor_position += strlen(&chd->mainbuffer[chd->cursor_position]);
                } else {
                    cc->hdl_ex(cli);
                }
                cliNewCommand(cli);
                return;
            }
        }
        // set/get variables
        // TODO add variable processing here
        cli->print(CLI_NEWLINE "unknown: %s", chd->mainbuffer);
    }
    cliNewCommand(cli);
}
/*
Esc[x~
HOME - 1
INS - 2
DEL - 3
END - 4
F1 - 11

нужно сделать нормальный парсер через sscanf???????
Esc - переключает буфер и ждет
*/
void cliParseChar(CliInst* cli, char ch)
{
    CliHandleData* chd = cli->chd;
    if (chd->status & CLI_STATUS_ESCAPE) {
        // escape ************************************************************
        if (ch == '[') {
            cliStatusSet(cli, CLI_STATUS_ESCAPEANSI);
            chd->csipos = 0;
        } else if (ch == '\x1b') {
            // double Ecs will reset input buffer
            cli->print(CLI_STR_CSI "2J"); // clear
            cli->print(CLI_STR_CSI "H"); // home
            chd->cursor_position = 0;
            chd->mainbuffer[0] = '\0';
            cli->print(cli->prompt);
        }
        cliStatusClear(cli, CLI_STATUS_ESCAPE);
    } else if (chd->status & CLI_STATUS_ESCAPEANSI) {
        // CSI ************************************************************
        if (ch == 'A') { // cursor up
            cliHistoryScroll(cli, 1);
        } else if (ch == 'B') { // cursor down
            cliHistoryScroll(cli, 0);
        } else if (ch == 'C') { // cursor right
            uint16_t ml = strlen(chd->mainbuffer);
            if (chd->cursor_position < ml) {
                chd->cursor_position++;
                cli->print("\x1B[C");
            }
        } else if (ch == 'D') { // cursor left
            if (chd->cursor_position) {
                chd->cursor_position--;
                cli->print("\x1B[D");
            }
        } else if ((ch >= '0') && (ch <= '9')) {
            if (chd->csipos < CLI_CSILENGTH - 1) {
                chd->csibuf[chd->csipos++] = ch;
                return; // keep status
            } else {
                chd->status = 0;
                // cli->print(CLI_NEWLINE "unsupported control sequence");
                // cliNewCommand(cli);
            }
        } else if (ch == '~') { // f1 or delete
            // convert
            int ccode = 0;
            chd->csibuf[chd->csipos] = '\0';
            sscanf(chd->csibuf, "%d", &ccode);
            switch (ccode) {
            // ************************************************************
            case 1: // HOME
                chd->cursor_position = 0;
                cliRedrawString(cli, 0);
                break;
            case 4: // END
                chd->cursor_position = strlen(chd->mainbuffer);
                cliRedrawString(cli, 0);
                break;
            case 3: // DELETE
                if (chd->cursor_position < strlen(chd->mainbuffer)) {
                    cliRemoveChar(cli);
                }
                break;
            case 11: // HELP
                cli->print(CLI_NEWLINE "help \'%s\' not yet ready", chd->mainbuffer);
                // TODO call help with current buffer arg
                // like it was "help <buffer>"

                break;
            default:
                // do nothing
                // cliError(cli, "unknown control sequence");
                ;
            }
        }
        cliStatusClear(cli, CLI_STATUS_ESCAPEANSI);
        // 1~ - HOME
        // 3~ - DELETE
        // 4~ - END
        // 11~ - F1 (help)

    } else {
        // regular ************************************************************
        if (ch > '\x7F') { // prohibit ASCII > 127
            cli->print(CLI_NEWLINE "only supports english");
            cliNewCommand(cli);
        } else if (ch == '\x1B') { // Esc
            chd->csipos = 0;
            cliStatusSet(cli, CLI_STATUS_ESCAPE);
        } else if (ch == '\x7F') { // backspace
            if (chd->cursor_position) {
                chd->cursor_position--;
                cliRemoveChar(cli);
            }
        } else if ((ch >= ' ') && (ch < '\x7F')) { // input
            cliInsertChar(cli, ch);
        } else if (ch == '\n') { // process input string
            cliProcessCommand(cli);
        }
    }
}

/*
TODO:
special variable names:
int8 0xAAAABBBB ?
int16
int32
float

additional commands for debug
add
rem
list
rate x (ms)
*/