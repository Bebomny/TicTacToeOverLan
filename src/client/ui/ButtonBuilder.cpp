#include "ButtonBuilder.h"

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

ButtonBuilder &ButtonBuilder::setColors(sf::Color idle, sf::Color hover, sf::Color active) {
    this->idle = idle;
    this->hover = hover;
    this->active = active;
    return *this;
}

std::unique_ptr<ButtonWidget> ButtonBuilder::build() {
    prevPosX = this->posX;
    prevPosY = this->posY;

    if (!this->font) throw std::runtime_error("[ButtonBuilder] No font available!");

    return std::make_unique<ButtonWidget>(
        posX, posY, width, height, text, *font, onClickCallback, idle, hover, active);
}



