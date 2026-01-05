#include "WinValidator.h"

bool WinValidator::checkWin(const BoardData &board, const int lastX, const int lastY) {
    const PieceType currentPiece = board.getSquareAt(lastX, lastY).piece;

    if (currentPiece == PieceType::EMPTY) return false;

    // Go in each direction and calculate the same consecutive pieces

    // Horizontal
    if (count(board, lastX, lastY, -1, 0) + count(board, lastX, lastY, 1, 0) + 1 >= board.winConditionLength) {
        return true;
    }

    // Vertical
    if (count(board, lastX, lastY, 0, -1) + count(board, lastX, lastY, 0, 1) + 1 >= board.winConditionLength) {
        return true;
    }

    // Diagonal
    if (count(board, lastX, lastY, -1, -1) + count(board, lastX, lastY, 1, 1) + 1 >= board.winConditionLength) {
        return true;
    }

    // 2nd Diagonal
    if (count(board, lastX, lastY, -1, 1) + count(board, lastX, lastY, 1, -1) + 1 >= board.winConditionLength) {
        return true;
    }

    return false;
}


int WinValidator::count(const BoardData &board, const int startX, const int startY, const int dx, const int dy) {
    const PieceType target = board.getSquareAt(startX, startY).piece;
    int count = 0;
    int x = startX + dx;
    int y = startY + dy;

    while (isValid(board, x, y)) {
        if (board.getSquareAt(x, y).piece == target) {
            count++;
        } else {
            break;
        }

        x += dx;
        y += dy;
    }
    return count;
}


bool WinValidator::isValid(const BoardData &board, const int x, const int y) {
    return x >= 0 && x < board.boardSize && y >= 0 && y < board.boardSize;
}
