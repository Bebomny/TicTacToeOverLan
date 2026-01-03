#ifndef TICTACTOEOVERLAN_BUTTON_H
#define TICTACTOEOVERLAN_BUTTON_H
#include <string>
#include <functional>
#include <cmath>

#include "../../common/Utils.h"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

class ButtonWidget;

class ButtonBuilder {
private:
    float posX = 0, posY = 0;
    float width = 121, height = 24;
    std::string text = "Button";
    int textSize = 20;
    sf::Color idle = sf::Color(PRIMARY_COLOR);
    sf::Color hover = sf::Color(ACCENT_COLOR);
    sf::Color active = sf::Color(SECONDARY_COLOR);
    sf::Font *font = nullptr;
    std::function<void()> onClickCallback = [](){};
    std::function<bool()> displayConditionCallback = [](){ return true; };

    //For the move function
    static float prevPosX, prevPosY;
    static sf::Font sharedFont;

public:
    ButtonBuilder() = default;
    ButtonBuilder(const std::string &text, const std::function<void()> &onClickCallback);

    //static variables initialized at the start of the program
    static void initFont(const sf::Font &font) {
        sharedFont = font;
        prevPosX = 0; prevPosY = 0;
        printf(ANSI_GREEN "[UI] Font and defaults for ButtonBuilder have been loaded!\n" ANSI_RESET);
    }

    ButtonBuilder& setPosition(float x, float y);
    ButtonBuilder& move(float byX, float byY);
    ButtonBuilder& setWidth(float width);
    ButtonBuilder& setHeight(float height);
    ButtonBuilder& setSize(float width, float height);
    ButtonBuilder& setText(std::string text);
    ButtonBuilder& setTextSize(int textSize);
    ButtonBuilder& setColors(sf::Color idle, sf::Color hover, sf::Color active);
    ButtonBuilder& setDisplayCondition(const std::function<bool()> &displayCondition);

    std::unique_ptr<ButtonWidget> build();
};

class ButtonWidget {
    sf::RectangleShape shape;

    sf::Color idleColor; // Primary
    sf::Color hoverColor; // Accent
    sf::Color activeColor; // Secondary

    std::function<void()> onClick;
    std::function<bool()> displayCondition;

public:
    sf::Text text;

    ButtonWidget(float x, float y, float width, float height,
        const std::string &btnText, int textSize, const sf::Font &font,
        const std::function<void()> &onClickCallback, const std::function<bool()> &displayConditionCallback,
        sf::Color idleColor, sf::Color hoverColor, sf::Color activeColor);

    static ButtonBuilder builder(std::string text, std::function<void()> onClick) {
        return {text, onClick};
    }

    bool handleEvent(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos);
    void update(const sf::Vector2i &mousePos);
    void render(sf::RenderTarget &window);
};


#endif //TICTACTOEOVERLAN_BUTTON_H