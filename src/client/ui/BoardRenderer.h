#ifndef TICTACTOEOVERLAN_BOARDRENDERER_H
#define TICTACTOEOVERLAN_BOARDRENDERER_H
#include "../../common/GameDefinitions.h"
#include "../../common/Utils.h"
#include "SFML/Graphics/RenderTarget.hpp"

/**
 * BoardRenderer handles the drawing and bound checking of the game board.
 */
class BoardRenderer {
public:
    /**
     * Renders the board at the specified drawArea
     *
     * @param window the target window
     * @param board board to be drawn
     * @param drawArea area with specified position and size for the board to be rendered at
     * @param myTurn if it's the users turn
     * @param highlightedSquare the positon of the mouse to highlight the square beneath
     */
    static void render(
        sf::RenderTarget &window,
        const BoardData &board,
        const sf::FloatRect &drawArea,
        bool myTurn,
        sf::Vector2i highlightedSquare = {-1, -1});

    /**
     * Returns the x and y position of the square which the mouse is currently hovering over.
     *
     * @param mousePos mouse position
     * @param board the board to check against
     * @param drawArea position and size of the board on the screen
     * @return x and y position of the square on the board matrix.
     *          Returns {-1, -1} if the click occurred outside the drawArea
     */
    static sf::Vector2i getSquareAt(const sf::Vector2i &mousePos, const BoardData &board,
                                    const sf::FloatRect &drawArea);

private:
    /**
     * Draws the piece on the chosen target window
     *
     * @param window the target window
     * @param piece piece to be drawn
     * @param x x coordinate of the square, relative to the screen
     * @param y y coordinate of the square, relative to the screen
     * @param size size of the piece, used for scaling to support multiple board sizes
     */
    static void drawPiece(sf::RenderTarget &window, PieceType piece, float x, float y, float size);
};


#endif //TICTACTOEOVERLAN_BOARDRENDERER_H
