#pragma once
#include <GLFW/glfw3.h>
#include "sceneGraph.hpp"
#include "utilities/window.hpp"

const uint N_LIGHTS = 7;

extern SceneNode* rootNode;
extern SceneNode* hudNode;
extern SceneNode* lightNode[N_LIGHTS];

extern vec3  fog_color;
extern float fog_strength;

extern glm::vec3 cameraPosition;
extern glm::vec3 cameraLookAt;
extern glm::vec3 cameraUpward;

void init_scene(CommandLineOptions options);

// same coords as hudNode, returns true if mouse should be disabled and centered.
// `scale` is window_height/2 
bool mouse_position_handler(double mx, double my, int scale);

void step_scene(double timeDelta);
