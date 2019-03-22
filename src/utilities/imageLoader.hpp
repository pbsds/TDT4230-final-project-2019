#pragma once

#include "lodepng.h"
#include <glm/vec4.hpp>
#include <vector>
#include <string>

typedef unsigned int uint;

struct PNGImage {
	uint width, height;
	bool repeat_mirrored = false;
	std::vector<unsigned char> pixels; // RGBA
	bool has_transparancy = false;
	
	glm::vec4 get(int x, int y);
	glm::vec4 at_nearest(double u, double v);
	glm::vec4 at_bilinear(double u, double v);
};

PNGImage loadPNGFile(std::string filename, bool flip_handedness=false);

PNGImage* loadPNGFileDynamic(std::string filename, bool flip_handedness=false);
PNGImage* loadPNGFileDynamicNoCaching(std::string filename, bool flip_handedness=false);

PNGImage makePerlinNoisePNG(uint w, uint h, float scale=0.1);

PNGImage makePerlinNoisePNG(uint w, uint h, std::vector<float> scales);
