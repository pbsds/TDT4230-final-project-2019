#pragma once

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <map>
#include <stack>
#include <stdbool.h>
#include <utilities/glutils.h>
#include <utilities/shader.hpp>
#include <vector>

using glm::vec3;
using glm::mat4;
using std::map;
using std::vector;
typedef unsigned int uint;

enum SceneNodeType {
	GEOMETRY, POINT_LIGHT, SPOT_LIGHT, HUD, TEXTURED_GEOMETRY, NORMAL_TEXTURED_GEOMETRY
};

struct SceneNode {
	SceneNode(SceneNodeType type = GEOMETRY) {
        nodeType = type;
	}
	
	void setMesh(Mesh* mesh) {
		static map<Mesh*, int> cache;

		if (cache.find(mesh) == cache.end())
			cache[mesh] = generateBuffer(*mesh, nodeType==NORMAL_TEXTURED_GEOMETRY);

		vertexArrayObjectID = cache[mesh];
		VAOIndexCount = mesh->indices.size();
	}
	void setTexture(PNGImage* diffuse, PNGImage* normal = nullptr) {
		static map<PNGImage*, int> cache;

		if (cache.find(diffuse) == cache.end()) cache[diffuse] = generateTexture(*diffuse);
		diffuseTextureID = cache[diffuse];
		
		if (!normal) return;
		if (cache.find(normal)  == cache.end()) cache[normal]  = generateTexture(*normal);
		normalTextureID  = cache[normal];
	}
	
	vector<SceneNode*> children;

	// The node's position and rotation relative to its parent
	vec3 position = vec3(0, 0, 0);
	vec3 rotation = vec3(0, 0, 0);
	vec3 scale    = vec3(1, 1, 1);

	// set this if the shape uses a custom shader other than the inherited one
	Gloom::Shader* shader = nullptr;
	
	// A transformation matrix representing the transformation of the node's location relative to its parent. This matrix is updated every frame.
	mat4 MVP; // MVP
	mat4 MV; // MV
	mat4 MVnormal; // transpose(inverse(MV))

	// The location of the node's reference point (center of rotation)
	vec3 referencePoint = vec3(0, 0, 0);

	// VAO IDs refering to a loaded Mesh and its length
	int vertexArrayObjectID = -1;
	uint VAOIndexCount = 0;
	
	// textures
	uint diffuseTextureID;
	uint normalTextureID;
	
	// shader flags
	bool isIlluminated = true;
	bool isInverted = false;

	// Node type is used to determine how to handle the contents of a node
	SceneNodeType nodeType;

	// for lights:
	uint lightID;
	SceneNode* targeted_by = nullptr; // spot
};

// Struct for keeping track of 2D coordinates

SceneNode* createSceneNode();
SceneNode* createSceneNode(SceneNodeType type);
void addChild(SceneNode* parent, SceneNode* child);
void printNode(SceneNode* node);
