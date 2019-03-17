#include "imageLoader.hpp"
#include <glm/vec2.hpp>
#include <glm/gtc/noise.hpp>
#include <iostream>

using glm::vec2;
using std::vector;
typedef unsigned int uint;

// Original source: https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_decode.cpp
PNGImage loadPNGFile(std::string fileName, bool flip_handedness) {
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

	if (flip_handedness) {
		for (uint xb = 0; xb < widthBytes; xb+=4)
		for (uint y = 0; y < height; y++) {
			unsigned char& r = pixels[y*widthBytes + xb + 0];
			unsigned char& g = pixels[y*widthBytes + xb + 1];
			unsigned char& b = pixels[y*widthBytes + xb + 2];
			unsigned char& a = pixels[y*widthBytes + xb + 3];
			
			r = 255 - r;
			g = 255 - g;
		}
	}

	PNGImage image;
	image.width = width;
	image.height = height;
	image.pixels = pixels;

	return image;
}


PNGImage makePerlinNoisePNG(uint w, uint h, float scale) {
	return makePerlinNoisePNG(w, h, vector<float>{scale});
}
PNGImage makePerlinNoisePNG(uint w, uint h, vector<float> scales) {
	uint wb = 4*w; // in bytes
	
	vector<unsigned char> pixels(wb*h);
	for (uint y = 0; y < h; y++)
	for (uint x = 0; x < w; x++) {
		float v = 0;
		for (float scale : scales)
			v += glm::simplex(vec2(x*scale, y*scale));
		v /= scales.size();
		
		unsigned char val =  (unsigned char)  (127 + 128*v);
		pixels[y*wb + x*4 + 0] = val;
		pixels[y*wb + x*4 + 1] = val;
		pixels[y*wb + x*4 + 2] = val;
		pixels[y*wb + x*4 + 3] = 0xff;
	}

	PNGImage image;
	image.width  = w;
	image.height = h;
	image.pixels = pixels;
	return image;
}
