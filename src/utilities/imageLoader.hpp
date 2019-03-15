#pragma once

#include "lodepng.h"
#include <vector>
#include <string>

typedef unsigned int uint;

typedef struct PNGImage {
	unsigned int width, height;
	std::vector<unsigned char> pixels; // RGBA
} PNGImage;

PNGImage loadPNGFile(std::string fileName);