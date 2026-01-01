#ifndef TICTACTOEOVERLAN_WINVALIDATOR_H
#define TICTACTOEOVERLAN_WINVALIDATOR_H
#include "../common/GameDefinitions.h"


class WinValidator {
public:
    static bool checkWin(const BoardData &board, int lastX, int lastY);

private:
    static int count(const BoardData &board, int startX, int startY, int dx, int dy);

    static bool isValid(const BoardData &board, int x, int y);
};


#endif //TICTACTOEOVERLAN_WINVALIDATOR_H
