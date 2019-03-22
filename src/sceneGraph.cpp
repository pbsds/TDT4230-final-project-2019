#include "sceneGraph.hpp"
#include <iostream>

SceneNode::SceneNode(SceneNodeType type) {
	nodeType = type;
}

void SceneNode::setMesh(const Mesh* mesh) {
	static map<const Mesh*, int> cache;

	if (cache.find(mesh) == cache.end())
		cache[mesh] = generateBuffer(*mesh, isNormalMapped || isDisplacementMapped);

	vertexArrayObjectID = cache[mesh];
	VAOIndexCount = mesh->indices.size();
	isVertexColored = ! mesh->colors.empty();
	mesh_has_transparancy = mesh->has_transparancy;
}
void SceneNode::setTexture(
		const PNGImage* diffuse,
		const PNGImage* normal,
		const PNGImage* displacement,
		const PNGImage* reflection,
		bool texture_reset) {
	static map<const PNGImage*, int> cache;
	if (texture_reset){
		isTextured = false;
		isNormalMapped = false;
		isDisplacementMapped = false;
		isReflectionMapped = false;
		tex_has_transparancy = false;
	}

	if (diffuse) {
		if (cache.find(diffuse) == cache.end())
			cache[diffuse] = generateTexture(*diffuse);
		diffuseTextureID = cache[diffuse];
		isTextured = true;
		tex_has_transparancy = diffuse->has_transparancy;
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
void SceneNode::setMaterial(const Material& mat, bool recursive) {
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

bool SceneNode::has_no_transforms() const {
	return position.x == 0 && position.y == 0 && position.z == 0
		&& rotation.x == 0 && rotation.y == 0 && rotation.z == 0
		&& scale.x    == 1 && scale.y    == 1 && scale.z    == 1;
}

bool SceneNode::has_transparancy() const {
	return mesh_has_transparancy
		|| tex_has_transparancy
		|| opacity < 0.98;
}

SceneNode* SceneNode::clone() const {
	SceneNode* out = new SceneNode();
	*out = *this;
	out->children.clear();
	for (SceneNode* child : children) {
		out->children.push_back(child->clone());
	}
	return out;
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
