#include "Entity.h"

void Entity::Update(float dt)
{
	if (renderer) {
		renderer->Update(dt);
	}
}