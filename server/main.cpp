#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "SDL_net.h"
#include "types.h"
#include "util.h"

#define UDP_SOCKET_PORT 2000
#define UDP_MAX_PACKET_SIZE 512

int main(int argc, char **argv)
{
    UDPsocket sd;
    UDPpacket *pkt;
    bool quit;

    // Initialize SDL_net
    if (SDLNet_Init() < 0) {
        printf("ERROR: SDLNet_Init: %s\n",
               SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    // Open a socket
    sd = SDLNet_UDP_Open(UDP_SOCKET_PORT);
    if (sd == 0) {
        printf("ERROR: SDLNet_UDP_Open(%d): %s\n",
               UDP_SOCKET_PORT,
               SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    // Make space for the packet
    pkt = SDLNet_AllocPacket(UDP_MAX_PACKET_SIZE);
    if (pkt == NULL) {
        printf("ERROR: SDLNet_AllocPacket(%d): %s\n",
               UDP_MAX_PACKET_SIZE,
               SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    printf("Ready to receive packets\n");

    // Main loop
    quit = false;
    while (!quit) {
        // Wait a packet. UDP_Recv returns != 0 if a packet is coming
        if (SDLNet_UDP_Recv(sd, pkt)) {
            printf("UDP Packet incoming\n");
            printf("\tChan:    %d\n", pkt->channel);
            printf("\tLen:     %d\n", pkt->len);
            printf("\tMaxlen:  %d\n", pkt->maxlen);
            printf("\tStatus:  %d\n", pkt->status);
            printf("\tAddress: %x %x\n", pkt->address.host, pkt->address.port);
            debugDumpMemoryContents(pkt->data, pkt->len);

            // Quit if packet contains "quit"
            if (strcmp((char *)pkt->data, "quit") == 0) {
                quit = true;
            }
        }
    }

    // Clean and exit
    SDLNet_FreePacket(pkt);
    SDLNet_Quit();

    return EXIT_SUCCESS;
}
