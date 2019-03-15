#pragma once

#include <GLFW/glfw3.h>
#include <utilities/window.hpp>

void initGame(GLFWwindow* window, CommandLineOptions options);
void updateFrame(GLFWwindow* window);
void renderFrame(GLFWwindow* window);
