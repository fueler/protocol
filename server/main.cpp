#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "SDL_net.h"
#include "types.h"
#include "util.h"
#include "serversocket.h"
#include "tcprotocol.h"
#include "servercfg.h"
#include "consoleutil.h"


int main(int argc, char **argv)
{
    ServerSocket server;
    bool quit;
    bool obtainingInput = false;

    // Initialize the console
    if (!ConsoleInit(CLEAR_LINE_ON_ENTER)) {
        printf("ERROR: ConsoleInit failed\n");
        exit(EXIT_FAILURE);
    }

    // Initialize SDL_net
    if (SDLNet_Init() < 0) {
        ConsolePrintf("ERROR: SDLNet_Init: %s\n",
                      SDLNet_GetError());
        exit(EXIT_FAILURE);
    }
    ConsolePrintf("SDLNet initialized\n");

    // Initialize server
    if (!server.init(UDP_SOCKET_PORT, UDP_MAX_PACKET_SIZE, MAX_CLIENTS)) {
        ConsolePrintf("ERROR: Unable to init server\n");
        exit(EXIT_FAILURE);
    }

    // Host and Port are in network order
    IPaddress *srvadd = server.getLocalServerIP();
    ConsolePrintf("Server Started: %d.%d.%d.%d:%d\n",
                  (srvadd->host >>  0) & 0xFF,
                  (srvadd->host >>  8) & 0xFF,
                  (srvadd->host >> 16) & 0xFF,
                  (srvadd->host >> 24) & 0xFF,
                  SDLNet_Read16(&srvadd->port));

    // Initialize the Messenger protocol
    if (!InitMessengerProtocol(&server)) {
        ConsolePrintf("ERROR: InitMessengerProtocol() failed\n");
        exit(EXIT_FAILURE);
    }
    ConsolePrintf("Ready to receive packets\n");

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
                if (!HandleUserInput(&server)) {
                    // time to quit
                    quit = true;
                }
            }

            obtainingInput = false;
        }

        // get network input
        if (!quit) {
            ServerPacket *pkt;

            pkt = server.allocPacket();
            if (pkt) {
                if (server.receiveData(pkt)) {
                    // handle data
                    if (!HandleClientData(&server, pkt)) {
                        quit = true;
                    }
                }

                // finish with the packet, free it
                server.freePacket(pkt);
            }
        } // end network
	}

    // Clean up and exit
    ShutdownMessengerProtocol(&server);
    server.shutdown();
    SDLNet_Quit();

    return EXIT_SUCCESS;
}
