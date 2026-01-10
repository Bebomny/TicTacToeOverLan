#ifndef TICTACTOEOVERLAN_SERVERUTILS_H
#define TICTACTOEOVERLAN_SERVERUTILS_H
#include "ClientContext.h"

#include "../common/NetworkProtocol.h"

/**
 * @brief Helper utilities specific to the InternalGameServer logic.
 */
class ServerUtils {
public:
    /**
     * @brief Converts the server-side internal client representation into a public Player struct.
     * <br> Used for network synchronization, as `ClientContext` contains private data that should not be sent to clients,
     * whereas `Player` is the "public" view of a user suitable for the UI.
     *
     * @param client The internal client context to convert.
     * @param requestingPlayerId The ID of the player requesting this data (or the ID of the
     * client to whom this packet will be sent). This is used to correctly set the `isMe` flag.
     * @return A Player struct populated with safe-to-share data.
     */
    static Player clientContextToPlayer(const ClientContext &client, bool requestingPlayerId);
};


#endif //TICTACTOEOVERLAN_SERVERUTILS_H
