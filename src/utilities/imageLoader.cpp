#include "imageLoader.hpp"
#include <glm/vec2.hpp>
#include <glm/gtc/noise.hpp>
#include <iostream>

using glm::vec2;
using std::vector;
typedef unsigned int uint;

// Original source: https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_decode.cpp
PNGImage loadPNGFile(std::string fileName) {
	vector<unsigned char> png;
	vector<unsigned char> pixels; //the raw pixels
	uint width, height;

	//load and decode
	uint error = lodepng::load_file(png, fileName);
	if(!error) error = lodepng::decode(pixels, width, height, png);

	//if there's an error, display it
	if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

	// Unfortunately, images usually have their origin at the top left.
	// OpenGL instead defines the origin to be on the _bottom_ left instead, so
	// here's the world's most inefficient way to flip the image vertically.

	// You're welcome :)

	uint widthBytes = 4 * width;

	for(uint row = 0; row < (height / 2); row++) {
		for(uint col = 0; col < widthBytes; col++) {
			std::swap(pixels[row * widthBytes + col], pixels[(height - 1 - row) * widthBytes + col]);
		}
	}

	PNGImage image;
	image.width = width;
	image.height = height;
	image.pixels = pixels;

	return image;
}
