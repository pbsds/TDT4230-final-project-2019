#include "modelLoader.hpp"

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/scene.h>       // aiScene and aiNode
#include <assimp/material.h>    // aiMaterial
#include <assimp/mesh.h>        // aiMesh
#include <assimp/texture.h>     // aiTexture
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>
#include "mesh.h"
#include "imageLoader.hpp"

typedef unsigned int uint;
using std::vector;
using std::map;
using std::cout;
using std::cerr;
using std::endl;

SceneNode* buildSceneNodes(
		const aiNode* node,
		const vector<Mesh>& meshes,
		const vector<Material*>& mat_lookup) {
	if (DEBUG) cout << "Building node from " << node->mName.data << "..." << endl;
	
	// filter semantic-only nodes
	if (node->mTransformation.IsIdentity()
			&& node->mNumMeshes == 0
			&& node->mNumChildren == 1)
		return buildSceneNodes(node->mChildren[0], meshes, mat_lookup);
	
	SceneNode* out = createSceneNode();

	if (!node->mTransformation.IsIdentity()) {
		aiQuaterniont<float> rotation;
		aiVector3t<float> position, scaling;
		node->mTransformation.Decompose(scaling, rotation, position);
		for(uint i=0; i<3; i++) out->position[i] = position[i];
		for(uint i=0; i<3; i++) out->scale[i]    = scaling[i];
		out->rotation = glm::eulerAngles(glm::quat(
			rotation.w, rotation.x, rotation.y, rotation.z));
	}

	for (uint i = 0; i < node->mNumMeshes; i++) {
		SceneNode* mesh_node = createSceneNode();
		out->children.push_back(mesh_node);
		uint meshidx = node->mMeshes[i];
		
		mesh_node->setMaterial(*mat_lookup[meshidx]);
		mesh_node->setMesh(&meshes[meshidx]);
		
	}

	for (uint i=0; i<node->mNumChildren; i++)
		out->children.push_back(buildSceneNodes(node->mChildren[i], meshes, mat_lookup));
	
	return out;
}

SceneNode* loadModelScene(const std::string& dirname, const std::string& filename, const map<int, Material>& overrides) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(dirname + "/" + filename,
		aiProcess_CalcTangentSpace
		//| aiProcess_FlipWindingOrder
		| aiProcess_Triangulate
		| aiProcess_GenNormals
		| aiProcess_ImproveCacheLocality
		//| aiProcess_JoinIdenticalVertices
		| aiProcess_SortByPType
	);
	if (!scene) {
		cerr << importer.GetErrorString() << endl;
		throw 1;
	}
	
	// read materials
	uint j=0;
	Material default_material;
	vector<Material> materials(scene->mNumMaterials);
	for (Material& material : materials) {
		const aiMaterial* aimat = scene->mMaterials[j++];
		
		// print material
		aiString name; aimat->Get(AI_MATKEY_NAME, name);
		if (DEBUG){
			cout << "Read material #" << j-1 << " '" << name.data << "':" << endl;
			for (uint i=0; i < aimat->mNumProperties; i++)
			cout << "   " << aimat->mProperties[i]->mKey.data << endl;
		}
		
		aiColor3D color (1,1,1);
		aiColor4D color4 (1,1,1,0);
		if (AI_SUCCESS == aimat->Get(AI_MATKEY_COLOR_DIFFUSE, color))
			material.diffuse_color = {color.r, color.g, color.b};
		if (AI_SUCCESS == aimat->Get(AI_MATKEY_COLOR_EMISSIVE, color))
			material.emissive_color = {color.r, color.g, color.b};
		if (AI_SUCCESS == aimat->Get(AI_MATKEY_COLOR_SPECULAR, color))
			material.specular_color = {color.r, color.g, color.b};
		//if (AI_SUCCESS == aiGetMaterialColor(aimat, "$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 0, &color4))
		//	material.diffuse_color = {color4.r, color4.g, color4.b};
		//if (AI_SUCCESS == aiGetMaterialColor(aimat, "$mat.gltf.pbrMetallicRoughness.metallicFactor", 0, 0, &color4))
		//	material.emissive_color *= vec3{color4.r, color4.g, color4.b};
		//if (AI_SUCCESS == aiGetMaterialColor(aimat, "$mat.gltf.pbrMetallicRoughness.roughnessFactor", 0, 0, &color4))
		//	material.specular_color = {color4.r, color4.g, color4.b};
		
		aimat->Get(AI_MATKEY_SHININESS, material.shininess);
		//todo: opacity?
		
		if (aimat->GetTextureCount(aiTextureType_DIFFUSE)) {
			aiString path; aimat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			if (DEBUG) cout << "  diffuse texture path: " << dirname << "/" << path.data << endl;
			material.diffuse_texture = loadPNGFileDynamic(dirname + "/" + path.data);
		}
		if (aimat->GetTextureCount(aiTextureType_NORMALS)) {
			aiString path; aimat->GetTexture(aiTextureType_NORMALS, 0, &path);
			if (DEBUG) cout << "  normal texture path: " << dirname << "/" << path.data << endl;
			material.normal_texture = loadPNGFileDynamic(dirname + "/" + path.data);
		}
		if (aimat->GetTextureCount(aiTextureType_DISPLACEMENT)) {
			aiString path; aimat->GetTexture(aiTextureType_DISPLACEMENT, 0, &path);
			if (DEBUG) cout << "  displacement texture path: " << dirname << "/" << path.data << endl;
			material.displacement_texture = loadPNGFileDynamic(dirname + "/" + path.data);
		}
		if (aimat->GetTextureCount(aiTextureType_REFLECTION)) {
			aiString path; aimat->GetTexture(aiTextureType_REFLECTION, 0, &path);
			if (DEBUG) cout << "  displacement texture path: " << dirname << "/" << path.data << endl;
			material.reflection_texture = loadPNGFileDynamic(dirname + "/" + path.data);
		}
		
	}
	
	// apply material overriders to material list
	for (auto override : overrides) {
		Material& mat = (override.first>=0) 
			? materials[override.first] 
			: default_material;
		mat = mat.apply(override.second);
	}
	
	vector<Mesh>     meshes(  scene->mNumMeshes);
	vector<Material*> mat_lookup(scene->mNumMeshes); // to be passed to buildSceneNodes

	// read meshes
	j=0;
	for (Mesh& mesh : meshes) {
		const aiMesh* aimesh = scene->mMeshes[j++];
		for (uint i=0;  i < aimesh->mNumVertices; i++){
			mesh.vertices.push_back({
				aimesh->mVertices[i].x,
				aimesh->mVertices[i].y,
				aimesh->mVertices[i].z,
			});
			mesh.normals.push_back({
				aimesh->mNormals[i].x,
				aimesh->mNormals[i].y,
				aimesh->mNormals[i].z,
			});
		}

		mat_lookup[j-1] = (aimesh->mMaterialIndex < meshes.size())
			? &materials[aimesh->mMaterialIndex]
			: &default_material;
		
		if (aimesh->GetNumUVChannels() >= 1)
		for (uint i=0;  i < aimesh->mNumVertices; i++){
			//assert(aimesh->mNumUVComponents[0] == 2);
			mesh.textureCoordinates.push_back({
				aimesh->mTextureCoords[0][i].x,
				aimesh->mTextureCoords[0][i].y,
			});
		}

		if (aimesh->GetNumColorChannels() >= 1)
		for (uint i=0;  i < aimesh->mNumVertices; i++){
			mesh.colors.push_back({
				aimesh->mColors[0][i].r,
				aimesh->mColors[0][i].g,
				aimesh->mColors[0][i].b,
				aimesh->mColors[0][i].a,
			});
		}
		
		for (uint i=0;  i < aimesh->mNumFaces; i++){
			assert(aimesh->mFaces[i].mNumIndices == 3);
			mesh.indices.insert(mesh.indices.end(), {
				aimesh->mFaces[i].mIndices[0],
				aimesh->mFaces[i].mIndices[1],
				aimesh->mFaces[i].mIndices[2],
			});
		}
	}
	
	// build scene node tree:
	SceneNode* out = buildSceneNodes(scene->mRootNode, meshes, mat_lookup);
	out->rotation.x += M_PI/2; // account for my weird coordinates. Z is upward damnit!
	return out;
}
