#pragma once
#include "Transform.h"
#include "Renderer.h"


class Entity
{
protected:
	Transform transform;
	Renderer* renderer = nullptr;

public:
	Entity() = default;
	
	virtual ~Entity()
	{
		if (renderer) {
			delete renderer;
			renderer = nullptr;
		}
	}


	inline Transform* GetTransform() { return &transform; }

	inline void SetRenderer(Renderer* r) { renderer = r; }
	inline Renderer* GetRenderer() const { return renderer; }

	virtual void Update(float dt);
};
