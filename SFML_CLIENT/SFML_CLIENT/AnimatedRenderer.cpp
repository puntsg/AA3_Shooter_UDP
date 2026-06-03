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
	startOffset = sf::Vector2f(frame.frameOffset.x, frame.frameOffset.y);
	endOffset = sf::Vector2f(frame.frameOffset.x + frame.frameSize.x, frame.frameOffset.y + frame.frameSize.y);
	sprite->setOrigin({ frame.frameSize.x / 2.f, frame.frameSize.y / 2.f }); 
	sprite->setScale({ flipped ? -scaleFactor : scaleFactor, scaleFactor });
	sprite->setPosition(transform->position);
	sprite->setColor(tint);
}

void AnimatedRenderer::ApplyFrameRect(sf::Vector2f start, sf::Vector2f end)
{
	if (end.x - start.x <= 0.f || end.y - start.y <= 0.f)
		return;

	startOffset = start;
	endOffset = end;

	sf::IntRect rect(
		sf::Vector2i((int)start.x, (int)start.y),
		sf::Vector2i((int)(end.x - start.x), (int)(end.y - start.y))
	);

	if (!sprite.has_value())
		sprite.emplace(texture, rect);
	else
		sprite->setTextureRect(rect);

	sprite->setOrigin({ (end.x - start.x) / 2.f, (end.y - start.y) / 2.f });
	sprite->setScale({ flipped ? -scaleFactor : scaleFactor, scaleFactor });
	sprite->setPosition(transform->position);
	sprite->setColor(tint);
}
