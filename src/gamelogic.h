#pragma once

#include <GLFW/glfw3.h>
#include <utilities/window.hpp>

void initGame(GLFWwindow* window, CommandLineOptions options);
void updateFrame(GLFWwindow* window, int windowWidth, int windowHeight);
void renderFrame(GLFWwindow* window, int windowWidth, int windowHeight);
