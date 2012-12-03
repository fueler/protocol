/**
 * @author Wayne Moorefield
 * @brief Server Socket
 */

#include "serversocket.h"
#include "util.h"


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
        printf("ERROR: Unable to allocate client list %d\n",
               mMaxClients);
    } else {
        // init client list
        for (U32 i=0; i<mMaxClients; ++mMaxClients) {
            mClientList[i].clear();
        }
    }

    // Is this needed for UDP?
    if (retval) {
        int hostResolved = SDLNet_ResolveHost(&mServerIP, NULL, mPort);

        if (hostResolved == -1) {
            retval = false;
            printf("ERROR: SDLNet_ResolveHost(%d): %s\n",
                   mPort,
                   SDLNet_GetError());
        }
    }

    if (retval) {
        mServerSocket = SDLNet_UDP_Open(mPort);
        if (mServerSocket == 0) {
            retval = false;
            printf("ERROR: SDLNet_UDP_Open(%d): %s\n",
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
        printf("ERROR: SDLNet_AllocPacket(%d): %s\n",
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

void ServerSocket::freeClient(int handle)
{
    if ((handle < 0) || (handle >= (int)mMaxClients)) {
        // invalid handle
        return;
    }
    if (mClientList[handle].used) {
        mClientList[handle].clear();
        --mClientCount;
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
        return true;
    } else if (numPkts < 0) {
        printf("ERROR: SDLNet_UDP_Recv(): %s\n",
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

IPaddress* ServerSocket::handleToPeerIPaddress(int handle)
{
    if ((handle < 0) || (handle >= (int)mMaxClients)) {
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

