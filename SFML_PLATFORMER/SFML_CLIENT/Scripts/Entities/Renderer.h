#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "Transform.h"


class Renderer
{
protected:
	Transform* transform; 

public:
	Renderer(Transform* t) : transform(t) {}
	virtual ~Renderer() = default;

	
	virtual void render(sf::RenderWindow& window) = 0;
	
	
	virtual void Update(float dt) = 0; 
	
	inline Transform* GetTransform() const { return transform; }
};
