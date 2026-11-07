#pragma once

#include <raylib.h>

#include <vector>
#include <string>

struct AnimatedSprite {
	std::vector<Texture> sprites;
	int frame;
	float frametime;
	float frametimeMax;

	inline AnimatedSprite() = default;
	inline AnimatedSprite(std::vector<std::string> spriteFiles, float frametimeMax) {
		frame = 0;
		frametime = 0;
		this->frametimeMax = frametimeMax;

		for (const std::string& filename : spriteFiles) {
			sprites.push_back(LoadTexture(filename.c_str()));
		}
	}

	inline void unloadTextures() {
		for (Texture texture : sprites) {
			UnloadTexture(texture);
		}

		sprites.clear();
	}

	inline void updateFrame(float deltaTime) {
		frametime += deltaTime;
		if (frametime >= frametimeMax) {
			frame = (frame + 1) % sprites.size();
			frametime = 0;
		}
	}

	inline void resetFrame() {
		frame = 0;
		frametime = 0;
	}

	inline const Texture& getCurrentTexture() {
		return sprites[frame];
	}
};
