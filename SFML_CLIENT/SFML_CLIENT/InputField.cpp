#include "InputField.h"

InputField::InputField(float x, float y, float w, float h, sf::Font& font): text(font), selected(false)
{
    rect.setPosition({ x, y });
    rect.setSize({ w, h });
    text.setPosition({ x + 5, y + 5 });
    text.setCharacterSize(18);
    text.setFillColor(sf::Color::Black);
}

void InputField::handleEvent(const sf::Event& event)
{
    if (const sf::Event::MouseButtonPressed* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (rect.getGlobalBounds().contains(sf::Vector2f(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y))))
            selected = true;
        else
            selected = false;
    }
        
    if (const sf::Event::TextEntered* e = event.getIf<sf::Event::TextEntered>())
    {
        if (selected) {
            if (e->unicode == '\b' && !input.empty())
                input.pop_back();
            else if (e->unicode >= 32 && e->unicode < 128)
                input += static_cast<char>(e->unicode);
            text.setString(input);
        }
    }
}

void InputField::Draw(sf::RenderWindow& window)
{
    window.draw(rect);
    window.draw(text);
}

std::string InputField::getText() const
{
    return input;
}
void InputField::setText(const std::string& newText)
{
    input = newText;
    text.setString(input);
}
