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
layout(binding = 3) uniform sampler2D reflectionTexture;
uniform float displacementCoefficient;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 MVnormal;

// material
uniform float opacity;
uniform float shininess;
uniform float backlight_strength;
uniform float reflexiveness;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform vec3 emissive_color;
uniform vec3 backlight_color;

uniform bool isIlluminated;
uniform bool isTextured;
uniform bool isVertexColored;
uniform bool isNormalMapped;
uniform bool isDisplacementMapped;
uniform bool isReflectionMapped;
uniform bool isInverted;

// lights
struct Light { // point lights, coordinates in MV space
    vec3 position;
    vec3 attenuation; // 1 / (x + y*l + z*l*l)
    vec3 color;
    
    bool is_spot; // false means point light
    vec3 spot_direction;
    float spot_cuttof_cos;
};

#define N_LIGHTS 7
uniform Light light[N_LIGHTS];


layout(location = 0) out vec4 color_out;


vec3 reflection(vec3 basecolor, vec3 nnormal) {
    vec3 up    = normalize(vec3(MVnormal * vec4(vec3(0.0, 0.0, 1.0), 1.0)));
    vec3 north = normalize(vec3(MVnormal * vec4(vec3(1.0, 0.0, 0.0), 1.0)));
    float u = acos(dot(reflect(normalize(vertex), nnormal), north)) / -3.141592;
    float v = acos(dot(reflect(normalize(vertex), nnormal), up   )) / -3.141592;
    vec3 reflection = texture(reflectionTexture, vec2(u, v)).rgb;
    return (reflexiveness < 0) 
        ? basecolor * mix(vec3(0.0), reflection, -reflexiveness)
        : mix(basecolor, reflection, reflexiveness);
}

vec3 get_nnormal() {
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
        return TBN * normalize(texture(normalTexture, UV).rgb * 2.0 - 1.0);
    }
    else {
        if (isDisplacementMapped) {
            float o = texture(displacementTexture, UV).r * 2.0 - 1.0;
            float u = (texture(displacementTexture, UV + vec2(0.00001, 0.0)).r*2.0-1.0 - o) / 0.00004;
            float v = (texture(displacementTexture, UV + vec2(0.0, 0.00001)).r*2.0-1.0 - o) / 0.00004;
            return normalize(cross(tangent + normal*u, bitangent + normal*v));
        }
        else {
            return normalize(normal);
        }
    }
}

vec3 phong(vec3 basecolor, vec3 nnormal) {
    vec3 diffuse_component  = vec3(0.0);
    vec3 specular_component = vec3(0.0);

    for (int i = 0; i<N_LIGHTS; i++) {
        vec3 L = light[i].position - vertex;
        float l = length(L);
        L = normalize(L);

        if (light[i].is_spot) {
            if (dot(light[i].spot_direction, -L) < light[i].spot_cuttof_cos) {
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

        specular_component += light[i].color * specular_i * attenuation;
        if (diffuse_i>0)  diffuse_component += light[i].color * diffuse_i * attenuation;
    }

    basecolor *= (emissive_color*light[0].color + diffuse_color *diffuse_component);

    if (isReflectionMapped)
        basecolor = reflection(basecolor, nnormal);

    return basecolor + specular_color * specular_component;
}

void main() {
    vec3 nnormal = get_nnormal(); // normalized normal
    vec4 c = vec4(vec3(1.0), opacity);
    if (isVertexColored)    c *= color;
    if (isTextured)         c *= texture(diffuseTexture, UV);
    if (isInverted)         c.rgb = 1 - c.rgb;
    if (isIlluminated)      c.rgb = phong(c.rgb, nnormal);
    else {
        c.rgb *= diffuse_color;
        if (isReflectionMapped)
            c.rgb = reflection(c.rgb, normalize(normal));
    }
    if (backlight_strength > 0.05)
        c.rgb += backlight_color * clamp((dot(normalize(vertex), nnormal) + backlight_strength) / backlight_strength, 0, 1);
    //c.rgb = diffuse_color;
    //c.rgb = emissive_color;
    //c.rgb = specular_color;
    color_out = c;
}
