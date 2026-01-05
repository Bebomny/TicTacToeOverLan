#ifndef TICTACTOEOVERLAN_TEXTFIELDWIDGET_H
#define TICTACTOEOVERLAN_TEXTFIELDWIDGET_H
#include <functional>

#include "Widget.h"
#include "../../common/Utils.h"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

class TextFieldWidget;

class TextFieldBuilder {
private:
    float posX = 0, posY = 0;
    float width = 200, height = 28;
    std::string initialText = "";
    int textSize = 20;
    size_t maxChars = 30;
    sf::Color textColor = sf::Color(TEXT_COLOR);
    sf::Color idleColor = sf::Color(BACKGROUND_COLOR);
    sf::Color focusColor = sf::Color(SECONDARY_COLOR);
    sf::Color inactiveColor = sf::Color(INACTIVE_COLOR);
    sf::Font* font;
    std::function<void(const std::string&)> onTextChange;
    std::function<bool()> displayCondition;

    static float prevPosX, prevPosY;
    static sf::Font sharedFont; // TODO: A widget class that contains methods for loading a sharedFont to remove boilerplate like this

public:
    TextFieldBuilder();
    TextFieldBuilder(const std::string &initialText, const std::function<void(const std::string&)> &onTextChangeCallback);

    static void initFont(const sf::Font &font) {
        sharedFont = font;
        prevPosX = 0, prevPosY = 0;
        printf(ANSI_GREEN "[UI] Font and defaults for TextFieldBuilder have been loaded!\n" ANSI_RESET);
    }

    TextFieldBuilder& setPosition(float x, float y);
    TextFieldBuilder& move(float byX, float byY);
    TextFieldBuilder& setWidth(float width);
    TextFieldBuilder& setHeight(float height);
    TextFieldBuilder& setSize(float width, float height);
    TextFieldBuilder& setMaxChars(size_t maxChars);
    TextFieldBuilder& setText(std::string text);
    TextFieldBuilder& setTextSize(int textSize);
    TextFieldBuilder& setColors(sf::Color idle, sf::Color focus, sf::Color text, sf::Color inactive);
    TextFieldBuilder& setOnTextChange(const std::function<void(const std::string&)> &onTextChange);
    TextFieldBuilder& setDisplayCondition(const std::function<bool()> &displayCondition);

    std::unique_ptr<TextFieldWidget> build();
};

class TextFieldWidget final : public Widget {
private:
    sf::RectangleShape backgroundShape;
    sf::Text renderText;
    sf::RectangleShape cursorShape;

    std::string currentString;
    bool isFocused = false;
    bool showCursor = false;

    size_t maxChars;
    sf::Color idleColor;
    sf::Color focusColor;
    sf::Color textColor;
    sf::Color inactiveColor; // Grayed out

    std::function<void(const std::string&)> onTextChange;

    sf::Time cursorTimer;
    static const sf::Time cursorBlickTime;

public:
    TextFieldWidget(
            float x, float y,
            float width, float height,
            const std::function<void(const std::string&)> &onTextChangeCallback, const std::function<bool()> &displayCondition,
            const sf::Font& font, sf::Color idleColor, sf::Color focusColor, sf::Color textColor, sf::Color inactiveColor,
            size_t maxChars = 30, const std::string &initialText = "");

    static TextFieldBuilder builder() {
        return {};
    }

    static TextFieldBuilder builder(std::string initialText, std::function<void(const std::string&)> onTextChange) {
        return {initialText, onTextChange};
    }

    bool handleEvent(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) override;
    void update(sf::Time deltaTime, const sf::Vector2i &mousePos) override;
    void render(sf::RenderTarget &window) const override;

    std::string getText() const {return currentString;}
    void setText(const std::string &text);
    void setCallback(const std::function<void(const std::string&)> &callback) {onTextChange = callback;}

    bool contains(const sf::Vector2f &pos) const;
};


#endif //TICTACTOEOVERLAN_TEXTFIELDWIDGET_H