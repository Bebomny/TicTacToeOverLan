#ifndef TICTACTOEOVERLAN_DRAWUTILS_H
#define TICTACTOEOVERLAN_DRAWUTILS_H
#include <cmath>

#include "SFML/Graphics/Text.hpp"

class DrawUtils {
public:
    static void centerText(sf::Text &text) {
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({
            std::floor(bounds.position.x + bounds.size.x / 2.0f),
            std::floor(bounds.position.y + bounds.size.y / 2.0f)
        });
    }

    static void centerTextVertically(sf::Text &text) {
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({
            bounds.position.x,
            std::floor(bounds.position.y + bounds.size.y / 2.0f)
        });
    }

    static void centerTextHorizontally(sf::Text &text) {
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({
            std::floor(bounds.position.x + bounds.size.x / 2.0f),
            bounds.position.y
        });
    }
};

#endif //TICTACTOEOVERLAN_DRAWUTILS_H
