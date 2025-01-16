#pragma once

#include <string>

struct config {
	int   seed = 1;
	float octaves = 6.0f;
	float frequency = 2.0f;
	float lacunarity = 3.0f;
	float gain = 0.1f;
	float ridgeOffset = 1.0f;
	int width = 100;
	int height = 100;
	int subdivisions = 100;
	int dotSize = 5;
	int type = 0;
};

config loadConfig(const std::string& path);
