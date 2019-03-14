#include <glad/glad.h>
#include <program.hpp>
#include "glutils.h"

unsigned int generateBuffer(Mesh &mesh, bool isNormalMapped) {
    unsigned int vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    unsigned int vertexBufferID;
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(glm::vec3), mesh.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    unsigned int normalBufferID;
    glGenBuffers(1, &normalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(glm::vec3), mesh.normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    unsigned int indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    if (mesh.textureCoordinates.empty()) return vaoID;
    
    unsigned int textureBufferID;
    glGenBuffers(1, &textureBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, textureBufferID);
    glBufferData(GL_ARRAY_BUFFER, mesh.textureCoordinates.size() * sizeof(glm::vec2), mesh.textureCoordinates.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(2);

    if (isNormalMapped) addTangents(vaoID, mesh);

    return vaoID;
}

void addTangents(unsigned int vaoID, Mesh& mesh) {
    glm::vec3 tangents[mesh.vertices.size()];
    glm::vec3 bitangents[mesh.vertices.size()];
    
    for (unsigned int i = 0; i < mesh.indices.size(); i+=3) {
        const glm::vec3& pos1 = mesh.vertices[mesh.indices[i+0]];
        const glm::vec3& pos2 = mesh.vertices[mesh.indices[i+1]];
        const glm::vec3& pos3 = mesh.vertices[mesh.indices[i+2]];
        
        const glm::vec2& uv1 = mesh.textureCoordinates[mesh.indices[i+0]];
        const glm::vec2& uv2 = mesh.textureCoordinates[mesh.indices[i+1]];
        const glm::vec2& uv3 = mesh.textureCoordinates[mesh.indices[i+2]];
        
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        
        glm::vec3 tangent, bitangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        
        tangent = glm::normalize(tangent);
        bitangent = glm::normalize(bitangent);
        
        // handedness
        tangents[i+0] = -tangent;
        tangents[i+1] = -tangent;
        tangents[i+2] = -tangent;
        bitangents[i+0] = -bitangent;
        bitangents[i+1] = -bitangent;
        bitangents[i+2] = -bitangent;
    }
    
    glBindVertexArray(vaoID);
    
    unsigned int tangentBufferID;
    glGenBuffers(1, &tangentBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, tangentBufferID);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(glm::vec3), tangents, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(3);
    
    unsigned int bitangentBufferID;
    glGenBuffers(1, &bitangentBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bitangentBufferID);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(glm::vec3), bitangents, GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(4);
}

unsigned int generateTexture(PNGImage& texture) {
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.pixels.data());
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.pixels.data());
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    return id;
}
