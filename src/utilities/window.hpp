#pragma once

// System Headers
#include <glad/glad.h>

// Standard headers
#include <string>

// Constants
const int         c_windowWidth     = 1366;
const int         c_windowHeight    = 768;
const std::string c_windowTitle     = "Glowbox";
const GLint       c_windowResizable = GL_TRUE;
const int         c_windowSamples   = 4;

struct CommandLineOptions {
    bool enableMusic;
    bool enableAutoplay;
};
