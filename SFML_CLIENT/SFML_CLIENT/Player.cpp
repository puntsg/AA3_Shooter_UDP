#include "Player.h"
#include "Constants.h"
static void SetupAnimations(AnimatedRenderer* ar)
{
	ar->LoadTexture("Sprites/newDuck.png");
	ar->scaleFactor = Config::Gameplay::SPRITE_SCALE;   // frame 49x65 -> ~15x20 px, para que no se solapen

	AnimationData idle;
	idle.name  = "idle";
	idle.fps   = 4;
	idle.Frames = {
		{ { Config::Gameplay::FRAME_W, Config::Gameplay::FRAME_H }, {0,   0} },  
		{ { Config::Gameplay::FRAME_W, Config::Gameplay::FRAME_H }, {49,  0} },   
		{ { Config::Gameplay::FRAME_W, Config::Gameplay::FRAME_H }, {98,  0} },   
		{ { Config::Gameplay::FRAME_W, Config::Gameplay::FRAME_H }, {0,  65} },  
	};
	ar->AddAnimation(idle);

	AnimationData move;
	move.name = "move";
	move.fps = 8;
	move.Frames = {
		{ { Config::Gameplay::FRAME_W, Config::Gameplay::FRAME_H }, {49, 65} },  
		{ { Config::Gameplay::FRAME_W, Config::Gameplay::FRAME_H }, {98, 65} },   
		{ { Config::Gameplay::FRAME_W, Config::Gameplay::FRAME_H }, {0, 130} },   
		{ { Config::Gameplay::FRAME_W, Config::Gameplay::FRAME_H }, {49,130} },   
	};
	ar->AddAnimation(move);
	ar->PlayAnimation("idle");
	// El origen y la escala los fija AnimatedRenderer::Update cada frame (centrado + scaleFactor).
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
		sf::Vector2f bulletPos = GetTransform()->position + dir * Config::Gameplay::BULLET_MUZZLE_OFFSET;
		pendingBullet = new Bullet(bulletPos, dir);
		fireCooldown = fireRate;
	}

	animRenderer->Update(dt);
}
