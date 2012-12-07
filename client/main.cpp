/**
 * @author Wayne Moorefield
 * @brief This file contains the client entry point
 *        and sets up the client socket.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL_net.h"
#include "types.h"
#include "util.h"
#include "clientsocket.h"
#include "consoleutil.h"
#include "tcprotocol.h"

#define UDP_MAX_PACKET_SIZE 512
#define USE_RANDOM_PORT 0


/**
 * @brief client program entry point
 * @param argc number of arguments
 * @param argv pointer to array of strings
 * @return return value of program
 */
int main(int argc, char **argv)
{
    ClientSocket client;
	IPaddress srvadd;
	bool quit;
    bool obtainingInput = false;

    if (!ConsoleInit(CLEAR_LINE_ON_ENTER)) {
        printf("ERROR: ConsoleInit failed\n");
        exit(EXIT_FAILURE);
    }

	// Check for parameters
	if (argc < 3) {
		ConsolePrintf("ERROR: Usage: %s host port\n",
                      argv[0]);
		exit(EXIT_FAILURE);
	}

	// Initialize SDL_net
	if (SDLNet_Init() != 0) {
		ConsolePrintf("ERROR: SDLNet_Init: %s\n",
                      SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
    ConsolePrintf("SDLNet Ready\n");

	// Resolve server name
	if (!client.toIPaddress(&srvadd, argv[1], atoi(argv[2])))
	{
		ConsolePrintf("ERROR: SDLNet_ResolveHost(%s:%d): %s\n",
                      argv[1],
                      atoi(argv[2]),
                      SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

    // Host and Port are in network order
    ConsolePrintf("Server: %d.%d.%d.%d:%d\n",
                  (srvadd.host >>  0) & 0xFF,
                  (srvadd.host >>  8) & 0xFF,
                  (srvadd.host >> 16) & 0xFF,
                  (srvadd.host >> 24) & 0xFF,
                  SDLNet_Read16(&srvadd.port));

    // Initialize client
    if (!client.init(USE_RANDOM_PORT, UDP_MAX_PACKET_SIZE, &srvadd)) {
        ConsolePrintf("ERROR: client.init(): failed\n");
        exit(EXIT_FAILURE);
    }
    ConsolePrintf("Client Ready\n");

    // Initialize the Messenger protocol
    if (!InitMessengerProtocol()) {
        ConsolePrintf("ERROR: InitMessengerProtocol() failed\n");
        exit(EXIT_FAILURE);
    }
    ConsolePrintf("Messenger Protocol Ready\n");

	// Main loop
	quit = false;
	while (!quit) {
        // get input
        if (ConsoleHandleInput()) {
            // user is typing something
            obtainingInput = true;
        } else {
            if (obtainingInput) {
                // user finished typing something
                if (!HandleUserInput(&client)) {
                    // time to quit
                    quit = true;
                }
            }

            obtainingInput = false;
        }

        // get network input
        if (!quit) {
            ClientPacket *pkt;

            pkt = client.allocPacket();
            if (pkt) {
                if (client.receiveData(pkt)) {
                    // handle data
                    if (!HandleServerData(&client, pkt)) {
                        quit = true;
                    }
                }

                // finish with the packet, free it
                client.freePacket(pkt);
            }
        } // end network
	}

    ConsolePrintf("Quiting...\n");

    // cleanup
    ShutdownMessengerProtocol();
    client.shutdown();
	SDLNet_Quit();

	return EXIT_SUCCESS;
}
