#ifndef TICTACTOEOVERLAN_GAMEDEFINITIONS_H
#define TICTACTOEOVERLAN_GAMEDEFINITIONS_H
#include <cstdint>
#include <vector>
/**
 * The available Piece Types. Used from Top to Bottom, selected by the player number
 */
enum class PieceType : uint8_t {
    EMPTY,
    CROSS,
    CIRCLE,
    TRIANGLE,
    SQUARE,
    DIAMOND,
    OCTAGON
};

struct Player {
    PieceType piece;
    uint8_t playerId;
    char playerName[32]; //Circular dependencies here move to a variable later
    bool myTurn;
    bool isMe;
};

struct BoardSquare {
    PieceType piece;
    uint8_t playerId;
    uint16_t turnPlaced;
};

struct Move {
    PieceType piece;
    uint8_t playerId;
    uint16_t turnPlaced;
    uint8_t posX;
    uint8_t posY;
};

struct BoardData {
    std::vector<std::vector<BoardSquare>> grid;
    uint8_t boardSize;
    uint8_t winConditionLength;
    uint16_t turn;
    uint8_t currentPlayerId;
};

#endif //TICTACTOEOVERLAN_GAMEDEFINITIONS_H