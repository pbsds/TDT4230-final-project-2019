#pragma once

#include "mesh.h"
#include "imageLoader.hpp"


unsigned int generateBuffer(Mesh &mesh, bool isNormalMapped = false);

void addTangents(unsigned int vaoID, Mesh& mesh);

unsigned int generateTexture(PNGImage& texture);
