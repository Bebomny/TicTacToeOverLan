#ifndef TICTACTOEOVERLAN_BOARDRENDERER_H
#define TICTACTOEOVERLAN_BOARDRENDERER_H
#include "../../common/GameDefinitions.h"
#include "../../common/Utils.h"
#include "SFML/Graphics/RenderTarget.hpp"


class BoardRenderer {
public:
    static void render(
        sf::RenderTarget &window,
        const BoardData& board,
        const sf::FloatRect& drawArea,
        const bool myTurn,
        sf::Vector2i highlightedSquare = {-1, -1});

    // Returns {-1, -1} if the click was outside the drawArea
    static sf::Vector2i getSquareAt(const sf::Vector2i& mousePos, const BoardData& board, const sf::FloatRect& drawArea);

private:
    static void drawPiece(sf::RenderTarget &window, PieceType piece, float x, float y, float size);
};


#endif //TICTACTOEOVERLAN_BOARDRENDERER_H