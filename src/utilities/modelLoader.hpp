#pragma once

#include "../sceneGraph.hpp"
#include "material.hpp"
#include <string>
#include <map>

#define DEBUG false

SceneNode* loadModelScene(
	const std::string& dirname,
	const std::string& filename, //basename
	const std::map<int, Material>& overrides={});
