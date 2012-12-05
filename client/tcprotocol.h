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
    TYPE_JOIN,
    TYPE_LEAVE,
    TYPE_TEXT
};

struct MessengerHdr
{
    U32 to;
    U32 from;
    U32 seq;
    U32 type;
    U32 length;
};

bool HandleUserInput();
bool HandleServerData(ClientSocket *client, ClientPacket *pkt);

#endif
