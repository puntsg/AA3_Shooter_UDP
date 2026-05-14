#pragma once
#include <functional>
class HealthBehaviour
{
private:
	int lifes, health;

public: 
	HealthBehaviour();

	inline int GetHeath() { return health; }
	inline int GetLifes() { return lifes;}

	void Damage();

	std::function <void()> onHealthAlter;
	std::function <void()> onDie;
	std::function <void()> onAllLifesLost;
};

