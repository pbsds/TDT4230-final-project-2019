#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 UV;
in layout(location = 3) vec3 tangent;
in layout(location = 4) vec3 bitangent;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 MVnormal;
uniform bool isIlluminated;
uniform bool isTextured;
uniform bool isNormalMapped;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec3 vertex_out;
out layout(location = 2) vec2 uv_out;
out layout(location = 3) mat3 TBN;

void main() {
    TBN = mat3(
        normalize(vec3(MV * vec4(tangent,   0.0))),
        normalize(vec3(MV * vec4(bitangent, 0.0))),
        normalize(vec3(MV * vec4(normal_in, 0.0)))
    );
    
    normal_out = normalize(vec3(MVnormal * vec4(normal_in, 1.0f)));
    vertex_out = vec3(MV*vec4(position, 1.0f));
    uv_out = UV;
    gl_Position =  MVP * vec4(position, 1.0f);
}
