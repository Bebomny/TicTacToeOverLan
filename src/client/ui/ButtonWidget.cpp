#include "ButtonWidget.h"

#include "../../common/Utils.h"
#include "SFML/Graphics/RenderTarget.hpp"

ButtonWidget::ButtonWidget(
    const float x, const float y, const float width, const float height,
    const std::string &btnText, const int textSize, const sf::Font &font,
    const std::function<void()> &onClickCallback,
    const sf::Color idleColor, const sf::Color hoverColor, const sf::Color activeColor) : text(font) {

    this->idleColor = idleColor;
    this->hoverColor = hoverColor;
    this->activeColor = activeColor;

    this->onClick = onClickCallback;

    shape.setPosition(sf::Vector2f(x, y));
    shape.setSize(sf::Vector2f(width, height));
    shape.setFillColor(idleColor);
    shape.setOutlineThickness(2);
    shape.setOutlineColor(sf::Color(hoverColor));

    text.setFont(font);
    text.setString(btnText);
    text.setFillColor(sf::Color(TEXT_COLOR));
    text.setCharacterSize(textSize); //Also pass to the constructor???

    const sf::FloatRect bounds = text.getGlobalBounds();
    text.setOrigin({
        bounds.position.x + bounds.size.x / 2.0f,
        bounds.position.y + bounds.size.y / 2.0f
    });
    text.setPosition({x + width / 2.0f, y + height / 2.0f});
}

bool ButtonWidget::handleEvent(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) {
    if (const auto pressEvent = event->getIf<sf::Event::MouseButtonReleased>()) {
        if (pressEvent->button == sf::Mouse::Button::Left) {
            sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
            if (shape.getGlobalBounds().contains(mPos)) {
                if (onClick) onClick();
                return true;
            }
        }
    }
    return false;
}

void ButtonWidget::update(const sf::Vector2i &mousePos) {
    sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    if (shape.getGlobalBounds().contains(mPos)) {
        shape.setFillColor(hoverColor);
    } else {
        shape.setFillColor(idleColor);
    }
}

void ButtonWidget::render(sf::RenderTarget &window) {
    window.draw(shape);
    window.draw(text);
}



