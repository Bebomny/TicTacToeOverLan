#include "ButtonWidget.h"

//// Builder ////
float ButtonBuilder::prevPosX = 0.0f;
float ButtonBuilder::prevPosY = 0.0f;
sf::Font ButtonBuilder::sharedFont;

ButtonBuilder::ButtonBuilder(const std::string &text, const std::function<void()> &onClickCallback) {
    this->font = &sharedFont;
    this->text = text;
    this->onClickCallback = onClickCallback;
}

ButtonBuilder &ButtonBuilder::setPosition(float x, float y) {
    this->posX = x;
    this->posY = y;
    return *this;
}

ButtonBuilder &ButtonBuilder::move(float byX, float byY) {
    //TODO: Test this!!!
    return this->setPosition(prevPosX + byX, prevPosY + byY);
}

ButtonBuilder &ButtonBuilder::setWidth(float width) {
    this->width = width;
    return *this;
}

ButtonBuilder &ButtonBuilder::setHeight(float height) {
    this->height = height;
    return *this;
}

ButtonBuilder &ButtonBuilder::setSize(float width, float height) {
    this->width = width;
    this->height = height;
    return *this;
}

ButtonBuilder &ButtonBuilder::setText(std::string text) {
    this->text = text;
    return *this;
}

ButtonBuilder &ButtonBuilder::setTextSize(int textSize) {
    this->textSize = textSize;
    return *this;
}

ButtonBuilder &ButtonBuilder::setColors(sf::Color idle, sf::Color hover, sf::Color active) {
    this->idle = idle;
    this->hover = hover;
    this->active = active;
    return *this;
}

ButtonBuilder &ButtonBuilder::setDisplayCondition(const std::function<bool()> &displayCondition) {
    this->displayConditionCallback = displayCondition;
    return *this;
}

std::unique_ptr<ButtonWidget> ButtonBuilder::build() {
    prevPosX = this->posX;
    prevPosY = this->posY;

    if (!this->font) throw std::runtime_error("[ButtonBuilder] No fonts available!");

    return std::make_unique<ButtonWidget>(
        posX, posY, width, height,
        text, textSize, *font,
        onClickCallback, displayConditionCallback,
        idle, hover, active);
}


//// Widget ////

ButtonWidget::ButtonWidget(
    const float x, const float y, const float width, const float height,
    const std::string &btnText, const int textSize, const sf::Font &font,
    const std::function<void()> &onClickCallback, const std::function<bool()> &displayConditionCallback,
    const sf::Color idleColor, const sf::Color hoverColor, const sf::Color activeColor) : text(font) {

    this->idleColor = idleColor;
    this->hoverColor = hoverColor;
    this->activeColor = activeColor;

    this->onClick = onClickCallback;
    this->displayCondition = displayConditionCallback;

    shape.setPosition(sf::Vector2f(x, y));
    shape.setSize(sf::Vector2f(width, height));
    shape.setFillColor(idleColor);
    shape.setOutlineThickness(2);
    shape.setOutlineColor(sf::Color(hoverColor));

    text.setFont(font);
    text.setString(btnText);
    text.setFillColor(sf::Color(TEXT_COLOR));
    text.setCharacterSize(textSize);

    const sf::FloatRect bounds = text.getGlobalBounds();
    text.setOrigin({
        std::floor(bounds.position.x + bounds.size.x / 2.0f),
        std::floor(bounds.position.y + bounds.size.y / 2.0f)
    });
    text.setPosition({
        std::floor(x + width / 2.0f),
        std::floor(y + height / 2.0f)
    });
}

bool ButtonWidget::handleEvent(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) {
    if (!displayCondition()) return false;

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
    if (!displayCondition()) return;

    sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    if (shape.getGlobalBounds().contains(mPos)) {
        shape.setFillColor(hoverColor);
    } else {
        shape.setFillColor(idleColor);
    }
}

void ButtonWidget::render(sf::RenderTarget &window) {
    if (!displayCondition()) return;

    window.draw(shape);
    window.draw(text);
}



