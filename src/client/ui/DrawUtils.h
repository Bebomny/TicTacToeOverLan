#ifndef TICTACTOEOVERLAN_DRAWUTILS_H
#define TICTACTOEOVERLAN_DRAWUTILS_H
#include <cmath>

#include "SFML/Graphics/Text.hpp"


/**
 * @brief A utility class for common graphics operations.
 * <br> Note: These functions should be called before setting the text's position.
 */
class DrawUtils {
public:
    /**
     * @brief Sets the text object's origin to its geometric center.
     * <br> This allows to position the text based on its center point rather than the top-left corner.
     *
     * @param text The SFML text object to modify.
     */
    static void centerText(sf::Text &text) {
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({
            std::floor(bounds.position.x + bounds.size.x / 2.0f),
            std::floor(bounds.position.y + bounds.size.y / 2.0f)
        });
    }

    /**
     * @brief Sets the text object's origin to its vertical center.
     * <br> The horizontal origin is reset to the left edge of the bounds.
     *
     * @param text The SFML text object to modify.
     */
    static void centerTextVertically(sf::Text &text) {
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({
            bounds.position.x,
            std::floor(bounds.position.y + bounds.size.y / 2.0f)
        });
    }

    /**
     * @brief Sets the text object's origin to its horizontal center.
     * <br> The vertical origin is reset to the top edge of the bounds.
     *
     * @param text The SFML text object to modify.
     */
    static void centerTextHorizontally(sf::Text &text) {
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({
            std::floor(bounds.position.x + bounds.size.x / 2.0f),
            bounds.position.y
        });
    }
};

#endif //TICTACTOEOVERLAN_DRAWUTILS_H
