#ifndef TICTACTOEOVERLAN_WIDGET_H
#define TICTACTOEOVERLAN_WIDGET_H
#include <functional>

#include "SFML/Graphics/Font.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

class Widget {
    protected:
    std::function<bool()> displayCondition = []() {return true;};
    bool active = true; //TODO: Make this a predicate too

public:
    virtual ~Widget() = default;

    void setDisplayCondition(std::function<bool()> displayCondition) {
        this->displayCondition = displayCondition;
    }

    void setActive(bool active) {
        this->active = active;
    }

    bool isVisible() const {
        return this->displayCondition();
    }

    bool isActive() const {
        return active;
    }

    /**
     *
     * @param event The captured event
     * @param mousePos The mouse position
     * @return Returns true if the event was captured by this widget, false otherwise. Prevents activating widgets stacked below.
     */
    virtual bool handleEvent(const std::optional<sf::Event> &event, const sf::Vector2i &mousePos) = 0;
    virtual void update(sf::Time deltaTime, const sf::Vector2i &mousePos) = 0;
    virtual void render(sf::RenderTarget &window) const = 0;
};

#endif //TICTACTOEOVERLAN_WIDGET_H