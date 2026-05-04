#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include "UIElement.h"
class Button: public UIElement
{
public:
	Button(float xPos, float yPos, float width, float height);
	Button(float xPos, float yPos, float width, float height, sf::Font& font);
	void SetText(std::string newText);
	void handleEvent(const sf::Event& event);
	void Draw(sf::RenderWindow& window) override;
	std::function <void()> onClick;
private:
	sf::RectangleShape rect;
	std::optional<sf::Text> text;
};

