#version 430 core

layout(location = 0) out vec4 color_out;

layout(binding = 0) uniform sampler2D framebuffer;
layout(binding = 1) uniform sampler2D depthbuffer;

uniform uint windowWidth;
uniform uint windowHeight;

uniform float time;

void main() {
    vec2 dx = vec2(1,0) * 1.0/windowWidth;
    vec2 dy = vec2(0,1) * 1.0/windowHeight;
    vec2 UV = gl_FragCoord.xy / vec2(windowWidth, windowHeight);

    float z = pow(texture(depthbuffer, UV).r , 0x800);
    z = abs(z*2-1)*1.2;
    
    int radius = int(5*z);
    vec3 color = vec3(0);
    for (int x = -radius; x <= radius; x++)
    for (int y = -radius; y <= radius; y++){
        vec2 p = UV + x*dx + y*dy;
        color += texture(framebuffer, p);
    }
    color /= pow(2*radius+1, 2);
    color_out = vec4(color * (1-pow(length((UV-0.5)*1.2), 3)), 1.0); // vignette
}
