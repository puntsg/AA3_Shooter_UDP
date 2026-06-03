#include "Player.h"
#include "Constants.h"
static void SetupAnimations(AnimatedRenderer* ar)
{
	ar->LoadTexture("Sprites/newDuck.png");
	ar->scaleFactor = 0.3f;   // frame 49x65 -> ~15x20 px, para que no se solapen

	AnimationData idle;
	idle.name  = "idle";
	idle.fps   = 4;
	idle.Frames = {
		{ {49,65}, {0,   0} },  
		{ {49,65}, {49,  0} },   
		{ {49,65}, {98,  0} },   
		{ {49,65}, {0,  65} },  
	};
	ar->AddAnimation(idle);

	AnimationData move;
	move.name = "move";
	move.fps = 8;
	move.Frames = {
		{ {49,65}, {49, 65} },  
		{ {49,65}, {98, 65} },   
		{ {49,65}, {0, 130} },   
		{ {49,65}, {49,130} },   
	};
	ar->AddAnimation(move);
	ar->PlayAnimation("idle");
	sf::FloatRect bounds = ar->sprite->getLocalBounds();
	ar->sprite->setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
	ar->sprite->setScale(sf::Vector2f(.75f,.75f));
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
	if (!isLocal)
		return;

	if (inputLocked) {
		velocity.x = 0;
		animRenderer->PlayAnimation("idle");
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
		velocity.x = -GameConstants::Player::SPEED;
		animRenderer->flipped = false;
		animRenderer->PlayAnimation("move");
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
		velocity.x = GameConstants::Player::SPEED;
		animRenderer->flipped = true;
		animRenderer->PlayAnimation("move");
	}
	else {
		animRenderer->PlayAnimation("idle");
		velocity.x = 0;
	}
	if (!inputLocked && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) && grounded) {
		grounded = false;
		velocity.y = -GameConstants::Player::JUMP_FORCE;
	}

	velocity.y += 9.81f * dt * GameConstants::Player::GRAVITY_MULT;

	transform.position = sf::Vector2f(
		transform.position.x + (velocity.x * dt),
		transform.position.y + (velocity.y * dt)
	);

	fireCooldown -= dt;
	if (!inputLocked && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && fireCooldown <= 0.f) {
		sf::Vector2f dir = animRenderer->flipped ? sf::Vector2f(1.f, 0.f) : sf::Vector2f(-1.f, 0.f);
		sf::Vector2f bulletPos = GetTransform()->position + dir * 16.f;
		pendingBullet = new Bullet(bulletPos, dir);
		fireCooldown = fireRate;
	}

	animRenderer->Update(dt);
}
