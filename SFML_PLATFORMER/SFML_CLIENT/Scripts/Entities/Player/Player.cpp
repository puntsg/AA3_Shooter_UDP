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
	animRenderer->Update(dt);
}
