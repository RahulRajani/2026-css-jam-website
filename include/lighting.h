#pragma once

#include <cstdint>
#include <vector>
#include <cmath>

#include <stdio.h>

#include <raylib.h>

struct LightSource {
	int x;
	int y;
	int radius;
};

struct LightingMap {
	int width;
	int height;

	std::vector<std::vector<uint8_t>> obstacles;

	inline LightingMap() = default;

	inline LightingMap(int width, int height) {
		this->width = width;
		this->height = height;

		for (int i = 0; i < height; i++) {
			this->obstacles.push_back(std::vector<uint8_t>(width, 0));
		}
	}

	inline void setObstacle(int x, int y, uint8_t val) {
		this->obstacles[y][x] = val;
	}

	struct LightNode {
		int x;
		int y;
		LightSource source;
		int steps;

		bool present;
	};

	// may be super fucking slow
	// sources normalised to screen space (i.e. subtract screen top left from all pos)
	inline void calcAndDrawLights(
		const std::vector<LightSource>& sources,
		int scrX, int scrY, int scrWid, int scrHeight
	) {
		std::vector<std::vector<LightNode>> propagatedNodes;
		for (int i = 0; i < scrHeight; i++) {
			propagatedNodes.push_back(std::vector<LightNode>(scrWid, LightNode{}));
		}

		std::vector<LightNode> lightQueue;
		size_t lightTop = 0;

		for (const LightSource& source : sources) {
			lightQueue.push_back(LightNode{source.x, source.y, source});
		}

		while (lightTop < lightQueue.size()) {
			const LightNode& node = lightQueue[lightTop++];

			if (node.steps >= node.source.radius) {
				continue;
			}

			if (node.y > 0 && node.y < scrHeight - 1) {
				if (node.y > 0 && !obstacles[node.y - 1 - scrY][node.x - scrX]) {
					LightNode& propNode = propagatedNodes[node.y - 1 - scrY][node.x - scrX];

					if (!propNode.present || propNode.steps > node.steps + 1) {
						propNode.present = true;
						propNode.source = node.source;
						propNode.steps = node.steps + 1;

						lightQueue.push_back(LightNode{node.x, node.y - 1, node.source, node.steps + 1});
					}
				}

				if (node.y < scrHeight - 1 && !obstacles[node.y + 1 - scrY][node.x - scrX]) {
					LightNode& propNode = propagatedNodes[node.y + 1 - scrY][node.x - scrX];

					if (!propNode.present || propNode.steps > node.steps + 1) {
						propNode.present = true;
						propNode.source = node.source;
						propNode.steps = node.steps + 1;

						lightQueue.push_back(LightNode{node.x, node.y + 1, node.source, node.steps + 1});
					}
				}
			}

			if (node.x > 0 && node.x < scrWid - 1) {
				if (node.x > 0 && !obstacles[node.y - scrY][node.x - 1 - scrX]) {
					LightNode& propNode = propagatedNodes[node.y - scrY][node.x - 1 - scrX];

					if (!propNode.present || propNode.steps > node.steps + 1) {
						propNode.present = true;
						propNode.source = node.source;
						propNode.steps = node.steps + 1;

						lightQueue.push_back(LightNode{node.x - 1, node.y, node.source, node.steps + 1});
					}
				}

				if (node.x < scrWid - 1 && !obstacles[node.y - scrY][node.x + 1 - scrX]) {
					LightNode& propNode = propagatedNodes[node.y - scrY][node.x + 1 - scrX];

					if (!propNode.present || propNode.steps > node.steps + 1) {
						propNode.present = true;
						propNode.source = node.source;
						propNode.steps = node.steps + 1;

						lightQueue.push_back(LightNode{node.x + 1, node.y, node.source, node.steps + 1});
					}
				}
			}
		}

		for (int y = 0; y < scrHeight; y++) {
			for (int x = 0; x < scrWid; x++) {
				const LightNode& node = propagatedNodes[y][x];
				if (node.present) {
					int nodeX = node.x - node.source.x;
					int nodeY = node.y - node.source.y;

					float str = (
						1.0f -
						sqrt(nodeX * nodeX + nodeY * nodeY) / (node.source.radius)
					);

					DrawPixel(x, y, {246, 154, 84, (uint8_t)(255 * 0.5f * str)});
				}
			}
		}
	}
};
