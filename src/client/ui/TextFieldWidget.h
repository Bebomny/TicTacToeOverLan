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

/**
 * @brief A Builder class for constructing TextFieldWidget instances.
 */
class TextFieldBuilder {
private:
    float posX = 0, posY = 0;
    float width = 200, height = 28;
    std::string initialText;
    int textSize = 20;
    size_t maxChars = 30;
    sf::Color textColor = sf::Color(TEXT_COLOR);
    sf::Color idleColor = sf::Color(BACKGROUND_COLOR);
    sf::Color focusColor = sf::Color(SECONDARY_COLOR);
    sf::Color inactiveColor = sf::Color(INACTIVE_COLOR);
    sf::Font *font;
    std::function<void(const std::string &)> onTextChange;
    std::function<bool()> displayCondition;

    static float prevPosX, prevPosY;
    static sf::Font sharedFont;

public:
    TextFieldBuilder();

    /**
     * @brief Constructor allowing immediate definition of text and callback.
     *
     * @param initialText The text string to start with.
     * @param onTextChangeCallback Function called whenever the user modifies the text.
     */
    TextFieldBuilder(const std::string &initialText,
                     const std::function<void(const std::string &)> &onTextChangeCallback);

    /**
     * @brief Initializes the global font resource and layout tracker.
     * <br> Must be called before building any TextFields.
     *
     * @param font The SFML font to be used by all text fields created via builder.
     */
    static void initFont(const sf::Font &font) {
        sharedFont = font;
        prevPosX = 0, prevPosY = 0;
        printf(ANSI_GREEN "[UI] Font and defaults for TextFieldBuilder have been loaded!\n" ANSI_RESET);
    }

    /**
     * @brief Sets the absolute position.
     *
     * @param x X Coordinate.
     * @param y Y Coordinate.
     * @return Reference to the builder for chaining.
     */
    TextFieldBuilder &setPosition(float x, float y);

    /**
     * @brief Offsets the position relative to the last built widget.
     * <br> Useful for stacking fields vertically or horizontally without calculating absolute coordinates.
     *
     * @param byX Amount to move on X axis.
     * @param byY Amount to move on Y axis.
     * @return Reference to the builder for chaining.
     */
    TextFieldBuilder &move(float byX, float byY);

    /**
     * @brief Sets the width of the input box.
     *
     * @param width Width in pixels.
     * @return Reference to the builder for chaining.
     */
    TextFieldBuilder &setWidth(float width);

    /**
     * @brief Sets the height of the input box.
     *
     * @param height Height in pixels.
     * @return Reference to the builder for chaining.
     */
    TextFieldBuilder &setHeight(float height);

    /**
     * @brief Sets dimensions of the input box.
     *
     * @param width Width in pixels.
     * @param height Height in pixels.
     * @return Reference to the builder for chaining.
     */
    TextFieldBuilder &setSize(float width, float height);

    /**
     * @brief Sets the maximum number of characters allowed.
     *
     * @param maxChars The character limit.
     * @return Reference to the builder for chaining.
     */
    TextFieldBuilder &setMaxChars(size_t maxChars);

    /**
     * @brief Sets the initial string content.
     * 
     * @param text The string.
     * @return Reference to the builder for chaining.
     */
    TextFieldBuilder &setText(std::string text);

    /**
     * @brief Sets the font size.
     * 
     * @param textSize Size in pixels.
     * @return Reference to the builder for chaining.
     */
    TextFieldBuilder &setTextSize(int textSize);

    /**
     * @brief Configures the color palette.
     * 
     * @param idle Background color when not focused.
     * @param focus Background/Outline color when typing.
     * @param text Color of the font.
     * @param inactive Color when disabled.
     * @return Reference to the builder for chaining.
     */
    TextFieldBuilder &setColors(sf::Color idle, sf::Color focus, sf::Color text, sf::Color inactive);

    /**
     * @brief Sets the callback function for text modifications.
     * 
     * @param onTextChange Function accepting the new string state.
     * @return Reference to the builder for chaining.
     */
    TextFieldBuilder &setOnTextChange(const std::function<void(const std::string &)> &onTextChange);

    /**
     * @brief Sets a custom visibility condition.
     * 
     * @param displayCondition Function returning true if button should be visible.
     * @return Reference to the builder for chaining.
     */
    TextFieldBuilder &setDisplayCondition(const std::function<bool()> &displayCondition);

    /**
     * @brief Finalizes construction and creates the TextFieldWidget.
     *
     * @return A unique pointer to the fully configured TextFieldWidget.
     */
    std::unique_ptr<TextFieldWidget> build();
};

/**
 * @brief A Widget allowing user text input.
 * Handles focus states (clicking to activate), keyboard events (typing, backspace), and cursor blinking animation.
 */
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

    std::function<void(const std::string &)> onTextChange;

    sf::Time cursorTimer;
    static const sf::Time cursorBlickTime;

public:
    /**
    * @brief Main Constructor.
    * <br> Do not use! Use TextFieldWidget::builder() instead.
    * <br> Called internally by TextFieldBuilder::build().
    */
    TextFieldWidget(
        float x, float y,
        float width, float height,
        const std::function<void(const std::string &)> &onTextChangeCallback,
        const std::function<bool()> &displayCondition,
        const sf::Font &font, sf::Color idleColor, sf::Color focusColor, sf::Color textColor, sf::Color inactiveColor,
        size_t maxChars = 30, const std::string &initialText = "");

    /**
     * @brief Creates a default builder.
     *
     * @return A new builder instance.
     */
    static TextFieldBuilder builder() {
        return {};
    }

    /**
     * @brief Creates a builder pre-filled with text and callback.
     *
     * @param initialText The starting text.
     * @param onTextChange The callback.
     * @return A new builder instance.
     */
    static TextFieldBuilder builder(std::string initialText, std::function<void(const std::string &)> onTextChange) {
        return {initialText, onTextChange};
    }

    /**
     * @brief Processes input for the text field.
     * <br> Handles two main things:
     * <br> 1. Mouse Clicks: Checks if the user clicked inside (gain focus) or outside (lose focus).
     * <br> 2. Text Input: If focused, handles `sf::Event::TextEntered` to update the string.
     *
     * @param event The system event (required for character typing).
     * @param mousePos Current mouse position.
     * @return True if the event was consumed (e.g., user typed or clicked the field).
     */
    bool handleEvent(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) override;

    /**
     * @brief Updates widget logic, primarily the cursor animation.
     *
     * @param deltaTime Time elapsed since last frame.
     * @param mousePos Mouse position
     */
    void update(sf::Time deltaTime, const sf::Vector2i &mousePos) override;

    /**
     * @brief Draws the background, text, and cursor (if focused and active).
     *
     * @param window The render target.
     */
    void render(sf::RenderTarget &window) const override;

    /**
     * @brief Gets the current user input.
     *
     * @return The string contained in the field.
     */
    std::string getText() const { return currentString; }

    /**
     * @brief Manually sets the content of the field.
     * <br> DOES NOT trigger the onTextChangeCallback
     *
     * @param text The new string.
     */
    void setText(const std::string &text);

    /**
     * @brief Updates the callback function dynamically.
     *
     * @param callback The new function to call on changes.
     */
    void setCallback(const std::function<void(const std::string &)> &callback) { onTextChange = callback; }

    /**
     * @brief Checks if a specific point is inside the widget's bounds.
     * <br> Used internally by handleEvent for focus detection.
     *
     * @param pos The position to check (usually mouse coordinates).
     * @return True if inside, false otherwise.
     */
    bool contains(const sf::Vector2f &pos) const;
};


#endif //TICTACTOEOVERLAN_TEXTFIELDWIDGET_H
