#include "HealthBehaviour.h"
#include "Constants.h"

HealthBehaviour::HealthBehaviour()
{
	health = GameConstants::Health::INITIAL_HEALTH;
	lifes = GameConstants::Health::INITIAL_LIFES;
}

void HealthBehaviour::Damage()
{
	health--;
	onHealthAlter();
	if (health <= 0) {
		health = GameConstants::Health::INITIAL_HEALTH;
		onDie();
		lifes--;
		if (lifes <= 0) {
			onAllLifesLost();
		}
	}
}
