/**
 * @author Wayne Moorefield
 * @brief This file contains TC Protocol header,
 *        types and function prototypes.
 */

#ifndef _TCPROTOCOL_H
#define _TCPROTOCOL_H

#include "types.h"
#include "clientsocket.h"

enum {
    TYPE_ACK = 1, // No ACKs right now
    TYPE_JOIN,
    TYPE_LEAVE,
    TYPE_TEXT
};

enum {
    TO_ADDRESS_SERVER = 0,
    TO_ADDRESS_BROADCAST = 1
};

enum {
    TC_MAX_NAME_SIZE = 8,
    TC_MAX_TEXT_SIZE = 128
};

struct MsgrHdr
{
    U32 to;
    U32 from;
    U32 seq;
    U32 type;
    U32 length;
};

struct MsgrJoin
{
    char name[TC_MAX_NAME_SIZE];
};

struct MsgrLeave
{
    char name[TC_MAX_NAME_SIZE];
};

struct MsgrText
{
    char name[TC_MAX_NAME_SIZE];
    char data[TC_MAX_TEXT_SIZE];
};

struct MessengerPacket
{
    MsgrHdr hdr;
    union {
        MsgrJoin join;
        MsgrLeave leave;
        MsgrText text;
    };
};

bool InitMessengerProtocol();
void ShutdownMessengerProtocol();
bool HandleUserInput(ClientSocket *client);
bool HandleServerData(ClientSocket *client, ClientPacket *pkt);

#endif
