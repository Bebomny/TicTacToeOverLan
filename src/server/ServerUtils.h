#ifndef TICTACTOEOVERLAN_SERVERUTILS_H
#define TICTACTOEOVERLAN_SERVERUTILS_H
#include "ClientContext.h"

#include "../common/NetworkProtocol.h"

class ServerUtils {
public:
    static Player clientContextToPlayer(const ClientContext &client, bool requestingPlayerId);
};


#endif //TICTACTOEOVERLAN_SERVERUTILS_H
