#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec3 vertex;

// point lights
struct Light {
    vec3 position;
    mat4 MV;
    bool is_spot;
    vec3 spot_target; // MV space coordinates
};

#define N_LIGHTS 3

uniform Light light[N_LIGHTS];

out vec4 color;

// constants
float shininess = 15;
vec3 c_diffuse = vec3(0.75390625, 0.4296875, 0.4375);
vec3 c_emissive = vec3(0.01171875, 0.0, 0.15234375);
vec3 c_specular = vec3(0.9453125, 0.94921875, 0.84765625);

float spot_cuttof_angle = cos(4 / 180.0 * 3.1415926535);

void main()
{
    vec3 nnormal = normalize(normal);

    float diffuse_intensity = 0.0;
    float specular_intensity = 0.0;

    for (int i = 0; i<3; i++) {
        vec3 L = vec3(light[i].MV * vec4(light[i].position, 1.0f)) - vertex;
        float attenuation = clamp(1000/(1 + 40*length(L) + 0.1*length(L)*length(L)), 0.0, 1.25);
        L = normalize(L);

        if (light[i].is_spot) {
            vec3 L2 = normalize(vec3(light[i].MV * vec4(light[i].position, 1.0f)) - light[i].spot_target);
            if (dot(L2, L) < spot_cuttof_angle) {
                continue;
            }
            attenuation *= 70;
        }

        float diffuse_i = dot(nnormal, L);

        float specular_i = pow(dot(reflect(-L, nnormal), normalize(vec3(0,0,0) - vertex)), shininess);


        if (diffuse_i > 0)  diffuse_intensity  += attenuation*diffuse_i;
        if (specular_i > 0) specular_intensity += attenuation*specular_i;
    }

    //diffuse_intensity *= 1.0 / N_LIGHTS;
    //specular_intensity *= 1.0 / N_LIGHTS;


    //float intensity = dot(normalize(normal), normalize(light_pos_2));
    //color = vec4(0.5 * normal + 0.5, 1.0);

    color = vec4(c_emissive
        + c_diffuse*diffuse_intensity
        + c_specular*specular_intensity, 1.0f);
}
