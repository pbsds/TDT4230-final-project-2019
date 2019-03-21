#pragma once
#include <GLFW/glfw3.h>
#include <utilities/shader.hpp>
#include <utilities/window.hpp>
#include "sceneGraph.hpp"

// further divied into:
#include "scene.hpp"

void initRenderer(GLFWwindow* window, CommandLineOptions options);
void updateFrame(GLFWwindow* window, int windowWidth, int windowHeight);
void renderFrame(GLFWwindow* window, int windowWidth, int windowHeight);
