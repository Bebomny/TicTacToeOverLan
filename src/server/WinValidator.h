#ifndef TICTACTOEOVERLAN_WINVALIDATOR_H
#define TICTACTOEOVERLAN_WINVALIDATOR_H
#include "../common/GameDefinitions.h"

/**
 * @brief Logic for detecting victory conditions on the board.
 * <br> Uses a raycasting algorithm to count consecutive pieces surrounding a newly placed token.
 */
class WinValidator {
public:
    /**
     * @brief Determines if the last move resulted in a win.
     * <br> Scans four axes (Horizontal, Vertical, Diagonal /, Diagonal \) centered on the last move.
     * <br> Logic: For each axis, it counts identical pieces in the positive direction and negative direction.
     * <br> If (count1 + count2 + 1) >= `board.winConditionLength`, the player has won.
     *
     * @param board The current state of the game board.
     * @param lastX The X coordinate of the most recently placed piece.
     * @param lastY The Y coordinate of the most recently placed piece.
     * @return True if the move created a winning line, false otherwise.
     */
    static bool checkWin(const BoardData &board, int lastX, int lastY);

private:
    /**
     * @brief Counts consecutive identical pieces in a specific direction.
     * <br> Steps through the grid by `(dx, dy)` starting from `(startX, startY)`
     * and increments the counter as long as the pieces match the target piece type.
     *
     * @param board The board data.
     * @param startX Starting X position.
     * @param startY Starting Y position.
     * @param dx X direction step (e.g., -1 for left, 1 for right).
     * @param dy Y direction step (e.g., -1 for up, 1 for down).
     * @return The number of consecutive matching neighbors found in that direction.
     */
    static int count(const BoardData &board, int startX, int startY, int dx, int dy);

    /**
     * @brief Bounds checking helper.
     * <br> Ensures the scanning algorithm doesn't try to access grid coordinates outside the vector's allocated size.
     *
     * @param board The board data.
     * @param x X coordinate to check.
     * @param y Y coordinate to check.
     * @return True if (x,y) is inside the playable grid area.
     */
    static bool isValid(const BoardData &board, int x, int y);
};


#endif //TICTACTOEOVERLAN_WINVALIDATOR_H
