#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;

uniform layout(location = 3) mat4 MVP;
uniform layout(location = 4) mat4 MV;
uniform layout(location = 5) mat4 MVnormal;


out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec3 vertex_out;

void main()
{
    normal_out = normalize(vec3(MVnormal * vec4(normal_in, 1.0f)));
    vertex_out = vec3(MV*vec4(position, 1.0f));
    gl_Position =  MVP * vec4(position, 1.0f);
}
