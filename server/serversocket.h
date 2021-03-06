/**
 * @author Wayne Moorefield
 * @brief Server Socket
 */

#ifndef _SERVERSOCKET_H
#define _SERVERSOCKET_H

#include "types.h"
#include "SDL_net.h"

typedef UDPpacket ServerPacket;

struct ClientConn
{
    bool used;
    IPaddress address;
    void *appData;

    void clear() {
        used = false;
        address.host = 0;
        address.port = 0;
        appData = NULL;
    }
};

struct ServerSocket
{
    U32 mPort;
    U32 mBufferSize;
    U32 mMaxClients;

    IPaddress mServerIP;
    UDPsocket mServerSocket;

    ClientConn *mClientList;
    U32 mClientCount;

    bool init(U32 port, U32 bufferSize, U32 maxClients);
    void shutdown();

    bool receiveData(ServerPacket *pkt);
    bool transmitData(int toHandle, ServerPacket *pkt);

    IPaddress* handleToPeerIPaddress(U32 handle);
    int peerIPaddressToHandle(IPaddress *address);

    ServerPacket* allocPacket();
    void freePacket(ServerPacket *pkt);

    int allocClient(IPaddress *address);
    void freeClient(U32 handle);

    void setPrivateData(U32 handle, void *ptr);
    void* getPrivateData(U32 handle);

    IPaddress* getLocalServerIP();
};

#endif
