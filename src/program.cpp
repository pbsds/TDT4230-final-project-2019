// Local headers
#include "program.hpp"
#include "utilities/window.hpp"
#include "renderlogic.hpp"
#include <glm/glm.hpp>
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <iomanip>
#include <SFML/Audio.hpp>
#include <SFML/System/Time.hpp>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <utilities/shader.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utilities/timeutils.hpp>

using std::cout;
using std::endl;
using std::setprecision;

void runProgram(GLFWwindow* window, CommandLineOptions options)
{
    // Enable depth (Z) buffer (accept "closest" fragment)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Configure miscellaneous OpenGL settings
    glEnable(GL_CULL_FACE);

    //enable alpha
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    int w, h;
    glfwGetWindowSize(window, &w, &h);
    
    initRenderer(window, w, h);
    init_scene(options);
    Clock c, prof;
    
    // Rendering Loop
    while (!glfwWindowShouldClose(window))
    {
        glfwGetWindowSize(window, &w, &h);
        
        double td = c.getTimeDeltaSeconds();
        step_scene(td);
        
        prof.getTimeDeltaSeconds();
        updateFrame(window, w, h);
        cout << "uf: " << setprecision(4) << prof.getTimeDeltaSeconds() / td * 100 << "\t";
        renderFrame(window, w, h);
        cout << "rf: " << setprecision(4) << prof.getTimeDeltaSeconds() / td * 100 << endl << endl;


        // Handle other events
        glfwPollEvents();
        handleKeyboardInput(window);

        // Flip buffers
        glfwSwapBuffers(window);
    }
}


void handleKeyboardInput(GLFWwindow* window)
{
    // Use escape key for terminating the GLFW window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
        || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}
