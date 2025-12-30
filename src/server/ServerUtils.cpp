#include "ServerUtils.h"

Player ServerUtils::clientContextToPlayer(const ClientContext &client, bool requestingPlayerId) {
    Player p {};
    p.playerId = client.playerId;
    // p.playerName = client.playerName;
    memset(p.playerName, 0, MAX_PLAYER_NAME_LENGTH);
    strncpy(p.playerName, client.playerName, MAX_PLAYER_NAME_LENGTH-1);
    p.piece = client.pieceType;
    p.isMe = (client.playerId == requestingPlayerId);
    p.myTurn = false;
    return p;
}
