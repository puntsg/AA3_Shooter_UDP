#pragma once
#include <vector>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../Entities/Entity.h"
class Scene
{
protected:
	std::vector<Entity*> ui;
	std::vector<Entity*> entities;

public:
	Scene() = default;
	virtual ~Scene() = default;
	
	virtual void OnEnter() = 0;
	
	virtual void HandleEvent(const sf::Event& event) {}
	
	virtual void Update(float dt) = 0;
	virtual void Render(sf::RenderWindow& window) = 0;
	virtual void OnExit() = 0;
};
