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
#include <utilities/material.hpp>
#include <vector>

using glm::vec2;
using glm::vec3;
using glm::vec4;
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
	SceneNode(SceneNodeType type = GEOMETRY);
	
	void setMesh(const Mesh* mesh);
	void setTexture(
				const PNGImage* diffuse,
				const PNGImage* normal=nullptr,
				const PNGImage* displacement=nullptr,
				const PNGImage* reflection=nullptr,
				bool texture_reset=true);
	void setMaterial(const Material& mat, bool recursive=false);
	bool has_no_transforms() const;
	bool has_transparancy() const;
	SceneNode* clone() const;
	
	// this node
	SceneNodeType nodeType;
	vector<SceneNode*> children;

	// light specific:
	uint lightID = -1;
	vec3 light_color = vec3(1.0);
	vec3 attenuation = vec3(1.0, 0.0, 0.001); // 1 / (x + y*l + z*l*l)
	float spot_cuttof_cos = glm::cos(glm::radians(1.5));
	vec3 spot_direction = vec3(0.0); // in MV space, must be normalized, automatically updated by spot_target
	SceneNode* spot_target  = nullptr; // spot will follow this node

	// The node's position and rotation relative to its parent
	vec3 position = vec3(0, 0, 0);
	vec3 rotation = vec3(0, 0, 0);
	vec3 scale    = vec3(1, 1, 1);
	vec3 referencePoint = vec3(0, 0, 0); // center of rotation, in model space

	// VAO IDs refering to a loaded Mesh and its length
	int vertexArrayObjectID = -1;
	uint VAOIndexCount = 0;

	// textures and materials
	float opacity = 1.0;
	float shininess = 1.0; // specular power
	float reflexiveness = 0.0; // 0 is no reflection, 1 is a mirror. Negative value will have it multiply with base instead
	vec3 diffuse_color  = vec3(1.0);
	vec3 emissive_color = vec3(0.5);
	vec3 specular_color = vec3(0.2);
	vec2 uvOffset = vec2(0.0, 0.0); // specular power
	uint diffuseTextureID;
	uint normalTextureID;
	uint displacementTextureID;
	float displacementCoefficient = 0.1; // in units
	uint reflectionTextureID;
	
	// has_transparancy
	const Mesh* m_gemoetry = nullptr;
	const PNGImage* t_diffuse = nullptr;
	
	// shader flags
	bool isTextured = false;
	bool isVertexColored = false;
	bool isNormalMapped = false;
	bool isDisplacementMapped = false;
	bool isReflectionMapped = false;
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
