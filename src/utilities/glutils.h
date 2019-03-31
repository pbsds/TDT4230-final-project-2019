#pragma once

#include "mesh.h"
#include "imageLoader.hpp"


unsigned int generateBuffer(const Mesh &mesh, bool doAddTangents=false);

void addTangents(unsigned int vaoID, const Mesh& mesh);

unsigned int generateTexture(const PNGImage& texture);

uint generatePostQuadBuffer();
