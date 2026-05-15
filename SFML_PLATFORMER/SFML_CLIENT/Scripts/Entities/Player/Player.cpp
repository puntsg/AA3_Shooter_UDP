#include "Player.h"
#include "../../Constants.h"
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
		velocity.x = -GameConstants::Player::SPEED;
		animRenderer->flipped = true;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
		velocity.x = GameConstants::Player::SPEED;
		animRenderer->flipped = false;
	}
	else
		velocity.x = 0;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) && grounded) {
		grounded = false;
		velocity.y = -GameConstants::Player::JUMP_FORCE;
	}

	velocity.y += 9.81f * dt * GameConstants::Player::GRAVITY_MULT;

	transform.position = sf::Vector2f(
		transform.position.x + (velocity.x * dt),
		transform.position.y + (velocity.y * dt)
	);

	fireCooldown -= dt;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && fireCooldown <= 0.f) {
		sf::Vector2f dir = animRenderer->flipped ? sf::Vector2f(-1.f, 0.f) : sf::Vector2f(1.f, 0.f);
		pendingBullet = new Bullet(GetTransform()->position, dir);
		fireCooldown = fireRate;
	}

	animRenderer->Update(dt);
}
