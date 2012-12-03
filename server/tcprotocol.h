#ifndef _TCPROTOCOL_H
#define _TCPROTOCOL_H

#include "types.h"
#include "serversocket.h"

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


bool HandleClientData(ServerSocket *server, ServerPacket *pkt);

#endif
