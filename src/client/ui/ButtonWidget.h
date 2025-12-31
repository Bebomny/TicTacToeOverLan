#ifndef TICTACTOEOVERLAN_BUTTON_H
#define TICTACTOEOVERLAN_BUTTON_H
#include <functional>

#include "ButtonBuilder.h"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Window/Event.hpp"

class ButtonWidget {
    sf::RectangleShape shape;


    sf::Color idleColor; // Primary
    sf::Color hoverColor; // Accent
    sf::Color activeColor; // Secondary

    std::function<void()> onClick;

public:
    sf::Text text;

    ButtonWidget(float x, float y, float width, float height,
        const std::string &btnText, int textSize, const sf::Font &font,
        const std::function<void()> &onClickCallback,
        sf::Color idleColor, sf::Color hoverColor, sf::Color activeColor);

    static ButtonBuilder builder(std::string text, std::function<void()> onClick) {
        return {text, onClick};
    }

    bool handleEvent(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos);
    void update(const sf::Vector2i &mousePos);
    void render(sf::RenderTarget &window);
};


#endif //TICTACTOEOVERLAN_BUTTON_H