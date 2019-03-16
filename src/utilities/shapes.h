#pragma once
#include "mesh.h"

Mesh generateBox(float width, float height, float depth, bool flipFaces = false);
Mesh generateSphere(float radius, int slices, int layers);
Mesh generateSegmentedPlane(float width, float height, uint x_segments, uint y_segments, float uv_scale = 1.0);
