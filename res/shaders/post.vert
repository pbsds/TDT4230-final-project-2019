#version 430 core

in layout(location = 0) vec3 vertex;

void main() {
    gl_Position = vec4(vertex, 1.0);
}
