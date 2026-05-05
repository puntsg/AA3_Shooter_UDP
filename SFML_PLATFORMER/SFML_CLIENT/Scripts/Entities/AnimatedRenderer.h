#pragma once
#include "Renderer.h"
#include <functional>
class AnimatedRenderer :
    public Renderer
{
private:
	sf::Vector2i frameSize;
	int fps;
	bool looping;
	float frameTime;
	float currentFrameTime;
	sf::Vector2f sourceSize;
	bool hasLooped;
public:
	AnimatedRenderer(Transform* transform, std::string resourcePath,
		sf::Vector2i sourceOffset, sf::Vector2i sourceSize,
		int frameWidth, int frameHeight,
		int fps, bool looping);
	AnimatedRenderer(Transform* transform, std::string resourcePath,
		sf::Vector2i sourceOffset, sf::Vector2i sourceSize,
		int frameWidth, int frameHeight,
		int fps, bool looping, bool executeOnStart);
	void PlayAnimation();
	virtual void Update(float dt) override;
	virtual void render();
	std::function <void()> onLooped;
};

