#version 430 core

in layout(location = 0) vec3 vertex;
in layout(location = 1) vec3 normal;
in layout(location = 2) vec2 UV;
in layout(location = 3) vec4 color;
in layout(location = 4) vec3 tangent;
in layout(location = 5) vec3 bitangent;

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D normalTexture;
layout(binding = 2) uniform sampler2D displacementTexture;
uniform float displacementCoefficient;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 MVnormal;

uniform float shininess;
uniform vec4 basecolor;

uniform bool isIlluminated;
uniform bool isTextured;
uniform bool isColorMapped;
uniform bool isNormalMapped;
uniform bool isDisplacementMapped;
uniform bool isInverted;

// lights
struct Light { // point lights, coordinates in MV space
    vec3 position;
    vec3 attenuation; // 1 / (x + y*l + z*l*l)
    vec3 color_emissive;
    vec3 color_diffuse;
    vec3 color_specular;
    
    bool is_spot; // false means point light
    vec3 spot_target;
    float spot_cuttof_angle;
};

#define N_LIGHTS 1
uniform Light light[N_LIGHTS];


out vec4 color_out;


vec4 phong(vec4 basecolor) {
    vec3 nnormal; // normalized normal
    if (isNormalMapped) {
        mat3 TBN;
        if (isDisplacementMapped) {
            float o = texture(displacementTexture, UV).r * 2.0 - 1.0;
            float u = (texture(displacementTexture, UV + vec2(0.0001, 0.0)).r*2.0-1.0 - o) / 0.0004; // magic numbers are great
            float v = (texture(displacementTexture, UV + vec2(0.0, 0.0001)).r*2.0-1.0 - o) / 0.0004; // magic numbers are great
            TBN = mat3(
                normalize(tangent   + normal*u),
                normalize(bitangent + normal*v),
                normalize(cross(tangent + normal*u, bitangent + normal*v))
            );
        }
        else {
            TBN = mat3(
                normalize(tangent),
                normalize(bitangent),
                normalize(normal)
            );
        }
        nnormal = TBN * normalize(texture(normalTexture, UV).rgb * 2.0 - 1.0);
    }
    else {
        if (isDisplacementMapped) {
            float o = texture(displacementTexture, UV).r * 2.0 - 1.0;
            float u = (texture(displacementTexture, UV + vec2(0.0001, 0.0)).r*2.0-1.0 - o) / 0.0004;
            float v = (texture(displacementTexture, UV + vec2(0.0, 0.0001)).r*2.0-1.0 - o) / 0.0004;
            nnormal = normalize(cross(tangent + normal*u, bitangent + normal*v));
        }
        else {
            nnormal = normalize(normal);
        }
    }

    vec3 emmissive_component = vec3(0.0);
    vec3 diffuse_component   = vec3(0.0);
    vec3 specular_component  = vec3(0.0);

    for (int i = 0; i<N_LIGHTS; i++) {
        vec3 L = light[i].position - vertex;
        float l = length(L);
        L = normalize(L);

        if (light[i].is_spot) {
            if (dot(normalize(light[i].position - light[i].spot_target), L) < light[i].spot_cuttof_angle) {
                continue;
            }
        }

        float attenuation = clamp(1 / (
            light[i].attenuation.x + 
            light[i].attenuation.y * l + 
            light[i].attenuation.z * l * l
            ), 0.0, 1.25);

        float diffuse_i = dot(nnormal, L);
        float specular_i = dot(reflect(-L, nnormal), -normalize(vertex));
        specular_i = (specular_i>0)
            ? pow(specular_i, shininess)
            : 0;

        emmissive_component += light[i].color_emissive;
        specular_component += light[i].color_specular * specular_i * attenuation;
        if (diffuse_i>0)  diffuse_component  += light[i].color_diffuse  * diffuse_i  * attenuation;
    }

    return vec4(basecolor.rgb * (emmissive_component + diffuse_component) + specular_component, basecolor.a);
}

void main() {
    vec4 c = basecolor;
    if (isColorMapped) c *= color;
    if (isTextured)    c *= texture(diffuseTexture, UV);
    if (isIlluminated) c = phong(c);
    if (isInverted)    c.rgb = 1 - c.rgb;
    color_out = c;
}
