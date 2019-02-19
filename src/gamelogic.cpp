#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "gamelogic.h"
#include "sceneGraph.hpp"


enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

double padPositionX = 0;
double padPositionY = 0;

unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

SceneNode* rootNode;
SceneNode* boxNode;
SceneNode* ballNode;
SceneNode* padNode;

SceneNode* lightNode[3];

double ballRadius = 3.0f;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
sf::Sound* sound;

const glm::vec3 boxDimensions(180, 90, 50);
const glm::vec3 padDimensions(30, 3, 40);

glm::vec3 ballPosition(0, ballRadius + padDimensions.y, boxDimensions.z / 2);
glm::vec3 ballDirection(1, 1, 0.02f);

const float BallVerticalTravelDistance = boxDimensions.y - 2.0 * ballRadius - padDimensions.y;

CommandLineOptions options;

bool hasStarted = false;
bool hasLost = false;
bool jumpedToNextFrame = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 45;
double totalElapsedTime = debug_startTime;



void mouseCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    padPositionX = x / double(windowWidth);
    padPositionY = y / double(windowHeight);

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
}

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
        return;
    }

    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    shader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    shader->activate();

    Mesh box = generateBox(boxDimensions.x, boxDimensions.y, boxDimensions.z, true);
    Mesh pad = generateBox(padDimensions.x, padDimensions.y, padDimensions.z, false);
    Mesh sphere = generateSphere(1.0, 40, 40);

    unsigned int ballVAO = generateBuffer(sphere);
    unsigned int boxVAO = generateBuffer(box);
    unsigned int padVAO = generateBuffer(pad);

    rootNode = createSceneNode();
    boxNode = createSceneNode() ;
    padNode = createSceneNode();
    ballNode = createSceneNode();

    rootNode->children.push_back(boxNode);
    rootNode->children.push_back(padNode);
    rootNode->children.push_back(ballNode);

    boxNode->vertexArrayObjectID = boxVAO;
    boxNode->VAOIndexCount = box.indices.size();

    padNode->vertexArrayObjectID = padVAO;
    padNode->VAOIndexCount = pad.indices.size();

    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount = sphere.indices.size();

    // task 1a, add point lights
    for (int i = 0; i<3; i++) {
        lightNode[i] = createSceneNode();
        lightNode[i]->nodeType = SceneNodeType::POINT_LIGHT;
        lightNode[i]->lightID = i;
    }
    rootNode->children.push_back(lightNode[0]);
    rootNode->children.push_back(lightNode[1]);
    ballNode->children.push_back(lightNode[2]);
    lightNode[0]->position = {boxDimensions.x/2 - 10, boxDimensions.y/2 - 10, boxDimensions.z/2 - 10};
    lightNode[1]->position = {300,300,400};
    lightNode[2]->position = {0, 0, 0};

    lightNode[1]->nodeType = SPOT_LIGHT;
    padNode->targeted_by = lightNode[1];

    getTimeDeltaSeconds();

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar, glm::mat4 V, glm::mat4 P) {

    glm::mat4 transformationMatrix(1.0);

    switch(node->nodeType) {
        case GEOMETRY:
            transformationMatrix =
                    glm::translate(glm::mat4(1.0), node->position)
                    * glm::translate(glm::mat4(1.0), node->referencePoint)
                    * glm::rotate(glm::mat4(1.0), node->rotation.z, glm::vec3(0,0,1))
                    * glm::rotate(glm::mat4(1.0), node->rotation.y, glm::vec3(0,1,0))
                    * glm::rotate(glm::mat4(1.0), node->rotation.x, glm::vec3(1,0,0))
                    * glm::translate(glm::mat4(1.0), -node->referencePoint)
                    * glm::scale(glm::mat4(1.0), node->scale);
            break;
        case POINT_LIGHT:
        case SPOT_LIGHT:
            transformationMatrix =
                    glm::translate(glm::mat4(1.0), node->position);
            break;
    }
    glm::mat4 M = transformationThusFar * transformationMatrix;
    glm::mat4 MV = V*M;

    node->currentTransformationMatrixMV = MV;
    node->currentTransformationMatrix = P*MV;
    node->currentTransformationMatrixMVnormal = glm::inverse(glm::transpose(MV));

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, M, V, P);
    }

    if (node->targeted_by != nullptr) {
        assert(node->targeted_by->nodeType == SPOT_LIGHT);
        node->targeted_by->rotation = glm::vec3(MV*glm::vec4(node->position, 1.0));

        //std::cout << node->targeted_by->rotation[0]
        //    << " " << node->targeted_by->rotation[1]
        //    << " " << node->targeted_by->rotation[2]
        //    << std::endl;
    }
}

void updateFrame(GLFWwindow* window) {
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

        ballPosition.x = (1 - padPositionX) * (boxDimensions.x - padDimensions.x) + padDimensions.x / 2.0;
        ballPosition.y = ballRadius + padDimensions.y;
    } else {

        // I really should calculate this using the std::chrono timestamp for this
        // You definitely end up with a cumulative error when doing lots of small additions like this
        // However, for a game that lasts only a few minutes this is fine.
        totalElapsedTime += timeDelta;

        if(hasLost) {
            ballRadius += 200 * timeDelta;
            if(ballRadius > 999999) {
                ballRadius = 999999;
            }
        } else {
            for (unsigned int i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++) {
                if (totalElapsedTime < keyFrameTimeStamps.at(i)) {
                    continue;
                }
                currentKeyFrame = i;
            }

            jumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

            double frameStart = keyFrameTimeStamps.at(currentKeyFrame);
            double frameEnd = keyFrameTimeStamps.at(currentKeyFrame + 1); // Assumes last keyframe at infinity

            double elapsedTimeInFrame = totalElapsedTime - frameStart;
            double frameDuration = frameEnd - frameStart;
            double fractionFrameComplete = elapsedTimeInFrame / frameDuration;

            double ballYCoord;

            const float ballBottomY = ballRadius + padDimensions.y;

            KeyFrameAction currentOrigin = keyFrameDirections.at(currentKeyFrame);
            KeyFrameAction currentDestination = keyFrameDirections.at(currentKeyFrame + 1);

            if (currentOrigin == BOTTOM && currentDestination == BOTTOM) {
                ballYCoord = ballBottomY;
            } else if (currentOrigin == TOP && currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance;
            } else if (currentDestination == BOTTOM) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * (1 - fractionFrameComplete);
            } else if (currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * fractionFrameComplete;
            }


            const float ballSpeed = 60.0f;

            ballPosition.x += timeDelta * ballSpeed * ballDirection.x;
            ballPosition.y = ballYCoord;
            ballPosition.z += timeDelta * ballSpeed * ballDirection.z;

            if (ballPosition.x + ballRadius > boxDimensions.x) {
                // Crude approximation, because it does not compute the intersection with the wall
                // Not doing it causes the ball to get stuck in the wall though
                ballPosition.x = boxDimensions.x - ballRadius;
                ballDirection.x *= -1;
            } else if (ballPosition.x - ballRadius < 0) {
                ballPosition.x = ballRadius;
                ballDirection.x *= -1;
            }

            if (ballPosition.y + ballRadius > boxDimensions.y) {
                ballPosition.y = boxDimensions.y - ballRadius;
                ballDirection.y *= -1;
            } else if (ballPosition.y - ballRadius < 0) {
                ballPosition.y = ballRadius;
                ballDirection.y *= -1;
            }

            if (ballPosition.z + ballRadius > boxDimensions.z) {
                ballPosition.z = boxDimensions.z - ballRadius;
                ballDirection.z *= -1;
            } else if (ballPosition.z - ballRadius < 0) {
                ballPosition.z = ballRadius;
                ballDirection.z *= -1;
            }

            if(options.enableAutoplay) {
                padPositionX = 1 - (ballPosition.x / (boxDimensions.x - 2 * ballRadius));
                padPositionY = 1 - (ballPosition.z / (boxDimensions.z - 2 * ballRadius));
            }

            // Check if the ball is hitting the pad when the ball is at the bottom.
            // If not, you just lost the game! (hehe)
            if (jumpedToNextFrame && currentOrigin == BOTTOM && currentDestination == TOP) {
                double padLeftXCoordinate = (1 - padPositionX) * (boxDimensions.x - padDimensions.x);
                double padRightXCoordinate = padLeftXCoordinate + padDimensions.x;

                double padFrontZCoordinate = (1 - padPositionY) * (boxDimensions.z - padDimensions.z);
                double padBackZCoordinate = padFrontZCoordinate + padDimensions.z;

                if (ballPosition.x < padLeftXCoordinate
                    || ballPosition.x > padRightXCoordinate
                    || ballPosition.z < padFrontZCoordinate
                    || ballPosition.z > padBackZCoordinate) {
                    hasLost = true;
                    if (options.enableMusic) {
                        sound->stop();
                    }
                }
            }
        }
    }

    glm::mat4 projection = glm::perspective(glm::radians(90.0f), float(windowWidth) / float(windowHeight), 0.1f,
                                            120.f);

    glm::mat4 cameraTransform =   glm::translate(glm::mat4(1), glm::vec3(0, 0, 0))
                                * glm::rotate(glm::mat4(1.0), 0.2f, glm::vec3(1, 0, 0))
                                * glm::rotate(glm::mat4(1.0), float(M_PI), glm::vec3(0, 1, 0));

    updateNodeTransformations(rootNode, glm::mat4(1.0), cameraTransform, projection);

    boxNode->position = {-boxDimensions.x / 2, -boxDimensions.y / 2 - 15, boxDimensions.z - 10};
    padNode->position = {-boxDimensions.x / 2 + (1 - padPositionX) * (boxDimensions.x - padDimensions.x),
                         -boxDimensions.y / 2 - 15,
                         boxDimensions.z - 10 + (1 - padPositionY) * (boxDimensions.z - padDimensions.z)};
    ballNode->position = {-boxDimensions.x / 2 + ballPosition.x,
                          -boxDimensions.y / 2 - 15 + ballPosition.y,
                          boxDimensions.z - 10 + ballPosition.z};

    ballNode->scale = {ballRadius, ballRadius, ballRadius};
}


void renderNode(SceneNode* node) {
    glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));
    glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrixMV));
    glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrixMVnormal));

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case SPOT_LIGHT:
        case POINT_LIGHT: {
            std::string pre = "light[" + std::to_string(node->lightID) + "]";

            glUniform3fv(shader->location(pre+".position"), 1, glm::value_ptr(node->position));
            glUniformMatrix4fv(shader->location(pre+".MV"), 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrixMV));

            glUniform1i(shader->location(pre+".is_spot"), node->nodeType == SPOT_LIGHT);
            glUniform3fv(shader->location(pre+".spot_target"), 1, glm::value_ptr(node->rotation));

            break;
        }
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    renderNode(rootNode);
}
