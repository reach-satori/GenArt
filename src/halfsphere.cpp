#include "halfsphere.h"
#include "consts.h"
#include <stack>

HalfSphere::HalfSphere() :
    // calls the base class constructor: this is the important part: change your shaders here
    Painting(shader_prog("shaders/dome.vert.glsl", "shaders/basic.frag.glsl"))
    {
        /////////////
        //this setup call MUST be inside the derived class constructor:
        //it doesn't work if it's in the base class for whatever reason
        //setup compiles the shaders, creates the program, attaches and links the shaders
        pshader.setup();
        /////////////
        //begin just calls glUseUniform, and end calls glUseUniform(0)
        pshader.begin();

        //this uniform stays constant so it doesn't have to be changed after initial setup
        pshader.uniformMatrix4fv("projectionMatrix", projectionMatrix);
        pshader.end();
    };

void HalfSphere::render(GLuint VAO) {
    std::stack<glm::mat4> ms;
    ms.push(glm::mat4(1.0));

    //set up the shaders, uniforms
    //rendering is as usual, but beginning and ending their own shaders, as well as updating necessary uniforms
    //in this case im passing in a VAO to render because I don't want each painting to have its own VAO,
    //but in principle you can store the objects VAO inside it as well, and it'll probably be more convenient
    pshader.begin();
    pshader.uniformMatrix4fv("viewMatrix", viewMatrix);
    glUniform1f(TIME_LOC, (float)glfwGetTime());

    ms.push(ms.top());
        ms.top() = glm::translate(ms.top(), position);
        ms.top() = glm::rotate(ms.top(), glm::radians(angle), glm::vec3(0., 1., 0.));
        ms.top() = glm::rotate(ms.top(), glm::radians(-90.f), glm::vec3(1., 0., 0.));
        //maybe we can even set up the modelMatrix only once in constructor as well if they don't move around
        pshader.uniformMatrix4fv("modelMatrix", ms.top());
        glDisable(GL_CULL_FACE);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 480 * 6, GL_UNSIGNED_INT, 0);
        glEnable(GL_CULL_FACE);
    ms.pop();
    pshader.end();
};
