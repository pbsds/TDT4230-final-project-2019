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
#include <utilities/timeutils.hpp>
#include <utilities/glfont.h>
#include <utilities/glmhelpers.hpp>

using std::cout;
using std::endl;
typedef unsigned int uint;

//vec3 cameraPosition = vec3(500, -100, 150);
//vec3 cameraPosition = vec3(500, -140, 200);
vec3 cameraPosition = vec3(420, -120, 190);
//vec3 cameraLookAt = vec3(500, 220, 0);
vec3 cameraLookAt = vec3(460, 220, 0);
vec3 cameraUpward = vec3(0, 0, 1);

const size_t N_GRASS = 150;
const size_t N_TREES = 30;
const size_t DISPLACEMENT = 40;
const vec2 plane_movement = {0.5, 0.1};

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

vector<SceneNode*> movingNodes;

Gloom::Shader* default_shader;
//Gloom::Shader* plain_shader;

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
    }
    
    SceneNode* treeModel = loadModelScene("../res/models/fur_tree", "scene.gltf");
    treeModel->setMaterial(Material().emissive(vec3(0.2)).emissive_only().no_texture_reset(), true);
    treeModel->scale *= 0.8;
    treeModel->scale.z *= 0.8;
    for (uint i = N_TREES; i--;) {
        SceneNode* tree = treeModel->clone();
        tree->position.x = (rand() % 10000) / 10;
        tree->position.y = (rand() % 10000) / 10;
        tree->position.z = DISPLACEMENT * (t_perlin.at_bilinear(tree->position.x*3/1000, tree->position.y*3/1000).x * 2 - 1) - 0.5;
        //node->position.z = DISPLACEMENT * (t_perlin.at_nearest(node->position.x*3/1000, node->position.y*3/1000).x * 2 - 1) - 0.5;
        tree->rotation.z = (rand() % 31415) / 10000;
        tree->scale.z *= 0.8 + (rand()%100)/250;
        rootNode->children.push_back(tree);
        movingNodes.push_back(tree);
    }
    
    SceneNode* grassModel = loadModelScene("../res/models/single_grass", "scene.gltf");
    grassModel->setMaterial(Material().emissive(vec3(0.2)).emissive_only().no_texture_reset(), true);
    grassModel->scale *= 1.3;
    grassModel->scale.z *= 0.4;
    for (uint i = N_GRASS; i--;) {
        SceneNode* grass = grassModel->clone();
        grass->position.x = (rand() % 10000) / 10;
        grass->position.y = (rand() % 10000) / 10;
        grass->position.z = DISPLACEMENT * (t_perlin.at_bilinear(grass->position.x*3/1000, grass->position.y*3/1000).x * 2 - 1) - 0.5;
        grass->rotation.z = (rand() % 31415) / 10000;
        grass->scale.z *= 0.8 + (rand()%100)/250;
        rootNode->children.push_back(grass);
        movingNodes.push_back(grass);
    }
    //treeNode
    
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
    carNode->setMaterial(Material().backlight(vec3(0.3), 0.3).backlight_only().no_texture_reset(), true);
    carNode->position = {522, 130, 0};
    carNode->referencePoint = {0, -1, 0};
    carNode->scale *= 28;
    carNode->rotation.z = -glm::acos(1/glm::sqrt(5*5 + 1*1));
    rootNode->children.push_back(carNode);
    
    //create the scene:
    plainNode = createSceneNode();
    plainNode->setMaterial(Material().specular(vec3(0.15), 3));
    plainNode->setTexture(&t_plain_diff, &t_plain_normal, &t_perlin);
    plainNode->setMesh(&m_plain);
    plainNode->position = {0, 0, 0};
    plainNode->displacementCoefficient = DISPLACEMENT;
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
    
    /*
    sphereNode = createSceneNode();
    //sphereNode->setTexture(&t_cobble_diff, &t_cobble_normal);
    sphereNode->setMesh(&m_sphere);
    sphereNode->position = {470, 130, 100};
    sphereNode->scale *= 0.2;
    sphereNode->scale.z *= 150*5;
    sphereNode->setMaterial(Material().reflection_mapped(&t_reflection, 0.5).no_colors().no_texture_reset(), true);
    rootNode->children.push_back(sphereNode);
    */
    
    //sphereNode = createSceneNode();
    //sphereNode->setMesh(&m_sphere);
    //sphereNode->position = {0.9, -4.3, 1.1};
    //sphereNode->scale *= 0.015;
    //sphereNode->emissive_color = vec3(1.0);
    //carNode->children.push_back(sphereNode);
    
    
    
    glClearColor(0.05, 0.1, 0.15, 1.0);

    lightNode[0]->position = {-600, 1400, 800};
    lightNode[0]->position = {-600, 0, 800};
    lightNode[0]->attenuation = vec3(1.8, 0.0, 0.0); // the color of the first light affects the emissive component aswell
    //lightNode[0]->light_color = vec3(0.3, 0.3, 0.9);
    lightNode[0]->light_color = vec3(0.5, 0.5, 1.0);
    rootNode->children.push_back(lightNode[0]);
    
    // car spotlights
    for (uint i =1; i < 3; i++) {
        lightNode[i]->nodeType = SPOT_LIGHT;
        lightNode[i]->position = {0.9, -4.3, 1.1};
        lightNode[i]->light_color = vec3(0.7, 0.7, 0.5);
        lightNode[i]->spot_direction = glm::normalize(vec3(-1, -0.15, 0));
        lightNode[i]->spot_cuttof_cos = glm::cos(glm::radians(20.0));
        lightNode[i]->transform_spot = true;
        lightNode[i]->attenuation = vec3(1.0, 0.0, 0.000005);
        carNode->children.push_back(lightNode[i]);
    }
    lightNode[2]->position.x *= -1;
    
    // car backlights
    for (uint i =3; i < 5; i++) {
        lightNode[i]->nodeType = POINT_LIGHT;
        lightNode[i]->position = {0.9, 1.3, 1.1};
        lightNode[i]->light_color = vec3(1.0, 0, 0);
        lightNode[i]->attenuation = vec3(1.0, 0, 0.003); // vec3(2.0, -0.175, 0.009);
        carNode->children.push_back(lightNode[i]);
    }
    lightNode[4]->position.x *= -1;
    
    // car frontlights point component
    for (uint i =5; i < 7; i++) {
        lightNode[i]->nodeType = POINT_LIGHT;
        lightNode[i]->position = {0.9, -4.7, 1.1};
        lightNode[i]->light_color = vec3(0.7, 0.7, 0.5);
        lightNode[i]->attenuation = vec3(1.0, -0.01, 0.005); // vec3(2.0, -0.175, 0.009);
        carNode->children.push_back(lightNode[i]);
    }
    lightNode[6]->position.x *= -1;

    
    // HUD
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
    
    
    if (boxNode) boxNode->rotation.z += timeDelta;

    //carNode->rotation.x = glm::sin(timeAcc*2);
    
    {
        vec3 o = carNode->position;
        o.x += plainNode->uvOffset.x*1000/3;
        o.y += plainNode->uvOffset.y*1000/3;
        float t/*heta*/ = carNode->rotation.z + 3*3.1415926535/2.0;
        
        vec3 fr =  o + vec3(60*glm::cos(t), 60*glm::sin(t), 0) + vec3(30*glm::sin(t), -30*glm::cos(t), 0);
        vec3 fl =  o + vec3(60*glm::cos(t), 60*glm::sin(t), 0) - vec3(30*glm::sin(t), -30*glm::cos(t), 0);
        vec3 bl =  o - vec3(40*glm::cos(t), 40*glm::sin(t), 0) - vec3(30*glm::sin(t), -30*glm::cos(t), 0);
        vec3 br =  o - vec3(40*glm::cos(t), 40*glm::sin(t), 0) + vec3(30*glm::sin(t), -30*glm::cos(t), 0);
        //sphereNode->position = fr; // to check where it is
        
        float frh = DISPLACEMENT * (t_perlin.at_bilinear(fr.x*3/1000, fr.y*3/1000).x * 2 - 1);
        float flh = DISPLACEMENT * (t_perlin.at_bilinear(fl.x*3/1000, fl.y*3/1000).x * 2 - 1);
        float brh = DISPLACEMENT * (t_perlin.at_bilinear(br.x*3/1000, br.y*3/1000).x * 2 - 1);
        float blh = DISPLACEMENT * (t_perlin.at_bilinear(bl.x*3/1000, bl.y*3/1000).x * 2 - 1);

        //cout << o.x << " " << o.y << endl;
        //cout << frh << "\t" << flh << "\t" << blh << "\t" << brh << endl;
        //cout << ((frh+flh)-(brh+blh))/2 / 100 << endl;
        
        carNode->rotation.x = -glm::asin(((frh+flh)-(brh+blh)) / 2 / 100);
        carNode->rotation.y =  glm::asin(((frh+brh)-(flh+blh)) / 2 / 60);
        carNode->position.z = (frh+flh+blh+brh)/4.0;
    }
    
    plainNode->uvOffset -= timeDelta * plane_movement;
    
    for (SceneNode* node : movingNodes) {
        node->position += vec3(plane_movement * (timeDelta*1000/3), 0.0);
        if (node->position.x > 1000.0) node->position.x -= 1000.0;
        if (node->position.y > 1000.0) node->position.y -= 1000.0;
        //node->position.z = DISPLACEMENT * (t_perlin.at_bilinear(node->position.x*3/1000, node->position.y*3/1000).x * 2 - 1) - 0.5;
        
        // cull objects in the cars path
        vec2 PQ = vec2(node->position - carNode->position);
        vec2 N = flip(plane_movement)*vec2(1, -1);
        float dist_from_path = glm::length(glm::dot(PQ, N)) / glm::length(N);
        node->isHidden = dist_from_path < 60;
    }

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
