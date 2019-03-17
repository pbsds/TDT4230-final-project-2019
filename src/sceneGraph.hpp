#pragma once

#include <assert.h>
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

using glm::vec2;
using glm::vec3;
using glm::mat4;
using std::map;
using std::vector;
typedef unsigned int uint;

enum SceneNodeType {
	GEOMETRY,
	POINT_LIGHT,
	SPOT_LIGHT,
};

struct SceneNode {
	SceneNode(SceneNodeType type = GEOMETRY) {
        nodeType = type;
	}
	
	void setMesh(Mesh* mesh) {
		static map<Mesh*, int> cache;

		if (cache.find(mesh) == cache.end())
			cache[mesh] = generateBuffer(*mesh, isNormalMapped || isDisplacementMapped);

		vertexArrayObjectID = cache[mesh];
		VAOIndexCount = mesh->indices.size();
	}
	void setTexture(PNGImage* diffuse, PNGImage* normal=nullptr, PNGImage* displacement=nullptr) {
		static map<PNGImage*, int> cache;
		assert(vertexArrayObjectID==-1);
		isTextured = false;
		isNormalMapped = false;
		isDisplacementMapped = false;

		if (diffuse) {
			if (cache.find(diffuse) == cache.end())
				cache[diffuse] = generateTexture(*diffuse);
			diffuseTextureID = cache[diffuse];
			isTextured = true;
		}
		
		if (normal) {
			if (cache.find(normal) == cache.end())
				cache[normal] = generateTexture(*normal);
			normalTextureID = cache[normal];
			isNormalMapped  = true;
		}
		
		if (displacement) {
			if (cache.find(displacement) == cache.end())
				cache[displacement] = generateTexture(*displacement);
			displacementTextureID = cache[displacement];
			isDisplacementMapped  = true;
		}
	}
	
	// this node
	SceneNodeType nodeType;
	vector<SceneNode*> children;

	// light specific:
	uint  lightID        = -1;
	vec3  color_emissive = vec3(0.0);
	vec3  color_diffuse  = vec3(0.0);
	vec3  color_specular = vec3(0.0);
	vec3  attenuation    = vec3(1.0, 0.0, 0.001); // 1 / (x + y*l + z*l*l)
	float spot_cuttof_angle = glm::radians(1.5); // radians
	SceneNode* targeted_by  = nullptr; // spot will follow this node

	// The node's position and rotation relative to its parent
	vec3 position = vec3(0, 0, 0);
	vec3 rotation = vec3(0, 0, 0); // also used as spot-target
	vec3 scale    = vec3(1, 1, 1);
	vec3 referencePoint = vec3(0, 0, 0); // center of rotation, in model space

	// VAO IDs refering to a loaded Mesh and its length
	int vertexArrayObjectID = -1;
	uint VAOIndexCount = 0;

	// textures
	float shinyness = 10.0; // specular power
	vec2 uvOffset = vec2(0.0, 0.0); // specular power
	uint diffuseTextureID;
	uint normalTextureID;
	uint displacementTextureID;
	float displacementCoefficient = 0.1; // in units

	// shader flags
	bool isTextured = false;
	bool isNormalMapped = false;
	bool isDisplacementMapped = false;
	bool isIlluminated = true;
	bool isInverted = false;

	// rendering
	Gloom::Shader* shader = nullptr;
	mat4 MVP; // MVP
	mat4 MV; // MV
	mat4 MVnormal; // transpose(inverse(MV))

};

// Struct for keeping track of 2D coordinates

SceneNode* createSceneNode();
SceneNode* createSceneNode(SceneNodeType type);
void addChild(SceneNode* parent, SceneNode* child);
void printNode(SceneNode* node);
