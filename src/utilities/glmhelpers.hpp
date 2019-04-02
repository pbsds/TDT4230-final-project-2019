#pragma once
#include <GLFW/glfw3.h>
#include <iostream>

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


inline std::ostream& operator<<(std::ostream& os, vec2 v) { return os << "vec2(" << v.x << ", " << v.y << ")"; }
inline std::ostream& operator<<(std::ostream& os, vec3 v) { return os << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")"; }
inline std::ostream& operator<<(std::ostream& os, vec4 v) { return os << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")"; }
