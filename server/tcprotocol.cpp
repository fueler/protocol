
#include "tcprotocol.h"
#include "util.h"

bool HandleClientData(ServerSocket *server, ServerPacket *pkt)
{
    bool fatalError = false;
    U32 ip = pkt->address.host;

    // Debug
    printf("UDP Packet incoming\n");
    printf("\tChannel: %d\n", pkt->channel);
    printf("\tLength:  %d\n", pkt->len);
    printf("\tMaxlen:  %d\n", pkt->maxlen);
    printf("\tStatus:  %d\n", pkt->status);
    printf("\tAddress: %d.%d.%d.%d:%d\n",
           (ip >>  0) & 0xFF,
           (ip >>  8) & 0xFF,
           (ip >> 16) & 0xFF,
           (ip >> 24) & 0xFF,
           pkt->address.port);
    debugDumpMemoryContents(pkt->data, pkt->len);

    // DEBUG
    // Quit if packet contains "quit"
    if (strcmp((char *)pkt->data, "quit") == 0) {
        fatalError = true;
    }

    if (fatalError) {
        // error
        return false;
    }

    return true;
}
