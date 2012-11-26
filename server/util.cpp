/**
 * @author Wayne Moorefield
 * @brief This file contains basic data types
 */

#include <stdio.h>
#include "util.h"

/**
 * @brief Dumps memory
 * @param bufPtr pointer to buffer to dump
 * @param length length of buffer
 * @param offset offset to start in the buffer
 */
void debugDumpMemoryContents(const U8* bufPtr, U32 length, U32 offset)
{
    int alignment = 16;
    U8* first = (U8*)&bufPtr[offset];
    U8* last = (U8*)&bufPtr[length];

    // Align memory for debug printing
    first = (U8*)((U32)first & (~(alignment-1)));
    last = (U8*)(((U32)last + (alignment-1)) & (~(alignment-1)));

    // Print memory dump header
    printf("Memory (0x%08x-0x%08x)\n", (U32)first, (U32)last);
    if (bufPtr == NULL) {
        printf("\tInvalid buffer pointer\n");
    }

    // Print memory dump
    for (U8 *addr=first; addr<last; addr+= alignment) {
        printf("0x%08x", (U32)addr);
        for (int i=0; i<alignment; ++i) {
            if ((i&(alignment/2 - 1)) == 0) {
                printf(" ");
            }

            if ((&addr[i] < bufPtr) || (&addr[i] >= &bufPtr[length])) {
                printf(".. ");
            } else {
                // valid address
                printf("%02x ", addr[i]);
            }
        }

        for (int i=0; i<alignment; ++i) {
            if ((i&(alignment/2 - 1)) == 0) {
                printf(" ");
            }

            if ((&addr[i] < bufPtr) || (&addr[i] >= &bufPtr[length])) {
                printf(".");
            } else {
                // valid address
                if ((addr[i] >= 32) && (addr[i] < 127)) {
                    printf("%c", addr[i]);
                } else if ((addr[i] >= 129) && (addr[i] < 255)) {
                    printf("%c", addr[i]);
                } else {
                    printf(".");
                }
            }
        }

        printf("\n");
    }
}
