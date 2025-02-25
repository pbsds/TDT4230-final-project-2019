#include <iostream>
#include "shapes.h"

typedef uint uint;
using glm::vec3;
using glm::vec2;
using std::vector;

Mesh generateBox(float width, float height, float depth, bool flipFaces) {
    // Hardcoded. Sue me.

    // Edit: well, that backfired..

    vector<vec3> vertices = {
            {0,     0,      0},
            {0,     0,      depth},
            {0,     height, depth},

            {0,     0,      0},
            {0,     height, depth},
            {0,     height, 0},

            {width, 0,      0},
            {width, height, depth},
            {width, 0,      depth},

            {width, 0,      0},
            {width, height, 0},
            {width, height, depth},

            {0,     0,      0},
            {width, height, 0},
            {width, 0,      0},

            {0,     0,      0},
            {0,     height, 0},
            {width, height, 0},

            {0,     0,      depth},
            {width, 0,      depth},
            {width, height, depth},

            {0,     0,      depth},
            {width, height, depth},
            {0,     height, depth},

            {0,     0,      0},
            {width, 0,      0},
            {width, 0,      depth},

            {0,     0,      0},
            {width, 0,      depth},
            {0,     0,      depth},

            {width, height, 0},
            {0,     height, 0},
            {0,     height, depth},

            {width, height, 0},
            {0,     height, depth},
            {width, height, depth},
    };

    // These are technically inverted relative to the vertex coordinates.
    // But for some strange reason the faces are rendered inverted.
    // So to make the assignment work this is the best I can do.

    vector<vec3> normals = {
            {1.0, 0.0, 0.0},
            {1.0, 0.0, 0.0},
            {1.0, 0.0, 0.0},

            {1.0, 0.0, 0.0},
            {1.0, 0.0, 0.0},
            {1.0, 0.0, 0.0},

            {-1.0, 0.0, 0.0},
            {-1.0, 0.0, 0.0},
            {-1.0, 0.0, 0.0},

            {-1.0, 0.0, 0.0},
            {-1.0, 0.0, 0.0},
            {-1.0, 0.0, 0.0},

            {0.0, 0.0, 1.0},
            {0.0, 0.0, 1.0},
            {0.0, 0.0, 1.0},

            {0.0, 0.0, 1.0},
            {0.0, 0.0, 1.0},
            {0.0, 0.0, 1.0},

            {0.0, 0.0, -1.0},
            {0.0, 0.0, -1.0},
            {0.0, 0.0, -1.0},

            {0.0, 0.0, -1.0},
            {0.0, 0.0, -1.0},
            {0.0, 0.0, -1.0},

            {0.0, 1.0, 0.0},
            {0.0, 1.0, 0.0},
            {0.0, 1.0, 0.0},

            {0.0, 1.0, 0.0},
            {0.0, 1.0, 0.0},
            {0.0, 1.0, 0.0},

            {0.0, -1.0, 0.0},
            {0.0, -1.0, 0.0},
            {0.0, -1.0, 0.0},

            {0.0, -1.0, 0.0},
            {0.0, -1.0, 0.0},
            {0.0, -1.0, 0.0},
    };

    float texScaleFactorX = depth / height;
    float texScaleFactorY = width / depth;
    float texScaleFactorZ = width / height;
    
    vector<vec2> textureCoordinates = {
            {0, 0},
            {texScaleFactorX, 0},
            {texScaleFactorX, 1},

            {0, 0},
            {texScaleFactorX, 1},
            {0, 1},

            {0, 0},
            {texScaleFactorX, 1},
            {texScaleFactorX, 0},

            {0, 0},
            {0, 1},
            {texScaleFactorX, 1},

            {0, 0},
            {texScaleFactorZ, 0},
            {texScaleFactorZ, 1},

            {0, 0},
            {texScaleFactorZ, 1},
            {0, 1},

            {0, 0},
            {texScaleFactorZ, 0},
            {texScaleFactorZ, 1},

            {0, 0},
            {texScaleFactorZ, 1},
            {0, 1},

            {0, 0},
            {texScaleFactorY, 0},
            {texScaleFactorY, 1},

            {0, 0},
            {texScaleFactorY, 1},
            {0, 1},

            {0, 0},
            {texScaleFactorY, 0},
            {texScaleFactorY, 1},

            {0, 0},
            {texScaleFactorY, 1},
            {0, 1},

    };


    vector<uint> indices = {
            0, 1, 2,
            3, 4, 5,
            6, 7, 8,
            9, 10, 11,
            12, 13, 14,
            15, 16, 17,
            18, 19, 20,
            21, 22, 23,
            24, 25, 26,
            27, 28, 29,
            30, 31, 32,
            33, 34, 35
    };

    if(flipFaces) {
        for(int i = 0; i < 36; i += 3) {
            uint temp = indices[i + 1];
            indices[i + 1] = indices[i + 2];
            indices[i + 2] = temp;

            normals[i + 0] *= -1;
            normals[i + 1] *= -1;
            normals[i + 2] *= -1;
        }
    }
    
    //2fix4u
    for(vec3& normal : normals) normal *= -1;

    Mesh mesh;
    mesh.vertices = vertices;
    mesh.normals = normals;
    mesh.textureCoordinates = textureCoordinates;
    mesh.indices = indices;

    return mesh;
}

Mesh generateSphere(float sphereRadius, int slices, int layers) {
    const uint triangleCount = slices * layers * 2;

    vector<vec3> vertices;
    vector<vec3> normals;
    vector<uint> indices;
    vector<vec2> texcoords;

    vertices.reserve(3 * triangleCount);
    normals.reserve(3 * triangleCount);
    indices.reserve(3 * triangleCount);
    texcoords.reserve(3 * triangleCount);

    // Slices require us to define a full revolution worth of triangles.
    // Layers only requires angle varying between the bottom and the top (a layer only covers half a circle worth of angles)
    const float degreesPerLayer = 180.0 / (float) layers;
    const float degreesPerSlice = 360.0 / (float) slices;

    uint i = 0;

    // Constructing the sphere one layer at a time
    for (int layer = 0; layer < layers; layer++) {
        int nextLayer = layer + 1;

        // Angles between the vector pointing to any point on a particular layer and the negative z-axis
        float currentAngleZDegrees = degreesPerLayer * layer;
        float nextAngleZDegrees = degreesPerLayer * nextLayer;

        // All coordinates within a single layer share z-coordinates.
        // So we can calculate those of the current and subsequent layer here.
        float currentZ = -cos(glm::radians(currentAngleZDegrees));
        float nextZ = -cos(glm::radians(nextAngleZDegrees));

        // The row of vertices forms a circle around the vertical diagonal (z-axis) of the sphere.
        // These radii are also constant for an entire layer, so we can precalculate them.
        float radius = sin(glm::radians(currentAngleZDegrees));
        float nextRadius = sin(glm::radians(nextAngleZDegrees));

        // Now we can move on to constructing individual slices within a layer
        for (int slice = 0; slice < slices; slice++) {

            // The direction of the start and the end of the slice in the xy-plane
            float currentSliceAngleDegrees = slice * degreesPerSlice;
            float nextSliceAngleDegrees = (slice + 1) * degreesPerSlice;

            // Determining the direction vector for both the start and end of the slice
            float currentDirectionX = cos(glm::radians(currentSliceAngleDegrees));
            float currentDirectionY = sin(glm::radians(currentSliceAngleDegrees));

            float nextDirectionX = cos(glm::radians(nextSliceAngleDegrees));
            float nextDirectionY = sin(glm::radians(nextSliceAngleDegrees));

            vertices.emplace_back(sphereRadius * radius * currentDirectionX,
                                  sphereRadius * radius * currentDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * radius * nextDirectionX,
                                  sphereRadius * radius * nextDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * nextRadius * nextDirectionX,
                                  sphereRadius * nextRadius * nextDirectionY,
                                  sphereRadius * nextZ);
            vertices.emplace_back(sphereRadius * radius * currentDirectionX,
                                  sphereRadius * radius * currentDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * nextRadius * nextDirectionX,
                                  sphereRadius * nextRadius * nextDirectionY,
                                  sphereRadius * nextZ);
            vertices.emplace_back(sphereRadius * nextRadius * currentDirectionX,
                                  sphereRadius * nextRadius * currentDirectionY,
                                  sphereRadius * nextZ);

            normals.emplace_back(radius * currentDirectionX,
                                 radius * currentDirectionY,
                                 currentZ);
            normals.emplace_back(radius * nextDirectionX,
                                 radius * nextDirectionY,
                                 currentZ);
            normals.emplace_back(nextRadius * nextDirectionX,
                                 nextRadius * nextDirectionY,
                                 nextZ);
            normals.emplace_back(radius * currentDirectionX,
                                 radius * currentDirectionY,
                                 currentZ);
            normals.emplace_back(nextRadius * nextDirectionX,
                                 nextRadius * nextDirectionY,
                                 nextZ);
            normals.emplace_back(nextRadius * currentDirectionX,
                                 nextRadius * currentDirectionY,
                                 nextZ);

            indices.emplace_back(i + 0);
            indices.emplace_back(i + 1);
            indices.emplace_back(i + 2);
            indices.emplace_back(i + 3);
            indices.emplace_back(i + 4);
            indices.emplace_back(i + 5);
            
            texcoords.emplace_back( currentSliceAngleDegrees /360., currentAngleZDegrees /180. );
            texcoords.emplace_back(    nextSliceAngleDegrees /360., currentAngleZDegrees /180. );
            texcoords.emplace_back(    nextSliceAngleDegrees /360.,    nextAngleZDegrees /180. );
            texcoords.emplace_back( currentSliceAngleDegrees /360., currentAngleZDegrees /180. );
            texcoords.emplace_back(    nextSliceAngleDegrees /360.,    nextAngleZDegrees /180. );
            texcoords.emplace_back( currentSliceAngleDegrees /360.,    nextAngleZDegrees /180. );
            
            i += 6;
        }
    }

    Mesh mesh;
    mesh.vertices = vertices;
    mesh.normals = normals;
    mesh.indices = indices;
    mesh.textureCoordinates = texcoords;
    return mesh;
}

Mesh generateSegmentedPlane(float width, float height, uint x_segments, uint y_segments, float uv_scale) {
    // i should perhaps initialize these to the length they should be, but insert is prettier :3
    vector<vec3> vertices;
    vector<vec3> normals;
    vector<vec2> textureCoordinates;
    vector<uint> indices;
    
    float step_x = width/x_segments;
    float step_y = height/y_segments;
    float tex_step_x = uv_scale/x_segments;
    float tex_step_y = uv_scale/y_segments;
    uint index_offset = 0;
    
    for (uint x = 0; x < x_segments; x++)
    for (uint y = 0; y < y_segments; y++) {
        vertices.insert(vertices.end(), {
            vec3(step_x*(x+0), step_y*(y+0), 0),
            vec3(step_x*(x+0), step_y*(y+1), 0),
            vec3(step_x*(x+1), step_y*(y+0), 0),
            vec3(step_x*(x+1), step_y*(y+1), 0),
        });
        normals.insert(normals.end(), {
            vec3(0.0, 0.0, 1.0),
            vec3(0.0, 0.0, 1.0),
            vec3(0.0, 0.0, 1.0),
            vec3(0.0, 0.0, 1.0),
        });
        textureCoordinates.insert(textureCoordinates.end(), {
            vec2(tex_step_x*(x+0), tex_step_y*(y+0)),
            vec2(tex_step_x*(x+0), tex_step_y*(y+1)),
            vec2(tex_step_x*(x+1), tex_step_y*(y+0)),
            vec2(tex_step_x*(x+1), tex_step_y*(y+1)),
        });
        indices.insert(indices.end(), {
            index_offset + 0, index_offset + 3, index_offset + 1,
            index_offset + 0, index_offset + 2, index_offset + 3,
        });
        index_offset += 4;
    }

    Mesh mesh;
    mesh.vertices = vertices;
    mesh.normals = normals;
    mesh.textureCoordinates = textureCoordinates;
    mesh.indices = indices;

    return mesh;

}
