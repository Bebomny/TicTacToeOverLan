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

#include "GameDefinitions.h"

class Utils {
public:
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
            case PieceType::DIAMOND:
                return "Diamond";
            case PieceType::OCTAGON:
                return "Octagon";
        }

        return "Empty";
    }
};

#endif //TICTACTOEOVERLAN_UTILS_H
