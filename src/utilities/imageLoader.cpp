#include "imageLoader.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/noise.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <string>

using glm::vec2;
using glm::vec4;
using std::string;
using std::map;
using std::vector;
typedef unsigned int uint;

inline vec4 operator*(vec4 a, double s) {
	return a *= s;
}

vec4 PNGImage::get(int x, int y) {
	if (repeat_mirrored) {
		x %= width *2;
		y %= height*2;
		if (x >= width)  x = width *2 - x - 1;
		if (y >= height) y = height*2 - y - 1;
	} else {
		x %= width; y %= height;
	}
	return vec4(
		float(pixels[x*4+y*width*4 + 0]) / 255,
		float(pixels[x*4+y*width*4 + 1]) / 255,
		float(pixels[x*4+y*width*4 + 2]) / 255,
		float(pixels[x*4+y*width*4 + 3]) / 255);
}
vec4 PNGImage::at_nearest(double u, double v) {
	int x = int( u*(width) - 0.5);
	int y = int(-v*(height) + 0.5);
	return get(x, y);
}
vec4 PNGImage::at_bilinear(double u, double v) {
	double x =  u*double(width) - 0.5;
	double y = -v*double(height) + 0.5;
	int x1 = int(x + 0.0);
	int y1 = int(y + 0.0);
	int x2 = int(x + 1.0);
	int y2 = int(y + 1.0);
	double xx1 = x - x1;
	double yy1 = y - y1;
	double x2x = 1 - xx1;
	double y2y = 1 - yy1;
	vec4 q11 = get(x1, y1);
	vec4 q21 = get(x2, y1);
	vec4 q12 = get(x1, y2);
	vec4 q22 = get(x2, y2);
	return (
		q11 * x2x * y2y +
		q21 * xx1 * y2y +
		q12 * x2x * yy1 +
		q22 * xx1 * yy1
	);
}

// Original source: https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_decode.cpp
PNGImage loadPNGFile(string filename, bool flip_handedness) {
	vector<unsigned char> png;
	vector<unsigned char> pixels; //the raw pixels
	uint width, height;

	//load and decode
	uint error = lodepng::load_file(png, filename);
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
	
	for (uint i = 3; i < pixels.size(); i+=4) {
		if (pixels[i] < 255) {
			image.has_transparancy = true;
			break;
		}
	}
	
	return image;
}

PNGImage* loadPNGFileDynamic(string filename, bool flip_handedness) {
	static map<string, PNGImage*> cache{};
	if (cache.find(filename) == cache.end())
		cache[filename] = loadPNGFileDynamicNoCaching(filename, flip_handedness);
	return cache[filename];
}
PNGImage* loadPNGFileDynamicNoCaching(string filename, bool flip_handedness) {
	PNGImage* out = new PNGImage;
	*out = loadPNGFile(filename, flip_handedness);
	return out;
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
