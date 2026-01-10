#ifndef TICTACTOEOVERLAN_BUTTON_H
#define TICTACTOEOVERLAN_BUTTON_H
#include <string>
#include <functional>
#include <cmath>

#include "Widget.h"
#include "../../common/Utils.h"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

class ButtonWidget;

/**
 * @brief A Builder class for constructing ButtonWidget instances.
 */
class ButtonBuilder {
private:
    float posX = 0, posY = 0;
    float width = 121, height = 24;
    std::string text = "Button";
    int textSize = 20;
    sf::Color idle = sf::Color(PRIMARY_COLOR);
    sf::Color hover = sf::Color(ACCENT_COLOR);
    sf::Color active = sf::Color(SECONDARY_COLOR);
    sf::Color inactive = sf::Color(INACTIVE_COLOR);
    sf::Font *font = nullptr;
    std::function<void()> onClickCallback = []() {
    };
    std::function<bool()> displayConditionCallback = []() { return true; };

    //For the move function
    static float prevPosX, prevPosY;
    static sf::Font sharedFont;

public:
    ButtonBuilder() = default;

    /**
     * @brief Constructs a builder with essential button properties.
     *
     * @param text The label text of the button.
     * @param onClickCallback The function to execute when the button is clicked.
     */
    ButtonBuilder(const std::string &text, const std::function<void()> &onClickCallback);

    /**
     * @brief Initializes global resources and layout trackers for all Buttons.
     * This must be called once at startup. It sets the shared font used by default
     * and resets the static layout cursor (prevPosX/Y).
     *
     * @param font The SFML font reference to store globally.
     */
    static void initFont(const sf::Font &font) {
        sharedFont = font;
        prevPosX = 0;
        prevPosY = 0;
        printf(ANSI_GREEN "[UI] Font and defaults for ButtonBuilder have been loaded!\n" ANSI_RESET);
    }

    /**
     * @brief Sets the absolute position of the button.
     *
     * @param x The X coordinate.
     * @param y The Y coordinate.
     * @return Reference to the builder for chaining.
     */
    ButtonBuilder &setPosition(float x, float y);

    /**
     * @brief Offsets the position relative to the last built widget.
     * Useful for stacking fields vertically or horizontally without calculating absolute coordinates.
     *
     * @param byX Amount to shift on the X axis.
     * @param byY Amount to shift on the Y axis.
     * @return Reference to the builder for chaining.
     */
    ButtonBuilder &move(float byX, float byY);

    /**
     * @brief Sets the width of the button.
     *
     * @param width Width in pixels.
     * @return Reference to the builder for chaining.
     */
    ButtonBuilder &setWidth(float width);

    /**
     * @brief Sets the height of the button.
     *
     * @param height Height in pixels.
     * @return Reference to the builder for chaining.
     */
    ButtonBuilder &setHeight(float height);

    /**
     * @brief Sets the size (width and height) of the button.
     *
     * @param width Width in pixels.
     * @param height Height in pixels.
     * @return Reference to the builder for chaining.
     */
    ButtonBuilder &setSize(float width, float height);

    /**
     * @brief Sets the label text.
     *
     * @param text The string to display.
     * @return Reference to the builder for chaining.
     */
    ButtonBuilder &setText(std::string text);

    /**
     * @brief Sets the character size of the text.
     *
     * @param textSize Size in pixels.
     * @return Reference to the builder for chaining.
     */
    ButtonBuilder &setTextSize(int textSize);

    /**
     * @brief Configures the color palette for the button's different states.
     *
     * @param idle Color when the button is default/resting.
     * @param hover Color when the mouse is over the button.
     * @param active Color when the button is being clicked (pressed).
     * @param inactive Color when the button is disabled.
     * @return Reference to the builder for chaining.
     */
    ButtonBuilder &setColors(sf::Color idle, sf::Color hover, sf::Color active, sf::Color inactive);

    /**
     * @brief Sets a custom visibility condition.
     *
     * @param displayCondition Function returning true if button should be visible.
     * @return Reference to the builder for chaining.
     */
    ButtonBuilder &setDisplayCondition(const std::function<bool()> &displayCondition);

    /**
     * @brief Finalizes construction and creates the ButtonWidget.
     *
     * @return A unique pointer to the fully configured ButtonWidget.
     */
    std::unique_ptr<ButtonWidget> build();
};

/**
 * @brief A Widget implementation representing a clickable button.
 * Handles visual states (Idle, Hover, Active, Inactive) and executes a callback when clicked.
 * It is recommended to create instances via the ButtonBuilder.
 */
class ButtonWidget final : public Widget {
    sf::RectangleShape shape;

    sf::Color idleColor; // Primary
    sf::Color hoverColor; // Accent
    sf::Color activeColor; // Secondary
    sf::Color inactiveColor; // Grayed out

    std::function<void()> onClick;

public:
    sf::Text text;

    /**
    * @brief Main Constructor.
    * <p> Do not use! Use ButtonWidget::builder() instead.
    * <p> Called internally by ButtonBuilder::build().
    */
    ButtonWidget(float x, float y, float width, float height,
                 const std::string &btnText, int textSize, const sf::Font &font,
                 const std::function<void()> &onClickCallback, const std::function<bool()> &displayConditionCallback,
                 sf::Color idleColor, sf::Color hoverColor, sf::Color activeColor, sf::Color inactiveColor);

    /**
     * @brief Static factory method to start the Builder chain.
     *
     * @param text The button label.
     * @param onClick The callback function.
     * @return A configured ButtonBuilder instance.
     */
    static ButtonBuilder builder(std::string text, std::function<void()> onClick) {
        return {text, onClick};
    }

    /**
     * @brief Handles mouse events for the button.
     * Detects if the mouse is hovering over the button or if a click occurred.
     * Executes the onClick callback if a left-click is released within bounds.
     *
     * @param event The SFML event (checked for MouseButtonReleased).
     * @param mousePos The current mouse coordinates.
     * @return True if the button was clicked or interacted with, consuming the event.
     */
    bool handleEvent(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) override;

    /**
     * @brief Updates the visual state of the button.
     * Changes the button color based on whether the mouse is hovering, or the widget is inactive.
     *
     * @param deltaTime Time elapsed since last update.
     * @param mousePos Current mouse position for hover detection.
     */
    void update(sf::Time deltaTime, const sf::Vector2i &mousePos) override;

    /**
     * @brief Renders the button to the target.
     * Checks `isVisible()` before drawing. If false, nothing is drawn.
     *
     * @param window The render target.
     */
    void render(sf::RenderTarget &window) const override;
};


#endif //TICTACTOEOVERLAN_BUTTON_H
