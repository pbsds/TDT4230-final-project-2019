#version 430 core

in layout(location = 0) vec3 vertex;
out layout(location = 0) vec2 UV;

void main() {
    gl_Position = vec4(vertex, 1.0);
    UV = vertex.xy*0.5 + 0.5;
}
