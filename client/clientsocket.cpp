/**
 * @author Wayne Moorefield
 * @brief Client Socket
 */

#include "clientsocket.h"


/**
 * @brief Initializes client socket
 * @param localport port to open on the client
 * @param bufferSize max buffer size for UDP packet
 * @param server server's IP address/port
 * @return true if success, otherwise failure
 */
bool ClientSocket::init(U32 localport, U32 bufferSize, IPaddress *server)
{
    bool retval = true; // default success

    mBufferSize = bufferSize;
    mClientSocket = 0;

    // validate arguments
    if (!server || (bufferSize == 0)) {
        retval = false;
    }

    // store server's IP address/port
    mServerIPaddress = *server;

    // Open up the socket using the local port
    mClientSocket = SDLNet_UDP_Open(localport);
    if (mClientSocket == 0) {
        retval = false;
        printf("ERROR: SDLNet_UDP_Open(%d): %s\n",
               localport,
               SDLNet_GetError());
    }

    return retval;
}


/**
 * @brief Closes the client socket
 *
 */
void ClientSocket::shutdown()
{
    if (mClientSocket) {
        SDLNet_UDP_Close(mClientSocket);
    }
}


/**
 * @brief Allocates a client packet using the buffer
 *        size indicated at init time.
 * @return NULL if error, otherwise pointer to valid packet
 */
ClientPacket* ClientSocket::allocPacket()
{
    ClientPacket *pkt;

    pkt = SDLNet_AllocPacket(mBufferSize);
    if (pkt == NULL) {
        printf("ERROR: SDLNet_AllocPacket(%d): %s\n",
               mBufferSize,
               SDLNet_GetError());
    }

    return pkt;
}


/**
 * @brief Frees the client packet previously allocated
 * @param pkt pointer to packet to be freed
 */
void ClientSocket::freePacket(ClientPacket *pkt)
{
    if (pkt) {
        SDLNet_FreePacket(pkt);
    }
}


/**
 * @brief Receives data from the network on client socket
 * @param pkt pointer to packet
 * @return true if pkt contains data from network, otherwise no data received
 */
bool ClientSocket::receiveData(ClientPacket *pkt)
{
    int numPkts;

    // validate arguments
    if (pkt == NULL) {
        return false;
    }

    // Attempt to receive data from socket
    numPkts = SDLNet_UDP_Recv(mClientSocket, pkt);
    if (numPkts > 0) {
        // packets received
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


/**
 * @brief Transmits data in the packet to server
 * @param pkt pointer to valid packet to transmit
 * @return true if success, otherwise error
 */
bool ClientSocket::transmitData(ClientPacket *pkt)
{
    int numRecepients;

    // validate arguments
    if (pkt == NULL) {
        return false;
    }

    // store server's IP address/port in pkt header
    pkt->address = mServerIPaddress;

    // Attempt to transmit data
    numRecepients = SDLNet_UDP_Send(mClientSocket, -1, pkt);
    if (numRecepients == 0) {
        // unable to send
        return false;
    } else {
        // success
        return true;
    }
}


/**
 * @brief Converts a string to an IP Address
 * @param address ptr to location where to store address
 * @param name string to lookup
 * @param port remote port to use
 * @return true if success, otherwise error
 */
bool ClientSocket::toIPaddress(IPaddress *address, char *name, int port)
{
    // validate arguments
    if (!address || !name) {
        return false;
    }

    // perform the lookup
    int hostResolved = SDLNet_ResolveHost(address, name, port);
    if (hostResolved == -1) {
        printf("ERROR: SDLNet_ResolveHost(%s:%d): %s\n",
               name,
               port,
               SDLNet_GetError());

        return false;
    } else {
        // success, address is already populated
        return true;
    }
}

