#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "UIElement.h"
class InputField: public UIElement
{
public:
    InputField(float x, float y, float w, float h, sf::Font& font);
    void handleEvent(const sf::Event& event) override;
    void Draw(sf::RenderWindow& window) override;
    std::string getText() const;
    void setText(const std::string& newText);
private:
    sf::RectangleShape rect;
    sf::Text text;
    std::string input;
    bool selected;
};

