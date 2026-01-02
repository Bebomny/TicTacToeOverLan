#include "ButtonBuilder.h"
#include "ButtonWidget.h"

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



