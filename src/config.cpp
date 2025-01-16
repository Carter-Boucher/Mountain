#include "config.h"

#include <fstream>
#include <iostream>

config loadConfig(const std::string& path) {
	config cfg; // Start with defaults

	std::ifstream in(path);
	if (!in.is_open()) {
		std::cerr << "Error: Could not open config file: " << path << std::endl;
		return cfg; // Return defaults on failure
	}

	// Read values in order
	in >> cfg.seed
		>> cfg.octaves
		>> cfg.frequency
		>> cfg.lacunarity
		>> cfg.gain
		>> cfg.ridgeOffset
		>> cfg.width
		>> cfg.height
		>> cfg.subdivisions
		>> cfg.dotSize
		>> cfg.type;

	// You could add more robust parsing (e.g., checking if the read failed).
	return cfg;
}
