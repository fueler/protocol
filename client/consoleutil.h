/**
 * @author Wayne Moorefield
 * @brief This file contains basic console input
 *        management.
 */

#ifndef _CONSOLEUTIL_H
#define _CONSOLEUTIL_H

#include <stdarg.h>
#include "types.h"

#define CONSOLE_MAX_INPUT   64

enum {
    CLEAR_LINE_ON_ENTER = 0x01
};

bool ConsoleInit(U32 flags);
bool ConsoleHandleInput();
int ConsoleFlushQueueToBuffer(char *buffer, U32 maxlen);
void ConsolePrintf(const char* format, ...);

#endif

