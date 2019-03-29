#pragma once
#include <GLFW/glfw3.h>

// prettifies some of the maths with vectors

constexpr glm::vec2 operator*(glm::vec2 lhs, double rhs) { return lhs *= rhs; }
constexpr glm::vec2 operator/(glm::vec2 lhs, double rhs) { return lhs /= rhs; }
constexpr glm::vec3 operator*(glm::vec3 lhs, double rhs) { return lhs *= rhs; }
constexpr glm::vec3 operator/(glm::vec3 lhs, double rhs) { return lhs /= rhs; }
constexpr glm::vec4 operator*(glm::vec4 lhs, double rhs) { return lhs *= rhs; }
constexpr glm::vec4 operator/(glm::vec4 lhs, double rhs) { return lhs /= rhs; }

constexpr glm::vec2 operator*(double lhs, glm::vec2 rhs) { return rhs * lhs; }
constexpr glm::vec2 operator/(double lhs, glm::vec2 rhs) { return rhs / lhs; }
constexpr glm::vec3 operator*(double lhs, glm::vec3 rhs) { return rhs * lhs; }
constexpr glm::vec3 operator/(double lhs, glm::vec3 rhs) { return rhs / lhs; }
constexpr glm::vec4 operator*(double lhs, glm::vec4 rhs) { return rhs * lhs; }
constexpr glm::vec4 operator/(double lhs, glm::vec4 rhs) { return rhs / lhs; }

constexpr glm::vec2 flip(glm::vec2 a) { return {a.y, a.x}; }
