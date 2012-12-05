/**
 * @author Wayne Moorefield
 * @brief This file contains basic console input
 *        management.
 */

#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "consoleutil.h"

static bool processAndQueueInput(char key);
static void processBackspace(U32 spaces);
static void processLineEnter();
static void processLineEscape();
static void processKey(char key);


/**
 * @brief Maintains the console buffer
 *
 */
struct ConsoleBuffer
{
    U32 length;
    char buffer[CONSOLE_MAX_INPUT];

    /**
     * @brief clears the buffer
     *
     */
    void clear()
    {
        buffer[0] = '\0';
        length = 0;
    }


    /**
     * @brief adds a character to the buffer
     * @param key character to add
     * @return true if success, otherwise error
     */
    bool addchar(char key)
    {
        // verify room available in buffer
        if (length < (CONSOLE_MAX_INPUT - 1)) {
            buffer[length] = key;
            ++length;
            buffer[length] = '\0';

            return true;
        } else {
            return false;
        }
    }


    /**
     * @brief removes a character from the buffer
     * @return true if success, otherwise error
     */
    bool delchar()
    {
        // verify that buffer isn't already empty
        if (length > 0) {
            --length;
            buffer[length] = '\0';

            return true;
        } else {
            return false;
        }
    }
};

static ConsoleBuffer gConsoleBuffer;


/**
 * @brief Initializes the console utility
 * @return true if success, otherwise error
 */
bool ConsoleInit()
{
    gConsoleBuffer.clear();

    return true;
}


/**
 * @brief Handles input by first checking to see
 *        if input is available then processing it.
 * @return true if user is tying, otherwise false
 */
bool ConsoleHandleInput()
{
    static bool userTyping = false;
    char key;

    // check to see if user has pressed a key
    if (kbhit()) {
        // user pressed a key, get it
        key = getch();
        userTyping = processAndQueueInput(key);
    }

    return userTyping;
}


/**
 * @brief Copies console buffer in to buffer passed by caller
 *        up to console buffer length or maxlen. Then flushes
 *        console buffer.
 * @param buffer location to store console buffer
 * @param maxlen length of buffer
 * @return length of string copied, 0 for no data
 */
int ConsoleFlushQueueToBuffer(char *buffer, U32 maxlen)
{
    int bufferLen = 0;

    // validate arguments
    if (!buffer) {
        // invalid buffer
        return -1;
    }

    bufferLen = gConsoleBuffer.length < (maxlen-1) ? gConsoleBuffer.length : (maxlen-1);

    memcpy(buffer, gConsoleBuffer.buffer, bufferLen);
    buffer[bufferLen] = '\0'; // ensure null terminated string

    // clear the buffer for the next line
    gConsoleBuffer.clear();

    return bufferLen;
}


/**
 * @brief Processes the new key and then queues it in the
 *        console buffer if needed.
 * @param key new key pressed
 * @return true if user is typing, otherwise false
 */
bool processAndQueueInput(char key)
{
    bool userTyping = true;

    // Based on what key was pressed do
    // different things.
    // www.asciitable.com for table
    switch (key) {
    case 0x08: // Backspace
        processBackspace(1);
        break;
    case 0x0B: // Line Feed
    case 0x0D: // Carriage return
        processLineEnter();
        userTyping = false;
        break;
    case 0x1B: // ESC
        processLineEscape();
        userTyping = false;
        break;
    case 0x7F: // DEL
        break;
    default:
        // space to ~
        if ((key >= 0x20) && (key <= 0x126)) {
            processKey(key);
        }
        break;
    }

    return userTyping;
}


/**
 * @brief Processes backspaces
 * @param number of times to backspace
 */
void processBackspace(U32 spaces)
{
    for (U32 i=0; i<spaces; ++i) {
        // 1. move back a space
        // 2. replace char with a space
        // 3. move back a space
        if (gConsoleBuffer.delchar()) {
            printf("%c %c", 0x08, 0x08);
        }
    }
}


/**
 * @brief Processes line enter, user is finished
 *        entering in text
 */
void processLineEnter()
{
    // Do the same thing as a backspace
    // but don't actually delete the char
    // from the buffer
    for (U32 i=0; i<gConsoleBuffer.length; ++i) {
        printf("%c %c", 0x08, 0x08);
    }
}


/**
 * @brief Processes the user pressing ESC on the
 *        keyboard. This causes all text to be
 *        cancelled.
 */
void processLineEscape()
{
    processBackspace(gConsoleBuffer.length);
}


/**
 * @brief Processes a normal key, if buffer has
 *        room to queue then prints it to the screen
 *        as well.
 * @param key new normal key
 */
void processKey(char key)
{
    if (gConsoleBuffer.addchar(key)) {
        printf("%c", key);
        //printf("[%x]", key);
    }
}

