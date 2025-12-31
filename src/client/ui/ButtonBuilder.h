#pragma once
#ifndef TICTACTOEOVERLAN_BUTTONBUILDER_H
#define TICTACTOEOVERLAN_BUTTONBUILDER_H
#include <functional>
#include <string>

#include "../../common/Utils.h"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"

class ButtonWidget;

class ButtonBuilder {
private:
    float posX = 0, posY = 0;
    float width = 120, height = 25;
    std::string text = "Button";
    int textSize = 20;
    sf::Color idle = sf::Color(PRIMARY_COLOR);
    sf::Color hover = sf::Color(ACCENT_COLOR);
    sf::Color active = sf::Color(SECONDARY_COLOR);
    sf::Font *font = nullptr;
    std::function<void()> onClickCallback = [](){};

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

    std::unique_ptr<ButtonWidget> build();
};

#endif //TICTACTOEOVERLAN_BUTTONBUILDER_H