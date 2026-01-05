#include "TextFieldWidget.h"


//// Builder ////
float TextFieldBuilder::prevPosX = 0.0f;
float TextFieldBuilder::prevPosY = 0.0f;
sf::Font TextFieldBuilder::sharedFont;

TextFieldBuilder::TextFieldBuilder() {
    this->font = &sharedFont;
}

TextFieldBuilder::TextFieldBuilder(const std::string &initialText, const std::function<void(const std::string &)> &onTextChangeCallback) {
    this->font = &sharedFont;
    this->initialText = initialText;
    this->onTextChange = onTextChangeCallback;
}


TextFieldBuilder &TextFieldBuilder::setPosition(float x, float y) {
    this->posX = x;
    this->posY = y;
    return *this;
}

TextFieldBuilder &TextFieldBuilder::move(float byX, float byY) {
    return this->setPosition(prevPosX + byX, prevPosY + byY);
}

TextFieldBuilder &TextFieldBuilder::setWidth(float width) {
    this->width = width;
    return *this;
}

TextFieldBuilder &TextFieldBuilder::setHeight(float height) {
    this->height = height;
    return *this;
}

TextFieldBuilder &TextFieldBuilder::setSize(float width, float height) {
    this->width = width;
    this->height = height;
    return *this;
}

TextFieldBuilder &TextFieldBuilder::setMaxChars(size_t maxChars) {
    this->maxChars = maxChars;
    return *this;
}

TextFieldBuilder &TextFieldBuilder::setText(std::string text) {
    this->initialText = text;
    return *this;
}

TextFieldBuilder &TextFieldBuilder::setColors(sf::Color idle, sf::Color focus, sf::Color text, sf::Color inactive) {
    this->idleColor = idle;
    this->focusColor = focus;
    this->textColor = text;
    this->inactiveColor = inactive;
    return *this;
}

TextFieldBuilder &TextFieldBuilder::setOnTextChange(const std::function<void(const std::string&)> &onTextChange) {
    this->onTextChange = onTextChange;
    return *this;
}

TextFieldBuilder &TextFieldBuilder::setDisplayCondition(const std::function<bool()> &displayCondition) {
    this->displayCondition = displayCondition;
    return *this;
}

TextFieldBuilder &TextFieldBuilder::setTextSize(int textSize) {
    this->textSize = textSize;
    return *this;
}


std::unique_ptr<TextFieldWidget> TextFieldBuilder::build() {
    prevPosX = this->posX;
    prevPosY = this->posY;

    if (!this->font) throw std::runtime_error("[ButtonBuilder] No fonts available!");

    return std::make_unique<TextFieldWidget>(
        posX, posY, width, height,
        onTextChange, displayCondition,
        *font, idleColor, focusColor, textColor, inactiveColor,
        maxChars, initialText);
}



//// Widget ////
const sf::Time TextFieldWidget::cursorBlickTime = sf::milliseconds(500);

TextFieldWidget::TextFieldWidget(float x, float y, float width, float height,
    const std::function<void(const std::string&)> &onTextChangeCallback, const std::function<bool()> &displayCondition,
    const sf::Font &font, const sf::Color idleColor, const sf::Color focusColor, const sf::Color textColor, const sf::Color inactiveColor,
    const size_t maxChars, const std::string &initialText) : renderText(font) , maxChars(maxChars), idleColor(idleColor), focusColor(focusColor), textColor(textColor), inactiveColor(inactiveColor) {

    this->onTextChange = onTextChangeCallback;
    this->displayCondition = displayCondition;

    // Background Box
    backgroundShape.setPosition({x, y});
    backgroundShape.setSize({width, height});
    backgroundShape.setFillColor(idleColor);
    backgroundShape.setOutlineThickness(1);
    backgroundShape.setOutlineColor(textColor);

    // Text
    renderText.setFont(font);
    renderText.setCharacterSize(20);
    renderText.setFillColor(textColor);
    renderText.setPosition({x + 5, y + 2});

    // Cursor
    cursorShape.setSize({2.0f, static_cast<float>(renderText.getCharacterSize())});
    cursorShape.setFillColor(textColor);

    this->setText(initialText);
}

bool TextFieldWidget::handleEvent(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) {
    if (!this->isVisible() || !this->isActive()) return false;

    const sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    if (const auto &btnEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
        if (btnEvent->button == sf::Mouse::Button::Left) {
            if (this->contains(mPos)) {
                isFocused = true;
                backgroundShape.setFillColor(focusColor);
                backgroundShape.setOutlineColor(sf::Color::Cyan); // Highlight
                return true;
            } else {
                isFocused = false;
                backgroundShape.setFillColor(idleColor);
                backgroundShape.setOutlineColor(textColor);
            }
        }
    }

    if (isFocused) {
        if (const auto &textEvent = event->getIf<sf::Event::TextEntered>()) {
            const uint32_t unicode = textEvent->unicode;

            if (unicode == 8) {
                if (!currentString.empty()) {
                    currentString.pop_back();
                }
            } else if (unicode < 128 && unicode > 30) {
                if (currentString.size() < maxChars) {
                    //TODO: check if the character fits inside the width of the text field?
                    currentString += static_cast<char>(unicode);
                }
            }

            renderText.setString(currentString);

            if (onTextChange) onTextChange(currentString);

            return true;
        }
    }

    return false;
}

void TextFieldWidget::update(const sf::Time deltaTime, const sf::Vector2i &mousePos) {
    if (!isVisible()) return;

    if (!this->isActive()) {
        backgroundShape.setFillColor(inactiveColor);
        isFocused = false;
    }

    if (!isFocused) {
        showCursor = false;
        return;
    }


    cursorTimer += deltaTime;
    if (cursorTimer >= cursorBlickTime) {
        showCursor = !showCursor;
        cursorTimer = sf::Time::Zero;
    }

    sf::Vector2f endPos = renderText.findCharacterPos(currentString.size());
    cursorShape.setPosition({endPos.x, endPos.y});
}

void TextFieldWidget::render(sf::RenderTarget &window) const {
    if (!isVisible()) return;

    window.draw(backgroundShape);
    window.draw(renderText);

    if (showCursor && isFocused) {
        window.draw(cursorShape);
    }
}

void TextFieldWidget::setText(const std::string &text) {
    currentString = text;
    renderText.setString(currentString);
}

bool TextFieldWidget::contains(const sf::Vector2f &pos) const {
    return backgroundShape.getGlobalBounds().contains(pos);
}

