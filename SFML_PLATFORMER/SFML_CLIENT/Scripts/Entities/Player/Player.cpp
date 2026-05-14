#include "Player.h"

static void SetupAnimations(AnimatedRenderer* ar)
{
	ar->LoadTexture("Sprites/duck.png");

	AnimationData idle;
	idle.name  = "idle";
	idle.fps   = 1;
	idle.Frames = {
		{ {14,11}, {0,  0} },
		{ {14,11}, {14, 0} },
		{ {14,11}, {0, 11} },
		{ {14,11}, {14, 11} },
	};
	ar->AddAnimation(idle);
	ar->PlayAnimation("idle");
	sf::FloatRect bounds = ar->sprite->getLocalBounds();
	ar->sprite->setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
}

Player::Player()
{
	animRenderer = new AnimatedRenderer(GetTransform());
	SetupAnimations(animRenderer);
	SetRenderer(animRenderer);
}

Player::Player(int _id, std::string _name, int _score, sf::Color _color, bool _isLocal)
	: Player()
{
	id            = _id;
	nickName      = _name;
	scoreRanking  = _score;
	color         = _color;
	isLocal       = _isLocal;
}

void Player::Update(float dt)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
		velocity.x = -50;
		animRenderer->flipped = true;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
		velocity.x = 50;
		animRenderer->flipped = false;
	}
	else
		velocity.x = 0;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) && grounded)
		velocity.y = -20;

	velocity.y += 9.8f * dt;
	
	transform.position = sf::Vector2f(
		transform.position.x + (velocity.x * dt),
		transform.position.y + (velocity.y * dt)
	);
	animRenderer->Update(dt);
}
