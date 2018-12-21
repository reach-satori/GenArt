#include "camera.h"
#include "consts.h"

    Camera::Camera() ://default constructor
        worldpos(glm::vec3(3.f, 6.f, 15.f)),
        rotation(glm::vec2(0.f, 0.f)),
        projection(glm::perspective(glm::radians(80.), (double)WINDOW_WIDTH/WINDOW_HEIGHT, 0.1, 100.)),
        view(glm::mat4(1.f))
        {
            initKeyboard();
        }

    Camera::Camera(glm::mat4 projection) ://with custom projection matrix
        worldpos(glm::vec3(3.f, 6.f, 15.f)),
        rotation(glm::vec2(0.f, 90.f)),
        projection(projection),
        view(glm::mat4(1.f))
        {
            initKeyboard();
        }

    void Camera::initKeyboard() {
        keyboard.insert(std::make_pair(GLFW_KEY_W, false));
        keyboard.insert(std::make_pair(GLFW_KEY_A, false));
        keyboard.insert(std::make_pair(GLFW_KEY_S, false));
        keyboard.insert(std::make_pair(GLFW_KEY_D, false));
        keyboard.insert(std::make_pair(GLFW_KEY_LEFT_SHIFT, false));
    }

    void Camera::updateViewMat() {
        float sinYaw = glm::sin(glm::radians(rotation.x));
        float cosYaw = glm::cos(glm::radians(rotation.x));
        float sinPitch = glm::sin(glm::radians(rotation.y));
        float cosPitch = glm::cos(glm::radians(rotation.y));

        glm::vec3 xaxis = glm::vec3(cosYaw, 0, -sinYaw);
        glm::vec3 yaxis = glm::vec3(sinYaw * sinPitch, cosPitch, cosYaw * sinPitch);
        glm::vec3 zaxis = glm::vec3(sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw);

        view = glm::mat4(
            glm::vec4(       xaxis.x,            yaxis.x,            zaxis.x,      0 ),
            glm::vec4(       xaxis.y,            yaxis.y,            zaxis.y,      0 ),
            glm::vec4(       xaxis.z,            yaxis.z,            zaxis.z,      0 ),
            glm::vec4( -glm::dot( xaxis, worldpos ), -glm::dot( yaxis, worldpos ), -glm::dot( zaxis, worldpos ), 1 )
        );
    }

    //runs every frame
    void Camera::processInput(float dt) {

        //get mouse position difference from last frame
        double currmousex, currmousey;
        float xdiff, ydiff;
        glfwGetCursorPos(win, &currmousex, &currmousey);
        xdiff = currmousex - mx;
        ydiff = currmousey - my;
        mx = currmousex;
        my = currmousey;
        rotation.x -= xdiff*0.1;
        rotation.y -= ydiff*0.1;

        //bounds check
        if (rotation.y > 90.f) rotation.y = 90.f;
        if (rotation.y < -90.f) rotation.y = -90.f;

        float mov = keyboard.at(GLFW_KEY_LEFT_SHIFT) ? dt * runspeed : dt * walkspeed;

        glm::vec3 frontvec(glm::row(view, 2));
        frontvec.y = 0.f;
        frontvec = glm::normalize(frontvec);
        glm::vec3 rightvec(glm::row(view, 0));
        rightvec.y = 0.f;
        rightvec = glm::normalize(rightvec);

        if (keyboard.at(GLFW_KEY_W)) {
            worldpos -= frontvec*mov;
        }
        if (keyboard.at(GLFW_KEY_S)) {
            worldpos += frontvec*mov;
        }
        if (keyboard.at(GLFW_KEY_A)) {
            worldpos -= rightvec*mov;
        }
        if (keyboard.at(GLFW_KEY_D)) {
            worldpos += rightvec*mov;
        }

        updateViewMat();

    }


