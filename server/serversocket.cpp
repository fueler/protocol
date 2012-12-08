/**
 * @author Wayne Moorefield
 * @brief Server Socket
 */

#include "serversocket.h"
#include "consoleutil.h"
#include "util.h"

// By commenting out these defines it turns off
// debugs. Likewise, uncommenting them out will
// turn them on.
//#define DEBUG_SHOW_RAW_RX_PACKET
//#define DEBUG_SHOW_RAW_TX_PACKET


bool ServerSocket::init(U32 port, U32 bufferSize, U32 maxClients)
{
    bool retval = true; // default success

    mPort = port;
    mBufferSize = bufferSize;
    mMaxClients = maxClients;

    mClientCount = 0;
    mServerSocket = 0;

    mClientList = new ClientConn[mMaxClients];
    if (mClientList == NULL) {
        retval = false;
        ConsolePrintf("ERROR: Unable to allocate client list %d\n",
                      mMaxClients);
    } else {
        // init client list
        for (U32 i=0; i<mMaxClients; ++i) {
            mClientList[i].clear();
        }
    }

    // Get the server's IP address
    if (retval) {
        int hostResolved = SDLNet_ResolveHost(&mServerIP, NULL, mPort);

        if (hostResolved == -1) {
            retval = false;
            ConsolePrintf("ERROR: SDLNet_ResolveHost(%d): %s\n",
                          mPort,
                          SDLNet_GetError());
        }
    }

    if (retval) {
        mServerSocket = SDLNet_UDP_Open(mPort);
        if (mServerSocket == 0) {
            retval = false;
            ConsolePrintf("ERROR: SDLNet_UDP_Open(%d): %s\n",
                          mPort,
                          SDLNet_GetError());
        }
    }

    return retval;
}

void ServerSocket::shutdown()
{
    if (mClientList) {
        delete [] mClientList;
    }

    if (mServerSocket) {
        SDLNet_UDP_Close(mServerSocket);
    }
}

ServerPacket* ServerSocket::allocPacket()
{
    ServerPacket *pkt;

    pkt = SDLNet_AllocPacket(mBufferSize);
    if (pkt == NULL) {
        ConsolePrintf("ERROR: SDLNet_AllocPacket(%d): %s\n",
                      mBufferSize,
                      SDLNet_GetError());
    }

    return pkt;
}

void ServerSocket::freePacket(ServerPacket *pkt)
{
    if (pkt) {
        SDLNet_FreePacket(pkt);
    }
}

int ServerSocket::allocClient(IPaddress *address)
{
    U32 clientIndex = 0;

    if (address == NULL) {
        return -1;
    }

    // find empty slot
    for (clientIndex=0; clientIndex<mMaxClients; ++clientIndex) {
        if (!mClientList[clientIndex].used) {
            // found empty slot
            break;
        }
    }

    if (clientIndex >= mMaxClients) {
        // too many clients
        return -1;
    }

    ++mClientCount;
    mClientList[clientIndex].used = true;
    mClientList[clientIndex].address = *address;

    return clientIndex;
}

void ServerSocket::freeClient(U32 handle)
{
    if (handle >= mMaxClients) {
        // invalid handle
        return;
    }

    if (mClientList[handle].used) {
        mClientList[handle].clear();
        --mClientCount;
    }
}

void ServerSocket::setPrivateData(U32 handle, void *ptr)
{
    if (handle >= mMaxClients) {
        // invalid handle
        return;
    } else if (mClientList[handle].used) {
        mClientList[handle].appData = ptr;
    } else {
        // invalid handle
        return;
    }
}

void* ServerSocket::getPrivateData(U32 handle)
{
    if (handle >= mMaxClients) {
        // invalid handle
        return NULL;
    } else if (mClientList[handle].used) {
        return mClientList[handle].appData;
    } else {
        // invalid handle
        return NULL;
    }
}

bool ServerSocket::receiveData(ServerPacket *pkt)
{
    int numPkts;

    if (pkt == NULL) {
        return false;
    }

    numPkts = SDLNet_UDP_Recv(mServerSocket, pkt);
    if (numPkts > 0) {
#ifdef DEBUG_SHOW_RAW_RX_PACKET
        ConsolePrintf("<---- UDP Packet Received\n");
        ConsolePrintf("\tChannel: %d\n", pkt->channel);
        ConsolePrintf("\tLength:  %d\n", pkt->len);
        ConsolePrintf("\tMaxlen:  %d\n", pkt->maxlen);
        ConsolePrintf("\tStatus:  %d\n", pkt->status);

        // Host and Port are in network order
        ConsolePrintf("\tAddress: %d.%d.%d.%d:%d\n",
                      (pkt->address.host >>  0) & 0xFF,
                      (pkt->address.host >>  8) & 0xFF,
                      (pkt->address.host >> 16) & 0xFF,
                      (pkt->address.host >> 24) & 0xFF,
                      pkt->address.port);
        debugDumpMemoryContents(pkt->data, pkt->len);
#endif
        return true;
    } else if (numPkts < 0) {
        ConsolePrintf("ERROR: SDLNet_UDP_Recv(): %s\n",
                      SDLNet_GetError());
        return false;
    } else {
        // no data
        return false;
    }
}

bool ServerSocket::transmitData(int toHandle, ServerPacket *pkt)
{
    int numRecepients;
    IPaddress *toAddress;

    if (pkt == NULL) {
        return false;
    }

    toAddress = handleToPeerIPaddress(toHandle);
    if (toAddress) {
        pkt->address = *toAddress;

#ifdef DEBUG_SHOW_RAW_TX_PACKET
        ConsolePrintf("----> UDP Packet Transmitted\n");
        ConsolePrintf("\tChannel: %d\n", pkt->channel);
        ConsolePrintf("\tLength:  %d\n", pkt->len);
        ConsolePrintf("\tMaxlen:  %d\n", pkt->maxlen);
        ConsolePrintf("\tStatus:  %d\n", pkt->status);

        // Host and Port are in network order
        ConsolePrintf("\tAddress: %d.%d.%d.%d:%d\n",
                      (pkt->address.host >>  0) & 0xFF,
                      (pkt->address.host >>  8) & 0xFF,
                      (pkt->address.host >> 16) & 0xFF,
                      (pkt->address.host >> 24) & 0xFF,
                      pkt->address.port);
        debugDumpMemoryContents(pkt->data, pkt->len);
#endif

        numRecepients = SDLNet_UDP_Send(mServerSocket, -1, pkt);
        if (numRecepients == 0) {
            // unable to send
            return false;
        } else {
            // success
            return true;
        }
    } else {
        // invalid handle
        return false;
    }
}

IPaddress* ServerSocket::handleToPeerIPaddress(U32 handle)
{
    if (handle >= mMaxClients) {
        // invalid handle
        return NULL;
    }

    return &mClientList[handle].address;
}

int ServerSocket::peerIPaddressToHandle(IPaddress *address)
{
    int handle = -1;

    for (U32 clientIndex=0; clientIndex<mMaxClients; ++clientIndex) {
        if (mClientList[clientIndex].address.host == address->host) {
            if (mClientList[clientIndex].address.port == address->port) {
                // found match
                handle = clientIndex;
                break;
            }
        }
    }

    return handle;
}

IPaddress* ServerSocket::getLocalServerIP()
{
    return &mServerIP;
}
