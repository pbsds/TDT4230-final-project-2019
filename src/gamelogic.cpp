#include "gamelogic.h"
#include "sceneGraph.hpp"
#include <GLFW/glfw3.h>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <chrono>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <string>
#include <utilities/glfont.h>
#include <utilities/glutils.h>
#include <utilities/imageLoader.hpp>
#include <utilities/modelLoader.hpp>
#include <utilities/mesh.h>
#include <utilities/shader.hpp>
#include <utilities/shapes.h>
#include <utilities/timeutils.h>

using glm::vec3;
using glm::vec4;
using glm::mat4;
typedef unsigned int uint;

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

uint currentKeyFrame = 0;
uint previousKeyFrame = 0;

SceneNode* rootNode;
SceneNode* plainNode;
SceneNode* carNode;
SceneNode* boxNode;
SceneNode* sphereNode;
SceneNode* hudNode;
SceneNode* textNode;

const uint N_LIGHTS = 1;
SceneNode* lightNode[N_LIGHTS];

// These are heap allocated, because they should not be initialised at the start of the program
sf::Sound* sound;
sf::SoundBuffer* buffer;
Gloom::Shader* default_shader;
Gloom::Shader* test_shader;
Gloom::Shader* plain_shader;
Gloom::Shader* post_shader;

vec3 cameraPosition = vec3(0, 0, 400);
vec3 cameraLookAt = vec3(500, 500, 0);
vec3 cameraUpward = vec3(0, 0, 1);

CommandLineOptions options;

bool hasStarted = false;
bool hasLost = false;
bool jumpedToNextFrame = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 45;
double totalElapsedTime = debug_startTime;

// textures
PNGImage t_charmap       = loadPNGFile("../res/textures/charmap.png");
PNGImage t_cobble_diff   = loadPNGFile("../res/textures/cobble_diff.png");
PNGImage t_cobble_normal = loadPNGFile("../res/textures/cobble_normal.png");
PNGImage t_plain_diff    = loadPNGFile("../res/textures/plain_diff.png");
PNGImage t_plain_normal  = loadPNGFile("../res/textures/plain_normal.png", true);
PNGImage t_perlin        = makePerlinNoisePNG(256, 256, 0.05/16);


void mouseCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    float mousePositionX = x / double(windowHeight); // like the hudNode space
    float mousePositionY = y / double(windowHeight);
    /*
    if(padPositionX > 1) {
        padPositionX = 1;
        glfwSetCursorPos(window, windowWidth, y);
    } else if(padPositionX < 0) {
        padPositionX = 0;
        glfwSetCursorPos(window, 0, y);
    }
    if(padPositionY > 1) {
        padPositionY = 1;
        glfwSetCursorPos(window, x, windowHeight);
    } else if(padPositionY < 0) {
        padPositionY = 0;
        glfwSetCursorPos(window, x, 0);
    }
    */
}

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
        return;
    }

    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    // load shaders
    default_shader = new Gloom::Shader();
    default_shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    
    Mesh box = generateBox(50, 50, 50);
    Mesh sphere = generateSphere(10, 100, 100);
    Mesh plain = generateSegmentedPlane(1000, 1000, 100, 100, 3);
    Mesh hello_world = generateTextGeometryBuffer("Skjer'a bagera?", 1.3, 2);
    t_perlin.repeat_mirrored = true;

    rootNode = createSceneNode();
    hudNode = createSceneNode();
    
    // create and add lights to graph
    for (uint i = 0; i<N_LIGHTS; i++) {
        lightNode[i] = createSceneNode(POINT_LIGHT);
        lightNode[i]->lightID = i;
        rootNode->children.push_back(lightNode[i]);
    }
    
    
    carNode = loadModelScene("../res/models/beetle/scene.gltf", {
        { 0, Material().diffuse({0.0, 0.0, 1.0}).diffuse_only().reflection_mapped(&t_reflection, 0.15)},// Blue_Metal
        { 1, Material().diffuse(vec3(0.85)).emissive(vec3(0.1)).reflection_mapped(&t_reflection, -1.0)},// Metal (decals)
        //{ 2, Material().diffuse({1.0, 1.0, 1.0})},// Front_Light_Glass
    //    { 3, Material().diffuse({0.2, 0.2, 0.2})},// Black_Rubber
        { 4, Material().no_colors().reflection_mapped(&t_reflection, 1.0)},// Mirror
        //{ 5, Material().diffuse({1.0, 1.0, 1.0})},// Black_Metal
        //{ 6, Material().diffuse({1.0, 1.0, 1.0})},// Plastic
//        { 7, Material().diffuse(vec3(0.2)).emissive(vec3(0.25)).specular(vec3(1.0), 70).reflection_mapped(&t_reflection, -0.8)},// Window_Glass
        { 7, Material().diffuse(vec3(0.2)).emissive(vec3(0.25)).specular(vec3(1.0), 70).reflection_mapped(&t_reflection, -0.8)},// Window_Glass
        //{ 8, Material().diffuse({1.0, 1.0, 1.0})},// Material
        { 9, Material().diffuse(vec3(1.0)).emissive(vec3(0.2)).specular(vec3(0.4), 70).reflection_mapped(&t_reflection, -1.0)},// Glossy_metal
        //{10, Material().diffuse({1.0, 1.0, 1.0})},// Rogh_Metal
//        {11, Material().no_colors().reflection_mapped(&t_reflection, 1.0)},// License_Plate_Metal
        {11, Material().no_colors().reflection_mapped(&t_reflection, 1.0)},// License_Plate_Metal
        //{12, Material().diffuse({1.0, 1.0, 1.0})},// License_Plate_Frame
        //{13, Material().diffuse({1.0, 1.0, 1.0})},// 
        });
    //carNode->setMaterial(Material().reflection_mapped(&t_reflection, 0.0).no_colors().no_texture_reset(), true);
    carNode->position = {500, 500, 100};
    carNode->scale *= 100;
    rootNode->children.push_back(carNode);
    
    //create the scene:
    plainNode = createSceneNode();
    plainNode->setTexture(&t_plain_diff, &t_plain_normal, &t_perlin);
    plainNode->setMesh(&plain);
    plainNode->position = {0, 0, 0};
    plainNode->shininess = 20;
    plainNode->displacementCoefficient = 40;
    rootNode->children.push_back(plainNode);
    
    /*
    boxNode = createSceneNode();
    boxNode->setTexture(&t_cobble_diff, &t_cobble_normal);
    boxNode->setMesh(&box);
    boxNode->position = {500, 500, 40};
    boxNode->referencePoint = {25, 25, 25};
    boxNode->scale *= 2;
    boxNode->shininess = 20;
    boxNode->displacementCoefficient = 40;
    rootNode->children.push_back(boxNode);
    */
    
    sphereNode = createSceneNode();
    //sphereNode->setTexture(&t_cobble_diff, &t_cobble_normal);
    sphereNode->setMesh(&sphere);
    sphereNode->position = {500, 500, 100};
    sphereNode->scale *= 15;
    sphereNode->diffuse_color;
    sphereNode->setMaterial(Material().reflection_mapped(&t_reflection, 0.5).no_colors().no_texture_reset(), true);
    //rootNode->children.push_back(sphereNode);
    
    lightNode[0]->position = {-600, 1400, 800};
    lightNode[0]->attenuation = vec3(1.8, 0.0, 0.0);
    
    
    textNode = createSceneNode();
    textNode->setTexture(&t_charmap);
    textNode->setMesh(&hello_world);
    textNode->position = vec3(-1.0, -1.0, 0.0);
    textNode->isIlluminated = false;
    textNode->isInverted = true;
    hudNode->children.push_back(textNode);
    
    
    getTimeDeltaSeconds();

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateNodeTransformations(SceneNode* node, mat4 transformationThusFar, mat4 const& V, mat4 const& P) {
    mat4 transformationMatrix
        = glm::translate(mat4(1.0), node->position)
        * glm::translate(mat4(1.0), node->referencePoint)
        * glm::rotate(mat4(1.0), node->rotation.z, vec3(0,0,1))
        * glm::rotate(mat4(1.0), node->rotation.y, vec3(0,1,0))
        * glm::rotate(mat4(1.0), node->rotation.x, vec3(1,0,0))
        * glm::scale(mat4(1.0), node->scale)
        * glm::translate(mat4(1.0), -node->referencePoint);

    mat4 M = transformationThusFar * transformationMatrix;

    node->MV = V*M;
    node->MVP = P*node->MV;
    node->MVnormal = glm::inverse(glm::transpose(node->MV));

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, M, V, P);
    }

    // move this into the renderNode method and have it be targeted_node from the spot
    if (node->targeted_by != nullptr) {
        assert(node->targeted_by->nodeType == SPOT_LIGHT);
        node->targeted_by->rotation = vec3(node->MV*vec4(node->position, 1.0));
    }
}

void updateFrame(GLFWwindow* window, int windowWidth, int windowHeight) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    double timeDelta = getTimeDeltaSeconds();

    if(!hasStarted) {

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
            if (options.enableMusic) {
                sound = new sf::Sound();
                sound->setBuffer(*buffer);
                sf::Time startTime = sf::seconds(debug_startTime);
                sound->setPlayingOffset(startTime);
                sound->play();
            }
            totalElapsedTime = debug_startTime;
            hasStarted = true;
        }
    } else {

        // I really should calculate this using the std::chrono timestamp for this
        // You definitely end up with a cumulative error when doing lots of small additions like this
        // However, for a game that lasts only a few minutes this is fine.
        totalElapsedTime += timeDelta;

        if(hasLost) {
            //ballRadius += 200 * timeDelta;
            //if(ballRadius > 999999) {
            //    ballRadius = 999999;
            //}
        } else {
            for (uint i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++) {
                if (totalElapsedTime < keyFrameTimeStamps.at(i)) {
                    continue;
                }
                currentKeyFrame = i;
            }

            jumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

        }
    }

    mat4 projection = glm::perspective(
        glm::radians(45.0f), // fovy
        float(windowWidth) / float(windowHeight), // aspect
        0.1f, 5000.f // near, far
    );

    mat4 cameraTransform
        = glm::lookAt(cameraPosition, cameraLookAt, cameraUpward);

    updateNodeTransformations(rootNode, mat4(1.0), cameraTransform, projection);

    // We orthographic now, bitches!
    // set orthographic VP
    cameraTransform = mat4(1.0);
    projection = glm::ortho(-float(windowWidth) / float(windowHeight), float(windowWidth) / float(windowHeight), -1.0f, 1.0f);
    updateNodeTransformations(hudNode, mat4(1.0), cameraTransform, projection);

    // update positions of nodes (like the car)
    plainNode->uvOffset.x += timeDelta*0.5;
    plainNode->uvOffset.y -= timeDelta*0.5;
    if (boxNode) boxNode->rotation.z += timeDelta;
    lightNode[1]->rotation.z -= timeDelta;
    lightNode[1]->position.z = 80 + 40*glm::sin(5 * lightNode[1]->rotation.z);
    //if(carNode) carNode->rotation.z += timeDelta;
}


void renderNode(SceneNode* node, Gloom::Shader* parent_shader = default_shader) {
    struct Light { // lights as stored in the shader
        // coordinates in MV space
        vec3  position; // MV
        vec3  attenuation;
        vec3  color;
        
        bool  is_spot;
        vec3  spot_direction; // MV, must be normalized
        float spot_cuttof_cos;
        
        void push_to_shader(Gloom::Shader* shader, uint id) {
            #define L(x) shader->location("light[" + std::to_string(id) + "]." #x)
            #define V(x) glUniform3fv(L(x), 1, glm::value_ptr(x))
                glUniform1i (L(is_spot)          , is_spot);
                glUniform1f (L(spot_cuttof_cos), spot_cuttof_cos);
                V(position);
                V(spot_direction);
                V(attenuation);
                V(color);
            #undef V
            #undef L
        }
    };
    static Light lights[N_LIGHTS];
    static Gloom::Shader* s = nullptr; // The currently active shader
    
    // activate the correct shader
    Gloom::Shader* node_shader = (node->shader != nullptr)
        ? node->shader
        : parent_shader;
    if (s != node_shader) {
        s = node_shader;
        s->activate();
        uint i = 0; for (Light l : lights) l.push_to_shader(s, i++);
    }

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                // load uniforms
                glUniformMatrix4fv(s->location("MVP")     , 1, GL_FALSE, glm::value_ptr(node->MVP));
                glUniformMatrix4fv(s->location("MV")      , 1, GL_FALSE, glm::value_ptr(node->MV));
                glUniformMatrix4fv(s->location("MVnormal"), 1, GL_FALSE, glm::value_ptr(node->MVnormal));
                glUniform2fv(s->location("uvOffset")      , 1,           glm::value_ptr(node->uvOffset));
                glUniform3fv(s->location("diffuse_color") , 1,           glm::value_ptr(node->diffuse_color));
                glUniform3fv(s->location("emissive_color"), 1,           glm::value_ptr(node->emissive_color));
                glUniform3fv(s->location("specular_color"), 1,           glm::value_ptr(node->specular_color));
                glUniform1f( s->location("opacity"),                 node->opacity);
                glUniform1f( s->location("shininess"),               node->shininess);
                glUniform1f( s->location("reflexiveness"),           node->reflexiveness);
                glUniform1f( s->location("displacementCoefficient"), node->displacementCoefficient);
                glUniform1ui(s->location("isTextured"),              node->isTextured);
                glUniform1ui(s->location("isVertexColored"),         node->isVertexColored);
                glUniform1ui(s->location("isNormalMapped"),          node->isNormalMapped);
                glUniform1ui(s->location("isDisplacementMapped"),    node->isDisplacementMapped);
                glUniform1ui(s->location("isReflectionMapped"),      node->isReflectionMapped);
                glUniform1ui(s->location("isIlluminated"),           node->isIlluminated);
                glUniform1ui(s->location("isInverted"),              node->isInverted);
                
                if (node->isTextured)           glBindTextureUnit(0, node->diffuseTextureID);
                if (node->isNormalMapped)       glBindTextureUnit(1, node->normalTextureID);
                if (node->isDisplacementMapped) glBindTextureUnit(2, node->displacementTextureID);
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case SPOT_LIGHT:
        case POINT_LIGHT: {
            uint id = node->lightID;
            lights[id].position          = vec3(node->MV * vec4(vec3(0.0), 1.0));
            lights[id].is_spot           = node->nodeType == SPOT_LIGHT;
            lights[id].spot_direction    = node->spot_direction; // MV space
            lights[id].spot_cuttof_cos   = node->spot_cuttof_cos;
            lights[id].attenuation       = node->attenuation;
            lights[id].color             = node->light_color;
            lights[id].push_to_shader(s, id);
            break;
        }
        default:
            break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child, node_shader);
    }
}

void renderFrame(GLFWwindow* window, int windowWidth, int windowHeight) {
    glViewport(0, 0, windowWidth, windowHeight);

    renderNode(rootNode);
    renderNode(hudNode);
}
