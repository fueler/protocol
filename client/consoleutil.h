/**
 * @author Wayne Moorefield
 * @brief This file contains basic console input
 *        management.
 */

#ifndef _CONSOLEUTIL_H
#define _CONSOLEUTIL_H

#include "types.h"

#define CONSOLE_MAX_INPUT   64

bool ConsoleInit();
bool ConsoleHandleInput();
int ConsoleFlushQueueToBuffer(char *buffer, U32 maxlen);

#endif

