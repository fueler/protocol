
#include <stdarg.h>
#include "util.h"
#include "consoleutil.h"
#include "tcprotocol.h"
#include "servercfg.h"

// By commenting out these defines it turns off
// debugs. Likewise, uncommenting them out will
// turn them on.
//#define DEBUG_SHOW_USER_INPUT

#define SERVER_NAME "SERVER"

struct ConsoleCommand
{
    char cmd[8];
};

struct MessengerClient
{
    U32 handle;
    U32 msgAddr;
    U32 txSeq;
    U32 rxSeq;

    char name[TC_MAX_NAME_SIZE];

    void clear()
    {
        handle = 0;
        msgAddr = 0;
        txSeq = 0;
        rxSeq = 0;
        name[0] = '\0';
    }

    U32 getNextTxSeq()
    {
        return ++txSeq;
    }
};

static void sendTextMsg(ServerSocket *server,
                        MessengerClient *client,
                        const char *from,
                        const char *fmt, ...);
static bool processLeave(ServerSocket *server, U32 handle);


static ConsoleCommand gCommandList[] = {
    {"help"},
    {"list"},
    {"kick"},
    {"quit"},
    {""}
};

bool InitMessengerProtocol(ServerSocket *server)
{
    ConsolePrintf("Commands begin with \"/\". List commands with \"/help\"\n");
    return true;
}

void ShutdownMessengerProtocol(ServerSocket *server)
{
    if (!server) {
        // no clean up required
        return;
    }

    // Iterate through all clients
    for (int i=0; i<MAX_CLIENTS; ++i) {
        MessengerClient *mc = (MessengerClient*)server->getPrivateData(i);
        if (mc) {
            server->setPrivateData(i, NULL);
            server->freeClient(i);

            mc->clear();
            delete mc;
        }
    }
}

/**
 * @brief This function handles input from the user
 * @return true if keep going, otherwise exit the program
 */
bool HandleUserInput(ServerSocket *server)
{
    char buffer[CONSOLE_MAX_INPUT];
    int length;
    bool keepGoing = true;

    length = ConsoleFlushQueueToBuffer(buffer, CONSOLE_MAX_INPUT);

    if (length > 0) {
#ifdef DEBUG_SHOW_USER_INPUT
        ConsolePrintf("USERINPUT: [%d][%s]\n", length, buffer);
#endif

        // Process commands from user
        if (!strcmp(buffer, "/help")) {
            ConsolePrintf("Command List\n");
            for(ConsoleCommand *cmd = &gCommandList[0]; strlen(cmd->cmd); ++cmd) {
                ConsolePrintf("  %s\n", cmd->cmd);
            }
        } else if (!strcmp(buffer, "/list")) {
            int numClientsFound = 0;

            // print list header
            ConsolePrintf("Client List\n");

            // print list
            for (int i=0; i<MAX_CLIENTS; ++i) {
                MessengerClient *client;

                client = (MessengerClient*)server->getPrivateData(i);
                if (client) {
                    ++numClientsFound;

                    ConsolePrintf("\t%02d:\t%s\t%6d%6d\n",
                                  i,
                                  client->name,
                                  client->txSeq,
                                  client->rxSeq);
                }
            }

            // print total
            ConsolePrintf("Total Clients %d\n",
                          numClientsFound);
        } else if (!strncmp(buffer, "/kick", strlen("/kick"))) {
            if (length > (int)strlen("/kick")) {
                MessengerClient *mc;
                int handle = atoi(&buffer[strlen("/kick") + 1]);

                mc = (MessengerClient*)server->getPrivateData(handle);
                if (mc) {
                    sendTextMsg(server,
                                mc,
                                SERVER_NAME,
                                "You are being kicked");

                    if (!processLeave(server, handle)) {
                        ConsolePrintf("ERROR: Unable to kick Client %d\n",
                                       handle);
                    } else {
                        ConsolePrintf("Client %d was kicked\n",
                                       handle);
                    }
                } else {
                    ConsolePrintf("ERROR: Unknown Client %d\n",
                                  handle);
                }
            } else {
                ConsolePrintf("Missing Client handle\n");
            }
        } else if (!strcmp(buffer, "/quit")) {
            // user wants to quit
            keepGoing = false;
        }
    }

    return keepGoing;
}


/**
 * @brief This function handles data from the clients
 * @param client pointer to client socket
 * @param pkt pointer to packet received from client
 * @return true to keep going, otherwise, quit the program
 */
bool HandleClientData(ServerSocket *server, ServerPacket *pkt)
{
    bool fatalError = false;

    // Determine what client sent
    if ((U32)pkt->len >= sizeof(MsgrHdr)) {
        // valid packet
        MessengerPacket *mpkt = (MessengerPacket*)pkt->data;
        int handle = server->peerIPaddressToHandle(&pkt->address);
        switch (mpkt->hdr.type) {
        case TYPE_ACK:
            // TODO:
            // This will not be done for the first part of this.
            break;
        case TYPE_JOIN:
            // Verify message size
            if (mpkt->hdr.length == sizeof(MsgrJoin)) {
                if (handle >= 0) {
                    // client already joined
                    ConsolePrintf("ERROR: Client[%d] attempting to join again\n",
                                  handle);
                    break;
                }

                handle = server->allocClient(&pkt->address);
                if (handle >= 0) {
                    MessengerClient *client = new MessengerClient;
                    client->clear();

                    client->handle = handle;
                    client->msgAddr = mpkt->hdr.from;
                    client->rxSeq = mpkt->hdr.seq;
                    strncpy(client->name, mpkt->join.name, TC_MAX_NAME_SIZE);
                    client->name[TC_MAX_NAME_SIZE-1] = '\0';

                    server->setPrivateData(handle, client);

                    sendTextMsg(server,
                                NULL, // broadcast
                                SERVER_NAME,
                                "%s has joined",
                                client->name);
                } else {
                    ConsolePrintf("ERROR: Unable to allocate client\n");
                    fatalError = true;
                    break;
                }
            } else {
                ConsolePrintf("ERROR: client sent join message of invalid length %d != %d\n",
                              mpkt->hdr.length,
                              sizeof(MsgrJoin));
            }
            break;
        case TYPE_LEAVE:
            // Verify message size
            if (mpkt->hdr.length == sizeof(MsgrLeave)) {
                if (handle >= 0) {
                    if (!processLeave(server, handle)) {
                        fatalError = true;
                    }
                } else {
                    ConsolePrintf("ERROR: client leaving when they haven't joined yet\n");
                }
            } else {
                ConsolePrintf("ERROR: client sent leave message of invalid length %d != %d\n",
                              mpkt->hdr.length,
                              sizeof(MsgrLeave));
            }
            break;
        case TYPE_TEXT:
            // Verify message size
            if (mpkt->hdr.length == sizeof(MsgrText)) {
                if (handle >= 0) {
                    MessengerClient *client;

                    client = (MessengerClient*)server->getPrivateData(handle);
                    if (client) {
                        client->rxSeq = mpkt->hdr.seq;
                        sendTextMsg(server,
                                    NULL, // broadcast
                                    mpkt->text.name,
                                    mpkt->text.data);
                    } else {
                        ConsolePrintf("ERROR: Unable to find client %d data\n",
                                      handle);
                        fatalError = true;
                    }
                } else {
                    ConsolePrintf("ERROR: client texting when they haven't joined yet\n");
                }
            } else {
                ConsolePrintf("ERROR: client sent text message of invalid length %d != %d\n",
                              mpkt->hdr.length,
                              sizeof(MsgrText));
            }
            break;
        }
    } else {
        ConsolePrintf("ERROR: client sent message with invalid header, %d\n",
                      pkt->len);
    }

    if (fatalError) {
        // error
        return false;
    }

    return true;
}


void sendTextMsg(ServerSocket *server,
                 MessengerClient *client,
                 const char *from,
                 const char *fmt, ...)
{
    char text[TC_MAX_TEXT_SIZE*2];
    va_list args;

    va_start(args, fmt);
    vsprintf(text, fmt, args);

    if (client == NULL) {
        // Iterate through all clients
        for (int i=0; i<MAX_CLIENTS; ++i) {
            MessengerClient *mc = (MessengerClient*)server->getPrivateData(i);
            if (mc) {
                sendTextMsg(server, mc, from, text);
            }
        }

        ConsolePrintf("%s: %s\n", from, text);
    } else {
        ServerPacket *pkt = server->allocPacket();
        if (pkt) {
            MessengerPacket *mpkt = (MessengerPacket*)pkt->data;

            // Header
            mpkt->hdr.to = client->handle;
            mpkt->hdr.from = 0; // Server
            mpkt->hdr.length = sizeof(MsgrText);
            mpkt->hdr.seq = client->getNextTxSeq();
            mpkt->hdr.type = TYPE_TEXT;

            // Text NAME
            strncpy(mpkt->text.name, client->name, TC_MAX_NAME_SIZE);
            mpkt->text.name[TC_MAX_NAME_SIZE - 1] = '\0';

            // Text DATA
            strncpy(mpkt->text.data, text, TC_MAX_TEXT_SIZE);
            mpkt->text.data[TC_MAX_TEXT_SIZE - 1] = '\0';

            // Pkt Hdr
            pkt->len = sizeof(MsgrHdr) + mpkt->hdr.length;

            if (!server->transmitData(client->handle, pkt)) {
                ConsolePrintf("ERROR: Unable to send to client %d\n",
                              client->handle);
            }

            server->freePacket(pkt);
        }
    }
    va_end(args);
}

bool processLeave(ServerSocket *server, U32 handle)
{
    bool fatalError = false;
    MessengerClient *client;

    client = (MessengerClient*)server->getPrivateData(handle);
    if (client) {
        server->setPrivateData(handle, NULL);
        server->freeClient(handle);

        sendTextMsg(server,
                    NULL, // broadcast
                    SERVER_NAME,
                    "%s has left",
                    client->name);

        client->clear();
        delete client;
    } else {
        ConsolePrintf("ERROR: Unable to find client %d data\n",
                      handle);
        fatalError = true;
    }

    if (fatalError) {
        return false;
    } else {
        return true;
    }
}
