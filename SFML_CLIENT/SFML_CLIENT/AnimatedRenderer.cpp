#include "AnimatedRenderer.h"

bool AnimatedRenderer::LoadTexture(const std::string& path)
{
	return texture.loadFromFile(path);
}

void AnimatedRenderer::AddAnimation(const AnimationData& anim)
{
	animations.push_back(anim);
}

void AnimatedRenderer::PlayAnimation(const std::string& name)
{
	for (AnimationData& ad : animations) {
		if (ad.name == name) {
			if (currentAnimation == &ad) return;
			currentAnimation = &ad;
			currentFrameIndex = 0;
			frameTimer = 0.f;
			return;
		}
	}
}

void AnimatedRenderer::render(sf::RenderWindow& window)
{
	if (sprite.has_value()) {
		sprite->setPosition(transform->position);
		sprite->setColor(tint);
		window.draw(*sprite);
	}
}

void AnimatedRenderer::Update(float dt)
{
	if (currentAnimation == nullptr || currentAnimation->Frames.empty()) return;

	frameTimer += dt;
	float frameDuration = 1.f / static_cast<float>(currentAnimation->fps);

	if (frameTimer >= frameDuration) {
		frameTimer -= frameDuration;
		currentFrameIndex++;

		if (currentFrameIndex >= static_cast<int>(currentAnimation->Frames.size())) {
			currentFrameIndex = 0;
			if (onAnimationLooped) onAnimationLooped();
		}
	}

	const FrameData& frame = currentAnimation->Frames[currentFrameIndex];
	sprite.emplace(
		texture,
		sf::IntRect(frame.frameOffset, frame.frameSize)
	);
	if (flipped)
		sprite->setScale({ -sprite->getScale().x,sprite->getScale().y });
	sprite->setPosition(transform->position);
	sprite->setColor(tint);
}
