#pragma once

#include "lodepng.h"
#include <vector>
#include <string>

typedef unsigned int uint;

struct PNGImage {
	uint width, height;
	std::vector<unsigned char> pixels; // RGBA
};

PNGImage loadPNGFile(std::string fileName);