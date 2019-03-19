#include "sceneGraph.hpp"
#include <iostream>

SceneNode::SceneNode(SceneNodeType type = GEOMETRY) {
	nodeType = type;
}

void SceneNode::setMesh(const Mesh* mesh) {
	static map<const Mesh*, int> cache;

	if (cache.find(mesh) == cache.end())
		cache[mesh] = generateBuffer(*mesh, isNormalMapped || isDisplacementMapped);

	vertexArrayObjectID = cache[mesh];
	VAOIndexCount = mesh->indices.size();
	isVertexColored = ! mesh->colors.empty();
}
void SceneNode::setTexture(
			const PNGImage* diffuse,
			const PNGImage* normal=nullptr,
			const PNGImage* displacement=nullptr,
			const PNGImage* reflection=nullptr,
			bool texture_reset=true) {
	static map<const PNGImage*, int> cache;
	if (texture_reset){
		isTextured = false;
		isNormalMapped = false;
		isDisplacementMapped = false;
		isReflectionMapped = false;
	}

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
	
	if (reflection) {
		if (cache.find(reflection) == cache.end())
			cache[reflection] = generateTexture(*reflection);
		reflectionTextureID = cache[reflection];
		isReflectionMapped  = true;
	}
}
void SceneNode::setMaterial(const Material& mat, bool recursive=false) {
	reflexiveness = mat.reflexiveness;
	if (!mat.ignore_diffuse)  diffuse_color  = mat.diffuse_color;
	if (!mat.ignore_emissive) emissive_color = mat.emissive_color;
	if (!mat.ignore_specular) specular_color = mat.specular_color;
	if (!mat.ignore_specular) shininess      = mat.shininess;
	setTexture(
		mat.diffuse_texture,
		mat.normal_texture,
		mat.displacement_texture,
		mat.reflection_texture,
		mat.texture_reset
	);
	if (recursive) for (SceneNode* child : children)
		child->setMaterial(mat, true);
}


SceneNode* createSceneNode() {
	return new SceneNode();
}
SceneNode* createSceneNode(SceneNodeType type) {
	return new SceneNode(type);
}

// Add a child node to its parent's list of children
void addChild(SceneNode* parent, SceneNode* child) {
	parent->children.push_back(child);
}

// Pretty prints the current values of a SceneNode instance to stdout
void printNode(SceneNode* node) {
	printf(
		"SceneNode {\n"
		"    Child count: %i\n"
		"    Rotation: (%f, %f, %f)\n"
		"    Location: (%f, %f, %f)\n"
		"    Reference point: (%f, %f, %f)\n"
		"    VAO ID: %i\n"
		"}\n",
		int(node->children.size()),
		node->rotation.x, node->rotation.y, node->rotation.z,
		node->position.x, node->position.y, node->position.z,
		node->referencePoint.x, node->referencePoint.y, node->referencePoint.z, 
		node->vertexArrayObjectID);
}
