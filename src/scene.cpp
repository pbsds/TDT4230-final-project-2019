#include "scene.hpp"
#include "renderlogic.hpp"
#include "sceneGraph.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <glad/glad.h>
#include <iostream>
#include <cstdlib>
#include <utilities/imageLoader.hpp>
#include <utilities/modelLoader.hpp>
#include <utilities/mesh.h>
#include <utilities/shader.hpp>
#include <utilities/shapes.h>
#include <utilities/timeutils.h>
#include <utilities/glfont.h>

using std::cout;
using std::endl;
typedef unsigned int uint;

vec3 cameraPosition = vec3(0, 0, 400);
vec3 cameraLookAt = vec3(500, 500, 0);
vec3 cameraUpward = vec3(0, 0, 1);

SceneNode* rootNode;
SceneNode* hudNode;
SceneNode* lightNode[N_LIGHTS];

SceneNode* plainNode;
SceneNode* carNode;
SceneNode* treeNode;
SceneNode* grassNode;
SceneNode* boxNode;
SceneNode* sphereNode;
SceneNode* textNode;

Gloom::Shader* default_shader;
Gloom::Shader* plain_shader;
Gloom::Shader* post_shader;

// todo: const the following:

// meshes
Mesh m_box = generateBox(50, 50, 50);
Mesh m_sphere = generateSphere(10, 100, 100);
Mesh m_plain = generateSegmentedPlane(1000, 1000, 100, 100, 3);
Mesh m_hello_world = generateTextGeometryBuffer("Skjer'a bagera?", 1.3, 2);

// textures
PNGImage t_charmap       = loadPNGFile("../res/textures/charmap.png");
PNGImage t_cobble_diff   = loadPNGFile("../res/textures/cobble_diff.png");
PNGImage t_cobble_normal = loadPNGFile("../res/textures/cobble_normal.png");
PNGImage t_plain_diff    = loadPNGFile("../res/textures/plain_diff.png");
PNGImage t_plain_normal  = loadPNGFile("../res/textures/plain_normal.png", true);
PNGImage t_reflection    = loadPNGFile("../res/textures/reflection_field.png");
PNGImage t_reflection2   = loadPNGFile("../res/textures/reflection_blurry.png");
PNGImage t_perlin        = makePerlinNoisePNG(256, 256, 0.05/16);

void init_scene(CommandLineOptions options) {
    default_shader = new Gloom::Shader();
    default_shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    
    rootNode = createSceneNode();
    hudNode = createSceneNode();
    rootNode->shader = default_shader;
    hudNode->shader = default_shader;

    t_perlin.repeat_mirrored = true; // no const for me ;(
    
    // create and add lights to graph
    for (uint i = 0; i<N_LIGHTS; i++) {
        lightNode[i] = createSceneNode(POINT_LIGHT);
        lightNode[i]->lightID = i;
        rootNode->children.push_back(lightNode[i]);
    }
    
    //treeNode = loadModelScene("../res/models/fur_tree", "scene.gltf");
    //treeNode->position = {300, 800, 10};
    //rootNode->children.push_back(treeNode);
    
    //uint i = 30;
    //while (i--) {
    //    SceneNode* asd = treeNode->clone();
    //    asd->position.x = (std::rand() % 10000) / 10;
    //    asd->position.y = (std::rand() % 10000) / 10;
    //    rootNode->children.push_back(asd);
    //}
    
    grassNode = loadModelScene("../res/models/single_grass", "scene.gltf");
    grassNode->position = {400, 400, 15};
    rootNode->children.push_back(grassNode);
    for (uint i = 100; i--;) {
        SceneNode* asd = grassNode->clone();
        asd->position.x = (std::rand() % 10000) / 10;
        asd->position.y = (std::rand() % 10000) / 10;
        rootNode->children.push_back(asd);
    }
    
    
    /*
    carNode = loadModelScene("../res/models/beetle", "scene.gltf", {
        { 0, Material().diffuse({0.0, 0.0, 1.0}).diffuse_only().reflection_mapped(&t_reflection, 0.15)},// Blue_Metal
        { 1, Material().diffuse(vec3(0.85)).emissive(vec3(0.1)).reflection_mapped(&t_reflection, -1.0)},// Metal (decals)
        //{ 2, Material().diffuse({1.0, 1.0, 1.0})},// Front_Light_Glass
    //    { 3, Material().diffuse({0.2, 0.2, 0.2})},// Black_Rubber
        { 4, Material().no_colors().reflection_mapped(&t_reflection, 1.0)},// Mirror
        //{ 5, Material().diffuse({1.0, 1.0, 1.0})},// Black_Metal
        //{ 6, Material().diffuse({1.0, 1.0, 1.0})},// Plastic
        { 7, Material().diffuse(vec3(0.2)).emissive(vec3(0.25)).specular(vec3(1.0), 70).reflection_mapped(&t_reflection, -0.8)},// Window_Glass
        //{ 8, Material().diffuse({1.0, 1.0, 1.0})},// Material
        { 9, Material().diffuse(vec3(1.0)).emissive(vec3(0.2)).specular(vec3(0.4), 70).reflection_mapped(&t_reflection, -1.0)},// Glossy_metal
        //{10, Material().diffuse({1.0, 1.0, 1.0})},// Rogh_Metal
        {11, Material().no_colors().reflection_mapped(&t_reflection, 1.0)},// License_Plate_Metal
        //{12, Material().diffuse({1.0, 1.0, 1.0})},// License_Plate_Frame
        //{13, Material().diffuse({1.0, 1.0, 1.0})},// 
        });
    carNode->position = {500, 500, 100};
    carNode->scale *= 100;
    rootNode->children.push_back(carNode);
    */
    
    //create the scene:
    plainNode = createSceneNode();
    plainNode->setTexture(&t_plain_diff, &t_plain_normal, &t_perlin);
    plainNode->setMesh(&m_plain);
    plainNode->position = {0, 0, 0};
    plainNode->shininess = 20;
    plainNode->displacementCoefficient = 40;
    rootNode->children.push_back(plainNode);
    
    /*
    boxNode = createSceneNode();
    boxNode->setTexture(&t_cobble_diff, &t_cobble_normal);
    boxNode->setMesh(&m_box);
    boxNode->position = {500, 500, 40};
    boxNode->referencePoint = {25, 25, 25};
    boxNode->scale *= 2;
    boxNode->shininess = 20;
    boxNode->displacementCoefficient = 40;
    rootNode->children.push_back(boxNode);
    */
    
    sphereNode = createSceneNode();
    //sphereNode->setTexture(&t_cobble_diff, &t_cobble_normal);
    sphereNode->setMesh(&m_sphere);
    sphereNode->position = {500, 500, 100};
    sphereNode->scale *= 15;
    sphereNode->diffuse_color;
    sphereNode->setMaterial(Material().reflection_mapped(&t_reflection, 0.5).no_colors().no_texture_reset(), true);
    //rootNode->children.push_back(sphereNode);
    
    lightNode[0]->position = {-600, 1400, 800};
    lightNode[0]->attenuation = vec3(1.8, 0.0, 0.0);
    
    /*
    lightNode[1]->position = {500, 0, 80};
    lightNode[1]->referencePoint = {0, 500, 0};
    lightNode[1]->scale *= 0.8;
    lightNode[1]->light_color = vec3(0.0);
    lightNode[1]->attenuation = vec3(1.0, 0.0, 0.000005);
    
    lightNode[2]->position = {400, -200, 300};
    lightNode[2]->nodeType = SPOT_LIGHT;
    lightNode[2]->attenuation = vec3(1, 0, 0);
    lightNode[2]->spot_target = lightNode[1];
    */
    
    textNode = createSceneNode();
    textNode->setTexture(&t_charmap);
    textNode->setMesh(&m_hello_world);
    textNode->position = vec3(-1.0, -1.0, 0.0);
    textNode->isIlluminated = false;
    textNode->isInverted = true;
    hudNode->children.push_back(textNode);
}

// returns true if mouse should be centered and invisible
bool mouse_position_handler(double mx, double my, int scale) {
    //cout << mx << "\t" << my << endl;
    
    return false;
}

void step_scene(double timeDelta) {
    static double timeAcc = 0; // shrug
    timeAcc += timeDelta;
    
    cout << "td: " << timeDelta << " " << 1/timeDelta << endl;
    
    plainNode->uvOffset.x += timeDelta*0.5;
    plainNode->uvOffset.y -= timeDelta*0.5;
    if (boxNode) boxNode->rotation.z += timeDelta;
    lightNode[1]->rotation.z -= timeDelta;
    //lightNode[1]->position.z = 80 + 40*glm::sin(5 * lightNode[1]->rotation.z);
    if(carNode) carNode->rotation.z += timeDelta;
    if(treeNode) treeNode->rotation.z += timeDelta;

    /*
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        if (options.enableMusic) {
            sound = new sf::Sound();
            sound->setBuffer(*buffer);
            sf::Time startTime = sf::seconds(debug_startTime);
            sound->setPlayingOffset(startTime);
            sound->play();
        }
        // I really should calculate this using the std::chrono timestamp for this
        // You definitely end up with a cumulative error when doing lots of small additions like this
        // However, for a game that lasts only a few minutes this is fine.
        totalElapsedTime += timeDelta;
    }
    */
}
