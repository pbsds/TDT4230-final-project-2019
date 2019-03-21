#pragma once
#include <GLFW/glfw3.h>
#include "sceneGraph.hpp"
#include "utilities/window.hpp"

const uint N_LIGHTS = 3;

extern SceneNode* rootNode;
extern SceneNode* hudNode;
extern SceneNode* lightNode[N_LIGHTS];

extern glm::vec3 cameraPosition;
extern glm::vec3 cameraLookAt;
extern glm::vec3 cameraUpward;

void init_scene(CommandLineOptions options);
void mouse_position_cb(double x, double y, int winw, int winh);
void step_scene(double timeDelta);
