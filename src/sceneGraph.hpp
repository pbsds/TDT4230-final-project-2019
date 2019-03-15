#pragma once

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <stack>
#include <stdbool.h>
#include <utilities/shader.hpp>
#include <vector>

enum SceneNodeType {
	GEOMETRY, POINT_LIGHT, SPOT_LIGHT, HUD, TEXTURED_GEOMETRY, NORMAL_TEXTURED_GEOMETRY
};

struct SceneNode {
	SceneNode() {
		position = glm::vec3(0, 0, 0);
		rotation = glm::vec3(0, 0, 0);
		scale = glm::vec3(1, 1, 1);

        referencePoint = glm::vec3(0, 0, 0);
        vertexArrayObjectID = -1;
        VAOIndexCount = 0;

        nodeType = GEOMETRY;
		targeted_by = nullptr;
		
		isIlluminated = true;
		isInverted = false;
	}
	SceneNode(SceneNodeType type) : SceneNode() {
		nodeType = type;
	}

	std::vector<SceneNode*> children;

	// The node's position and rotation relative to its parent
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	// set this if the shape uses a custom shader other than the default one
	Gloom::Shader* shader = nullptr;
	
	// A transformation matrix representing the transformation of the node's location relative to its parent. This matrix is updated every frame.
	glm::mat4 MVP; // MVP
	glm::mat4 MV; // MV
	glm::mat4 MVnormal; // transpose(inverse(MV))

	// The location of the node's reference point
	glm::vec3 referencePoint;

	// The ID of the VAO containing the "appearance" of this SceneNode.
	int vertexArrayObjectID;
	unsigned int VAOIndexCount;
	
	unsigned int diffuseTextureID;
	unsigned int normalTextureID;
	
	bool isIlluminated;
	bool isInverted;

	// Node type is used to determine how to handle the contents of a node
	SceneNodeType nodeType;

	// for lights:
	unsigned int lightID;
	SceneNode* targeted_by; // spot
};

// Struct for keeping track of 2D coordinates

SceneNode* createSceneNode();
SceneNode* createSceneNode(SceneNodeType type);
void addChild(SceneNode* parent, SceneNode* child);
void printNode(SceneNode* node);
