#include "Player.h"

Player::Player() {
	//https://gamefromscratch.com/sfml-c-tutorial-spritesheets-and-animation/
	texture.loadFromFile("Sprites/duck.png");
	sprite.emplace(texture, sf::IntRect(sf::Vector2(0, 0), sf::Vector2(28, 22)));
}
Player::Player(int _id, std::string _name, int _score, sf::Color _color, bool _isLocal, Transform _transform, sf::Texture _texture, sf::Sprite _sprite)
	: id(_id), nickName(_name), scoreRanking(_score), color(_color), isLocal(_isLocal), transform(_transform), texture(_texture), sprite(_sprite)
{
	//https://gamefromscratch.com/sfml-c-tutorial-spritesheets-and-animation/
	texture.loadFromFile("Sprites/duck.png");
	sprite.emplace(texture,sf::IntRect(sf::Vector2(0,0),sf::Vector2(28,22)));
}

void Player::Update(float dt)
{
	
	if (sprite) {
		std::cout << "a";
		SM.window.draw(*sprite);
	}
}
