#ifndef TICTACTOEOVERLAN_UTILS_H
#define TICTACTOEOVERLAN_UTILS_H

#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_RESET   "\x1b[0m"

//https://www.realtimecolors.com/?colors=070806-fbfcfb-809476-aabcb5-8ea6a5&fonts=Inter-Inter
#define BACKGROUND_COLOR {251, 252, 251}
#define TEXT_COLOR {7, 8, 6}
#define PRIMARY_COLOR {128, 148, 118}
#define SECONDARY_COLOR {170, 188, 181}
#define ACCENT_COLOR {142, 166, 165}
#define INACTIVE_COLOR {150, 150, 150}

#include "GameDefinitions.h"
#include "string"

/**
 * @brief General purpose helper functions for Game Logic and Networking.
 * <br> Contains static methods for string conversion and memory management relating to the board state.
 */
class Utils {
public:
    /**
     * @brief Converts a PieceType enum value to a human-readable string.
     *
     * @param piece The enum value to convert.
     * @return A string representation (e.g., "Cross", "Circle", "Empty").
     */
    static std::string pieceTypeToString(const PieceType piece) {
        switch (piece) {
            case PieceType::EMPTY:
                return "EMPTY";
            case PieceType::CROSS:
                return "Cross";
            case PieceType::CIRCLE:
                return "Circle";
            case PieceType::TRIANGLE:
                return "Triangle";
            case PieceType::SQUARE:
                return "Square";
            case PieceType::OCTAGON:
                return "Octagon";
            case PieceType::HEXAGON:
                return "Hexagon";
        }

        return "Empty";
    }

    /**
     * @brief Allocates and resets the game grid.
     * <br> Resizes the 2D `grid` vector within `BoardData` based on the configured `boardSize`
     * and fills every cell with `PieceType::EMPTY`.
     *
     * @param board The BoardData structure to initialize.
     */
    static void initializeGameBoard(BoardData &board) {
        std::vector<std::vector<BoardSquare> > grid;

        for (int i = 0; i < board.boardSize; ++i) {
            std::vector<BoardSquare> row;
            for (int j = 0; j < board.boardSize; ++j) {
                BoardSquare square{};
                square.piece = PieceType::EMPTY;
                row.push_back(square);
            }
            grid.push_back(row);
        }

        board.grid = grid;
    }

    /**
     * @brief Flattens the 2D dynamic grid into a 1D static array for networking.
     * <br> Converts the logical `std::vector<std::vector<...>>` structure into a continuous
     * block of memory suitable for packet structs.
     * <br> Mapping: `index = y * width + x`
     *
     * @param inputBoard The source logical board containing the 2D vector grid.
     * @param outputBoard Pointer to the destination flat array (usually inside a Packet struct).
     * @param bufferSize The maximum size of the output buffer to prevent overflows.
     */
    static void serializeBoard(const BoardData &inputBoard, BoardSquare *outputBoard, int bufferSize) {
        int totalSquares = inputBoard.boardSize * inputBoard.boardSize;
        int limit = std::min(totalSquares, bufferSize);

        for (int y = 0; y < inputBoard.boardSize; ++y) {
            for (int x = 0; x < inputBoard.boardSize; ++x) {
                int flatIndex = y * inputBoard.boardSize + x;

                if (flatIndex >= limit) return;

                outputBoard[flatIndex] = inputBoard.getSquareAt(x, y);
            }
        }
    }

    /**
     * @brief Reconstructs the 2D grid from a received 1D network array.
     * <br> Reverses the serialization process, mapping the linear index back to X/Y coordinates
     * and updating the local `BoardData` state.
     * <br> Mapping: `x = index % width`, `y = index / width`
     *
     * @param inputBoard Pointer to the source flat array from a received Packet.
     * @param outputBoard Reference to the local BoardData to update.
     */
    static void deserializeBoard(const BoardSquare *inputBoard, BoardData &outputBoard) {
        int totalSquares = outputBoard.boardSize * outputBoard.boardSize;

        for (int i = 0; i < totalSquares; ++i) {
            int x = i % outputBoard.boardSize;
            int y = i / outputBoard.boardSize;

            outputBoard.setSquareAt(x, y, inputBoard[i]);
        }
    }
};

#endif //TICTACTOEOVERLAN_UTILS_H
