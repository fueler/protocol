/**
 * @author Wayne Moorefield
 * @brief Client Socket
 */

#ifndef _CLIENTSOCKET_H
#define _CLIENTSOCKET_H

#include "types.h"
#include "SDL_net.h"

typedef UDPpacket ClientPacket;

struct ClientSocket
{
    U32 mBufferSize;
    UDPsocket mClientSocket;
    IPaddress mServerIPaddress;

    bool init(U32 localport, U32 bufferSize, IPaddress *server);
    void shutdown();

    bool receiveData(ClientPacket *pkt);
    bool transmitData(ClientPacket *pkt);

    ClientPacket* allocPacket();
    void freePacket(ClientPacket *pkt);

    bool toIPaddress(IPaddress *address, char *name, int port);
};

#endif

