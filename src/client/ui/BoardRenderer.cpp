#include "BoardRenderer.h"

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/RectangleShape.hpp"

void BoardRenderer::render(sf::RenderTarget &window, const BoardData &board, const sf::FloatRect &drawArea, const bool myTurn, sf::Vector2i highlightedSquare) {
    int sideLength = board.boardSize;

    if (sideLength == 0) return;

    float cellWidth = drawArea.size.x / static_cast<float>(sideLength);
    float cellHeight = drawArea.size.y / static_cast<float>(sideLength);
    float cellSize = std::min(cellWidth, cellHeight);

    float totalBoardSideLength = cellSize * sideLength;

    float startX = drawArea.position.x + (drawArea.size.x - totalBoardSideLength) / 2.0f;
    float startY = drawArea.position.y + (drawArea.size.y - totalBoardSideLength) / 2.0f;

    sf::RectangleShape cellShape({cellSize, cellSize});
    cellShape.setOutlineThickness(1);
    cellShape.setOutlineColor(sf::Color::Black);

    for (int y = 0; y < sideLength; y++) {
        for (int x = 0; x < sideLength; x++) {
            float pixelX = startX + (x * cellWidth);
            float pixelY = startY + (y * cellHeight);

            cellShape.setPosition({pixelX, pixelY});

            //Color
            if (highlightedSquare.x == x && highlightedSquare.y == y && myTurn) {
                cellShape.setFillColor(sf::Color(ACCENT_COLOR));
            } else {
                cellShape.setFillColor(sf::Color(SECONDARY_COLOR));
            }

            window.draw(cellShape);

            PieceType piece = board.getSquareAt(x, y).piece;
            if (piece != PieceType::EMPTY) {
                drawPiece(window, piece, pixelX, pixelY, cellSize);
            }
        }
    }
}

void BoardRenderer::drawPiece(sf::RenderTarget &window, PieceType piece, float x, float y, float size) {
    float padding = size * 0.2f;
    float actualSize = size - (2 * padding);

    switch (piece) {
        case PieceType::CROSS: {
            sf::RectangleShape line({static_cast<float>(actualSize * std::numbers::sqrt2), 5.0f});
            line.rotate(sf::degrees(45));
            line.setPosition({x + padding, y + padding});
            line.setFillColor(sf::Color(TEXT_COLOR));
            window.draw(line);

            line.rotate(sf::degrees(90));
            line.move({actualSize, 0.0f});
            window.draw(line);
            break;
        }

        case PieceType::CIRCLE: {
            sf::CircleShape circle(actualSize/2.0f);
            circle.setPosition({x + padding, y + padding});
            circle.setOutlineThickness(4);
            circle.setOutlineColor(sf::Color(TEXT_COLOR)); //TODO: highlight the local players pieces?
            circle.setFillColor(sf::Color::Transparent);
            window.draw(circle);

            break;
        }
            //TODO: add more pieces
    }
}


sf::Vector2i BoardRenderer::getSquareAt(const sf::Vector2i &mousePos, const BoardData &board, const sf::FloatRect &drawArea) {
    int sideLength = board.boardSize;

    if (sideLength == 0) return {-1, -1};

    float cellWidth = drawArea.size.x / static_cast<float>(sideLength);
    float cellHeight = drawArea.size.y / static_cast<float>(sideLength);
    float cellSize = std::min(cellWidth, cellHeight);

    float totalBoardSideLength = cellSize * sideLength;

    float startX = drawArea.position.x + (drawArea.size.x - totalBoardSideLength) / 2.0f;
    float startY = drawArea.position.y + (drawArea.size.y - totalBoardSideLength) / 2.0f;

    if (mousePos.x < startX || mousePos.x > startX + totalBoardSideLength ||
        mousePos.y < startY || mousePos.y > startY + totalBoardSideLength) {
        return {-1, -1};
    }

    // (Mouse - Start) / Size = Index
    int gridX = static_cast<int>((mousePos.x - startX) / cellSize);
    int gridY = static_cast<int>((mousePos.y - startY) / cellSize);

    if (gridX < 0 || gridX >= sideLength || gridY < 0 || gridY >= sideLength) {
        return {-1, -1};
    }

    return {gridX, gridY};
}
