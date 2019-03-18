#pragma once

#include "../sceneGraph.hpp"
#include <string>
#include <map>

#define DEBUG false

struct Material {
	vec4 basecolor;
	float shininess = 10;
	int texture_id = -1;
	
	static Material diffuse(vec4 color, float shininess = 10){
		Material out;
		out.basecolor = color;
		out.shininess = shininess;
		return out;
	}
};

SceneNode* loadModelScene(
	const std::string& filename,
	const std::map<int, Material>& overrides={});
