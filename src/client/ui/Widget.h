#ifndef TICTACTOEOVERLAN_WIDGET_H
#define TICTACTOEOVERLAN_WIDGET_H
#include <functional>
#include <utility>

#include "SFML/Graphics/Font.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

/**
 * @brief Abstract base class representing a UI element.
 * * The Widget class provides the foundational interface for all UI components,
 * managing visibility states, activity states, and the core game loop methods
 * (event handling, updating, and rendering).
 */
class Widget {
protected:
    /**
     * @brief A predicate function determining if the widget should be visible.
     * Defaults to always returning true.
     */
    std::function<bool()> displayCondition = []() { return true; };

    /**
     * @brief Internal state indicating if the widget is currently active/interactable.
     * Defaults to active.
     */
    bool active = true;

public:
    virtual ~Widget() = default;

    /**
     * @brief Sets a dynamic condition for the widget's visibility.
     * * This allows the widget to automatically show or hide itself based on external
     * logic (e.g., If a player had won a game).
     * * It reduces boilerplate code in the main loop, by allowing the widget to check, if it should be rendered by itself.
     * * @param displayCondition A function returning true if the widget should be visible, false otherwise.
     */
    void setDisplayCondition(std::function<bool()> displayCondition) {
        this->displayCondition = std::move(displayCondition);
    }

    /**
     * @brief Manually sets the active state of the widget.
     * * An inactive widget should not respond to events, though it is still drawn in an inactive state.
     * * If a widget requires different activation logic it should be handled in the handleEvent, update and render methods.
     *
     * @param active True to enable the widget, false to disable it.
     */
    void setActive(bool active) {
        this->active = active;
    }

    /**
     * @brief Checks if the widget should currently be visible.
     * @return True if the displayCondition is met, false otherwise.
     */
    bool isVisible() const {
        return this->displayCondition();
    }

    /**
     * @brief Checks if the widget is currently active.
     *
     * @return True if the widget is active, false otherwise.
     */
    bool isActive() const {
        return active;
    }

    /**
     * @brief Processes an input event.
     * * This function should check if the event interacts with the widget (e.g., mouse click inside bounds).
     *
     * @param event The captured system event.
     * @param mousePos The current position of the mouse relative to the window.
     * @return True if the event was consumed by this widget, false otherwise.
     * Returning true prevents this event from propagating to widgets stacked below.
     */
    virtual bool handleEvent(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) = 0;

    /**
     * @brief Updates the widget's logic.
     * * Use this for animations, state changes over time, or responding to mouse hover states
     * that don't require specific events.
     *
     * @param deltaTime The time elapsed since the last frame update.
     * @param mousePos The current position of the mouse relative to the window.
     */
    virtual void update(sf::Time deltaTime, const sf::Vector2i &mousePos) = 0;

    /**
     * @brief Draws the widget to the specified render target.
     *
     * @param window The target window to render the widget onto.
     */
    virtual void render(sf::RenderTarget &window) const = 0;
};

#endif //TICTACTOEOVERLAN_WIDGET_H
