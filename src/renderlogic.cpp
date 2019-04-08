#include "renderlogic.hpp"
#include "sceneGraph.hpp"
#include <GLFW/glfw3.h>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <utilities/glutils.h>
#include <utilities/shader.hpp>

#include <utilities/timeutils.hpp>


using glm::vec3;
using glm::vec4;
using glm::mat4;
using std::cout;
using std::endl;
typedef unsigned int uint;

sf::Sound* sound;
sf::SoundBuffer* buffer;

// for keeping track of the currently loaded shader in renderNode()
Gloom::Shader* current_shader = nullptr;
Gloom::Shader* prev_shader = nullptr; // The last shader to glDrawElements

// the framebuffer we render the scene to before post-processing
GLuint framebufferID = 0;
GLuint framebufferTextureID = 0;
GLuint framebufferDepthBufferID = 0;
GLuint framebufferDepthTextureID = 0;

// the surface we use for post-processing
GLuint postVAO;
Gloom::Shader* post_shader = nullptr;

void mouse_callback(GLFWwindow* window, double x, double y) {
    static bool mouse_mode = false;
    int winw, winh;
    glfwGetWindowSize(window, &winw, &winh);
    glViewport(0, 0, winw, winh);

    double mx = (x - winw/2) / double(winh) * 2; // winh instead of winw, like the hudNode space
    double my = (winh/2 - y) / double(winh) * 2;

    bool reset_mouse = mouse_position_handler(mx, my, winh/2);

    if (reset_mouse)
        glfwSetCursorPos(window, winw/2, winh/2);
    if (reset_mouse != mouse_mode) {
        mouse_mode = reset_mouse;
        glfwSetInputMode(window, GLFW_CURSOR, (reset_mouse)
            ? GLFW_CURSOR_DISABLED
            : GLFW_CURSOR_NORMAL);
    }
}

void initRenderer(GLFWwindow* window, int windowWidth, int windowHeight) {
    static bool first/*time*/ = true;

    if (first&&false) {
        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
            return;
        }
    }

    if(first) glfwSetCursorPosCallback(window, mouse_callback);

    // setup the framebuffer we render the scene to, and the tris we render
    // as the post-processing stage

    if (first) glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

    if (first) glGenTextures(1, &framebufferTextureID);
    glBindTexture(GL_TEXTURE_2D, framebufferTextureID);

    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    
    if (first) glGenTextures(1, &framebufferDepthTextureID);
    glBindTexture(GL_TEXTURE_2D, framebufferDepthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    if (first) glGenRenderbuffers(1, &framebufferDepthBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, framebufferDepthBufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDepthBufferID);
    
    // Set "framebufferTextureID" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebufferTextureID, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, framebufferDepthTextureID, 0);

    // Set the list of draw buffers.
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << (glCheckFramebufferStatus(GL_FRAMEBUFFER)) << endl;
        throw 1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (first) {
        postVAO = generatePostQuadBuffer();
        post_shader = new Gloom::Shader();
        post_shader->makeBasicShader("../res/shaders/post.vert", "../res/shaders/post.frag");
    }

    first = false;
}

// traverses and updates matricies
void updateNodeTransformations(SceneNode* node, mat4 transformationThusFar, mat4 const& V, mat4 const& P) {
    mat4 M = (node->has_no_transforms())
        ? transformationThusFar
        : transformationThusFar
        * glm::translate(mat4(1.0), node->position)
        * glm::translate(mat4(1.0), node->referencePoint)
        * glm::rotate(mat4(1.0), node->rotation.z, vec3(0,0,1))
        * glm::rotate(mat4(1.0), node->rotation.y, vec3(0,1,0))
        * glm::rotate(mat4(1.0), node->rotation.x, vec3(1,0,0))
        * glm::scale(mat4(1.0), node->scale)
        * glm::translate(mat4(1.0), -node->referencePoint);

    node->MV = V*M;
    node->MVP = P*node->MV;
    node->MVnormal = glm::inverse(glm::transpose(node->MV));

    for(SceneNode* child : node->children)
        updateNodeTransformations(child, M, V, P);
}

// step
void updateFrame(GLFWwindow* window, int windowWidth, int windowHeight) {
    float aspect = float(windowWidth) / float(windowHeight);

    // calculate camera
    mat4 projection = glm::perspective(
        glm::radians(45.0f), // fovy
        aspect, // aspect
        0.1f, 5000.f // near, far
    );

    mat4 cameraTransform
        = glm::lookAt(cameraPosition, cameraLookAt, cameraUpward);

    // update scene with camera
    updateNodeTransformations(rootNode, mat4(1.0), cameraTransform, projection);

    // We orthographic now, bitches!
    // set orthographic VP for hud
    cameraTransform = mat4(1.0);
    projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f);

    // update hud
    updateNodeTransformations(hudNode, mat4(1.0), cameraTransform, projection);

    // update spots
    for (SceneNode* node : lightNode) {
        if (node->nodeType == SPOT_LIGHT && node->spot_target) {
            node->spot_direction = glm::normalize(
                vec3(node->spot_target->MV * vec4(0,0,0,1))
                - vec3(node->MV * vec4(0,0,0,1)));
        }
    }

}

// traverses and renders one and one node
struct NodeDistShader{
    SceneNode* node;
    Gloom::Shader* s;
    float dist;
    NodeDistShader(SceneNode* node, Gloom::Shader* s, float dist)
        : node(node), s(s), dist(dist) {}
};
void renderNode(SceneNode* node, Gloom::Shader* parent_shader, vector<NodeDistShader>* transparent_nodes=nullptr, bool do_recursive=true) {
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

    if (node->isHidden) return;

    // activate the correct shader
    Gloom::Shader* s = (node->shader != nullptr)
        ? node->shader
        : parent_shader;
    if (current_shader != s) {
        current_shader = s;
        current_shader->activate();
        uint i = 0; for (Light l : lights) l.push_to_shader(s, i++);
    }

    bool shader_changed = current_shader != prev_shader;
    #define init_cache(x) static decltype(node->x) cached_##x;
    #define if_cache(x) if (shader_changed || cached_##x != node->x) { cached_##x = node->x;
    #define cache(x) init_cache(x) if_cache(x)
    #define um4fv(x) cache(x) glUniformMatrix4fv(s->location(#x), 1, GL_FALSE, glm::value_ptr(node->x)); }
    #define u2fv(x)  cache(x) glUniform2fv( s->location(#x), 1, glm::value_ptr(node->x)); }
    #define u3fv(x)  cache(x) glUniform3fv( s->location(#x), 1, glm::value_ptr(node->x)); }
    #define u1f(x)   cache(x) glUniform1f(  s->location(#x), node->x); }
    #define u1ui(x)  cache(x) glUniform1ui( s->location(#x), node->x); }
    //#define ubtu(n,i,x) init_cache(x) if(node->i) { if_cache(x) glBindTextureUnit(n, node->x); } } else cached_##x = -1;
    #define ubtu(n,i,x) init_cache(x) if(node->i) { if_cache(x) glActiveTexture(GL_TEXTURE0+n); glBindTexture(GL_TEXTURE_2D, node->x); } } else cached_##x = -1;

    switch(node->nodeType) {
        case GEOMETRY:
            if (transparent_nodes!=nullptr && node->has_transparancy()) {
                // defer to sorted pass later on
                transparent_nodes->emplace_back(node, s, glm::length(vec3(node->MVP*vec4(0,0,0,1))));
            }
            else if(node->vertexArrayObjectID != -1) {
                if (node->opacity <= 0.05) break;
                
                // load scene uniforms
                if (shader_changed) { // guaranteed at start of every frame, due to post_shader
                    glUniform3fv(s->location("fog_color"), 1, glm::value_ptr(fog_color));
                    glUniform1f( s->location("fog_strength"), fog_strength);
                }
                
                // load material uniforms
                um4fv(MVP);
                um4fv(MV);
                um4fv(MVnormal);
                u2fv (uvOffset);
                u3fv (diffuse_color);
                u3fv (emissive_color);
                u3fv (specular_color);
                u3fv (backlight_color);
                u1f  (opacity);
                u1f  (shininess);
                u1f  (backlight_strength);
                u1f  (reflexiveness);
                u1f  (displacementCoefficient);
                u1ui (isTextured);
                u1ui (isVertexColored);
                u1ui (isNormalMapped);
                u1ui (isDisplacementMapped);
                u1ui (isReflectionMapped);
                u1ui (isIlluminated);
                u1ui (isInverted);
                ubtu(0, isTextured          , diffuseTextureID);
                ubtu(1, isNormalMapped      , normalTextureID);
                ubtu(2, isDisplacementMapped, displacementTextureID);
                ubtu(3, isReflectionMapped  , reflectionTextureID);
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
                prev_shader = current_shader;
            }
            break;
        case SPOT_LIGHT:
        case POINT_LIGHT: {
            uint id = node->lightID;
            lights[id].position          = vec3(node->MV * vec4(vec3(0.0), 1.0));
            lights[id].is_spot           = node->nodeType == SPOT_LIGHT;
            lights[id].spot_direction    = (node->transform_spot)
                ? node->spot_direction                                   // MV space
                : vec3(node->MVnormal * vec4(node->spot_direction, 1.0));// Model space
            lights[id].spot_cuttof_cos   = node->spot_cuttof_cos;
            lights[id].attenuation       = node->attenuation;
            lights[id].color             = node->light_color;
            lights[id].push_to_shader(s, id);
            break;
        }
        default:
            break;
    }

    #undef um4fv
    #undef u2fv
    #undef u3fv
    #undef u1f
    #undef u1ui
    #undef ubtu
    #undef cache

    if (do_recursive)
    for(SceneNode* child : node->children)
        renderNode(child, s, transparent_nodes, true);
}

// draw
void renderFrame(GLFWwindow* window, int windowWidth, int windowHeight) {
    static int old_windowWidth  = windowWidth;
    static int old_windowHeight = windowHeight;

    if (old_windowWidth != windowWidth || old_windowHeight != windowHeight) {
        old_windowWidth = windowWidth;
        old_windowHeight = windowHeight;
        cout << "reinit renderer" << endl;
        initRenderer(window, old_windowWidth, windowHeight);
    }
    
    // render to internal buffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    glViewport(0, 0, windowWidth, windowHeight);

    // Clear colour and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static vector<NodeDistShader> transparent_nodes;
    transparent_nodes.clear();
    renderNode(rootNode, nullptr, &transparent_nodes); // rootNode defined in scene.hpp

    // sort and render transparent node, sorted by distance from camera
    std::sort(
        transparent_nodes.begin(),
        transparent_nodes.end(),
        [](NodeDistShader a, NodeDistShader b) {
            return a.dist > b.dist;
    });
    glDepthMask(GL_FALSE); // read only
    for (NodeDistShader a : transparent_nodes)
        renderNode(a.node, a.s, nullptr, false);
    renderNode(hudNode, nullptr); // rootNode defined in scene.hpp
    glDepthMask(GL_TRUE); // read write

  
    // render framebuffer to window
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);
    post_shader->activate();
    current_shader = post_shader;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    static Clock c;
    static float t = 0.0;
    t += c.getTimeDeltaSeconds();
    glUniform1f(post_shader->location("time"), t);
    glUniform1ui(post_shader->location("windowWidth"), windowWidth);
    glUniform1ui(post_shader->location("windowHeight"), windowHeight);
    //glBindTextureUnit(0, framebufferTextureID);
    //glBindTextureUnit(1, framebufferDepthTextureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebufferTextureID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, framebufferDepthTextureID);
    glBindVertexArray(postVAO);
    glDrawElements(GL_TRIANGLES, 6 /*vertices*/, GL_UNSIGNED_INT, nullptr);
    prev_shader = post_shader;
    
}
