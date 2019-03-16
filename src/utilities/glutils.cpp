#include <vector>
#include <glad/glad.h>
#include <program.hpp>
#include "glutils.h"

using std::vector;
using glm::vec3;
using glm::vec2;
typedef unsigned int uint;

uint generateBuffer(Mesh &mesh, bool isNormalMapped) {
    uint vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    uint vertexBufferID;
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(vec3), mesh.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    uint normalBufferID;
    glGenBuffers(1, &normalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(vec3), mesh.normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    uint indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint), mesh.indices.data(), GL_STATIC_DRAW);

    if (mesh.textureCoordinates.empty()) return vaoID;
    
    uint textureBufferID;
    glGenBuffers(1, &textureBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, textureBufferID);
    glBufferData(GL_ARRAY_BUFFER, mesh.textureCoordinates.size() * sizeof(vec2), mesh.textureCoordinates.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(2);

    if (isNormalMapped) addTangents(vaoID, mesh);

    return vaoID;
}

void addTangents(uint vaoID, Mesh& mesh) {
    vector<vec3> tangents(mesh.vertices.size());
    vector<vec3> bitangents(mesh.vertices.size());
    
    for (uint i = 0; i < mesh.indices.size(); i+=3) {
        const vec3& pos1 = mesh.vertices[mesh.indices[i+0]];
        const vec3& pos2 = mesh.vertices[mesh.indices[i+1]];
        const vec3& pos3 = mesh.vertices[mesh.indices[i+2]];
        
        const vec2& uv1 = mesh.textureCoordinates[mesh.indices[i+0]];
        const vec2& uv2 = mesh.textureCoordinates[mesh.indices[i+1]];
        const vec2& uv3 = mesh.textureCoordinates[mesh.indices[i+2]];
        
        vec3 edge1 = pos2 - pos1;
        vec3 edge2 = pos3 - pos1;
        vec2 deltaUV1 = uv2 - uv1;
        vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        
        vec3 tangent{
            f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
            f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
            f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z),
        };
        vec3 bitangent{
            f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x),
            f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y),
            f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z),
        };
        
        tangent = glm::normalize(tangent);
        bitangent = glm::normalize(bitangent);
        
        // handedness
        tangents[mesh.indices[i+0]] = tangent;
        tangents[mesh.indices[i+1]] = tangent;
        tangents[mesh.indices[i+2]] = tangent;
        bitangents[mesh.indices[i+0]] = bitangent;
        bitangents[mesh.indices[i+1]] = bitangent;
        bitangents[mesh.indices[i+2]] = bitangent;
    }
    
    glBindVertexArray(vaoID);
    
    uint tangentBufferID;
    glGenBuffers(1, &tangentBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, tangentBufferID);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(vec3), tangents.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(3);
    
    uint bitangentBufferID;
    glGenBuffers(1, &bitangentBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bitangentBufferID);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(vec3), bitangents.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(4);
}

uint generateTexture(PNGImage& texture, bool mirrored) {
    uint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (mirrored) ? GL_MIRRORED_REPEAT : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (mirrored) ? GL_MIRRORED_REPEAT : GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.pixels.data());
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.pixels.data());
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    return id;
}
