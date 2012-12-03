#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "SDL_net.h"
#include "types.h"
#include "util.h"
#include "serversocket.h"
#include "tcprotocol.h"

#define UDP_SOCKET_PORT 2000
#define UDP_MAX_PACKET_SIZE 512
#define MAX_CLIENTS 10

int main(int argc, char **argv)
{
    ServerSocket server;
    bool quit;

    // Initialize SDL_net
    if (SDLNet_Init() < 0) {
        printf("ERROR: SDLNet_Init: %s\n",
               SDLNet_GetError());
        exit(EXIT_FAILURE);
    }
    printf("SDLNet initialized\n");


    // SDL by default redirects output to console
    // to files, this puts it back to the console
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);

    // Initialize server
    if (!server.init(UDP_SOCKET_PORT, UDP_MAX_PACKET_SIZE, MAX_CLIENTS)) {
        printf("ERROR: Unable to init server\n");
        exit(EXIT_FAILURE);
    }

    printf("Ready to receive packets\n");

    // Main loop
    quit = false;
    while (!quit) {
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
    }

    // Clean up and exit
    server.shutdown();
    SDLNet_Quit();

    return EXIT_SUCCESS;
}
