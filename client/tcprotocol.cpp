/**
 * @author Your Name Here
 * @brief This file contains the TC Protocol
 *        implementation.
 */

#include "tcprotocol.h"
#include "consoleutil.h"
#include "util.h"

// By commenting out these defines it turns off
// debugs. Likewise, uncommenting them out will
// turn them on.
#define DEBUG_SHOW_RAW_PACKET
//#define DEBUG_SHOW_USER_INPUT


/**
 * @brief This function handles input from the user
 * @return true if keep going, otherwise exit the program
 */
bool HandleUserInput()
{
    char buffer[CONSOLE_MAX_INPUT];
    int length;
    bool keepGoing = true;

    length = ConsoleFlushQueueToBuffer(buffer, CONSOLE_MAX_INPUT);

    if (length > 0) {
#ifdef DEBUG_SHOW_USER_INPUT
        printf("USERINPUT: [%s]\n", buffer);
#endif

        // Process commands from user

        // "/join" format: name(8)

        // "/leave" format: name(8)

        // "/quit"
        if (!strcmp(buffer, "/quit")) {
            // user wants to quit
            keepGoing = false;

            // cleanup
        }

        // text format: name(8) text(128)
    }

    return keepGoing;
}


/**
 * @brief This function handles data from the server
 * @param client pointer to client socket
 * @param pkt pointer to packet received from server
 * @return true to keep going, otherwise, quit the program
 */
bool HandleServerData(ClientSocket *client, ClientPacket *pkt)
{
#ifdef DEBUG_SHOW_RAW_PACKET
    // Debug
    printf("UDP Packet incoming\n");
    printf("\tChannel: %d\n", pkt->channel);
    printf("\tLength:  %d\n", pkt->len);
    printf("\tMaxlen:  %d\n", pkt->maxlen);
    printf("\tStatus:  %d\n", pkt->status);

    // Host and Port are in network order
    printf("\tAddress: %d.%d.%d.%d:%d\n",
           (pkt->address.host >>  0) & 0xFF,
           (pkt->address.host >>  8) & 0xFF,
           (pkt->address.host >> 16) & 0xFF,
           (pkt->address.host >> 24) & 0xFF,
           SDLNet_Read16(&pkt->address.port));
    debugDumpMemoryContents(pkt->data, pkt->len);
#endif

    bool keepGoing = true;

    // message from server


    return keepGoing;
}

