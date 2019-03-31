#version 430 core

layout(location = 0) out vec4 color_out;

layout(binding = 0) uniform sampler2D framebuffer;
layout(binding = 1) uniform sampler2D depthbuffer;

uniform uint windowWidth;
uniform uint windowHeight;

uniform float time;

void main() {
    float dx = 1.0/windowWidth;
    float dy = 1.0/windowHeight;
    
    vec2 UV = gl_FragCoord.xy / vec2(windowWidth, windowHeight);
    color_out = texture(framebuffer, UV);
}
