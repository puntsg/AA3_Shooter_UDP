#pragma once
#include "Renderer.h"
#include <functional>
#include <optional>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

struct FrameData {
	sf::Vector2i frameSize;
	sf::Vector2i frameOffset;
};
struct AnimationData {
	std::string name;
	int fps;
	std::vector<FrameData> Frames;
};

class AnimatedRenderer : public Renderer
{
private:
	std::vector<AnimationData> animations;
	AnimationData* currentAnimation = nullptr;
	int currentFrameIndex = 0;
	float frameTimer = 0.f;
public:
	sf::Vector2f startOffset, endOffset;
	sf::Texture texture;
	std::optional<sf::Sprite> sprite;
	sf::Color tint = sf::Color::White;
	bool flipped = false;
	float scaleFactor = 1.f; // <1 para empequenecer el sprite (se aplica cada frame)
	std::function<void()> onAnimationLooped;

	AnimatedRenderer(Transform* t) : Renderer(t) {}

	bool LoadTexture(const std::string& path);
	void AddAnimation(const AnimationData& anim);
	void PlayAnimation(const std::string& name);

	void render(sf::RenderWindow& window) override;
	void Update(float dt) override;
	void ApplyFrameRect(sf::Vector2f start, sf::Vector2f end); // aplica un frame recibido por red
};

