#include "consts.h"
// ---------------------------- Includes -------------------------- //
#include <stdlib.h>         // C++ standard library
#include <stack>            // We use the standard C++ stack implementation to create model matrix stacks
#include <memory>           // Smart pointers
#include <vector>
#include <unistd.h>         // Threading
#include <stdio.h>          // Input/Output
#include <GLEW/glew.h>      // OpenGL Extension Wrangler -
//#include <GL/glew.h> // this is the default include folder location in ubuntu...
#include <GLFW/glfw3.h>     // Windows and input
#include <glm/glm.hpp>      // OpenGL math library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader_util.h"    // Utility methods to keep this file a bit shorter.
#include "testpaintings.h"
#include "simplepainting.h"
#include "input.h"
#include "camera.h"
#include "cylinder.h"
#include "halfsphere.h"

// so far i've only added to this globals header globals which need to be visible across multiple files:
// keyboard and cam
// other main.cpp only globals are defined below
#include "globals.h"
using std::vector;
using std::unique_ptr;
using std::make_unique;

//our globals
shader_prog basicshader("shaders/basic.vert.glsl", "shaders/basic.frag.glsl");
GLuint floorVAO, paintingVAO, cylinderVAO, domeVAO;

// all functions defined in main.cpp... it's ugly but i'm ok with it //
GLuint createQuad(glm::vec3 color, float s);
vector<unique_ptr<Painting>> makePaintings();
void initGeom();
void drawWorld();
GLuint importMesh(const std::string& pFile);

vector<unique_ptr<Painting>> makePaintings() {
    //just making and placing them manually...

    vector<const char*> fragshaders{
        "shaders/bad_noise_pattern.frag.glsl",
        "shaders/pulsingcircles.frag.glsl",
        "shaders/psychconcentric.frag.glsl",
        "shaders/ojgreen.frag.glsl",
        "shaders/canvas.frag.glsl",
        "shaders/gears.frag.glsl",
        "shaders/rainy.frag.glsl",
        "shaders/boringsines.frag.glsl",
        "shaders/dotclock.frag.glsl",
        "shaders/trigonometric_modulus.frag.glsl",
        "shaders/joydivision.frag.glsl",
    };

    //can't use a vector initializer {} because unique_ptr can't be copied... therefore we gotta pushback them
    vector<unique_ptr<Painting>> vec;
    vec.reserve(30);//should be enough to avoid resizing

    for (int i = 0; i < fragshaders.size(); i++) {
        auto p = make_unique<SimplePainting>("shaders/basic.vert.glsl",fragshaders[i]);
        p->position = glm::vec3(-80.f+i*40, 10.f, -30.f);
        vec.push_back(std::move(p));
    }


    return vec;
}

void initGeom() {
    floorVAO = createQuad(glm::vec3(0.22, 0.22, 0.22), 50);
    paintingVAO = createQuad(glm::vec3(0.50, 0.50, 0.50), 15);
    cylinderVAO = importMesh("data/cylinder.obj");
    domeVAO = importMesh("data/halfsphere.obj");
}

GLuint createQuad(glm::vec3 color, float s) {
    GLfloat vertexdata[] = {
                            -s, -s, 0.0, color[0], color[1], color[2], 0.f, 1.f,
                             s, -s, 0.0, color[0], color[1], color[2], 1.f, 1.f,
                             s,  s, 0.0, color[0], color[1], color[2], 1.f, 0.f,
                            -s,  s, 0.0, color[0], color[1], color[2], 0.f, 0.f
                        };

    GLubyte indices[] = {
                            0, 1, 2,
                            0, 2, 3
                        };

    GLuint vertexArrayHandle;
    glGenVertexArrays(1, &vertexArrayHandle);
    glBindVertexArray(vertexArrayHandle);

    GLuint vboHandle;
    glGenBuffers(1, &vboHandle);
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexdata), vertexdata, GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERTEX_POSITION_LOC);
    //indexes are defined inside the vertex shader itself with layout specification
    glVertexAttribPointer(
        VERTEX_POSITION_LOC,
        3,                 // number of elements per vertex, here (r,g,b)
        GL_FLOAT,          // the type of each element
        GL_FALSE,          // take our values as-is
        8*sizeof(float),                 // no extra data between each position
        (const GLvoid*)(0*sizeof(float))                  // offset of first element
    );
    glEnableVertexAttribArray(COLOR_LOC);
    glVertexAttribPointer(
        COLOR_LOC,
        3,
        GL_FLOAT,
        GL_FALSE,
        8*sizeof(float),
        (const GLvoid*)(3*sizeof(float))
    );
    glEnableVertexAttribArray(UV_LOC);
    glVertexAttribPointer(
        UV_LOC,
        2,
        GL_FLOAT,
        GL_FALSE,
        8*sizeof(float),
        (const GLvoid*)(6*sizeof(float))
    );

    glGenBuffers(1, &vboHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLfloat)*6, indices, GL_STATIC_DRAW);
    //We return handle to vertex array.
    return vertexArrayHandle;
}


GLuint importMesh( const std::string& pFile ) {
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(pFile,
        aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);

    if (!scene){
        printf("Error importing a file: %s\n", importer.GetErrorString());
        return 0;
    }

    printf("Loaded mesh: %s \n", scene->mMeshes[0][0].mName.C_Str());

    aiMesh *imported = scene->mMeshes[0];
    printf("number of vertices: %d, number of faces: %d\n", imported->mNumVertices, imported->mNumFaces);


    ////////////////////////////////////

    std::vector<GLfloat> vertexdata;
    vertexdata.reserve(imported->mNumVertices * 5);
    std::vector<GLuint> indices;

    for (int i = 0; i < imported->mNumVertices; i++) {
        vertexdata.push_back(imported->mVertices[i].x * 50);
        vertexdata.push_back(imported->mVertices[i].y * 50);
        vertexdata.push_back(imported->mVertices[i].z * 50);
        vertexdata.push_back(imported->mTextureCoords[0][i].x);
        vertexdata.push_back(imported->mTextureCoords[0][i].y);
        vertexdata.push_back(imported->mNormals[i].x);
        vertexdata.push_back(imported->mNormals[i].y);
        vertexdata.push_back(imported->mNormals[i].z);

    }
    // we know its triangles
    for (unsigned int i = 0; i < imported->mNumFaces; i++) {
        for (int j = 0; j < imported->mFaces[i].mNumIndices; j++){
            indices.push_back((GLuint)imported->mFaces[i].mIndices[j]);
        }
    }

    GLuint vertexArrayHandle;
    glGenVertexArrays(1, &vertexArrayHandle);
    glBindVertexArray(vertexArrayHandle);

    GLuint vboHandle;
    glGenBuffers(1, &vboHandle);
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vertexdata.size(), &vertexdata[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERTEX_POSITION_LOC);
    //indexes are defined inside the vertex shader itself with layout specification
    glVertexAttribPointer(
        VERTEX_POSITION_LOC,
        3,                 // number of elements per vertex, here
        GL_FLOAT,          // the type of each element
        GL_FALSE,          // take our values as-is
        8*sizeof(GLfloat),
        (const GLvoid*)(0*sizeof(GLfloat))                  // offset of first element
    );

    //vx, vy, vz, u, v, nx, ny, nz
    glEnableVertexAttribArray(NORMAL_LOC);
    glVertexAttribPointer(
        NORMAL_LOC,
        3,                 // number of elements per vertex, here
        GL_FLOAT,          // the type of each element
        GL_FALSE,          // take our values as-is
        8*sizeof(GLfloat),
        (const GLvoid*)(5*sizeof(GLfloat))                  // offset of first element
    );

    glEnableVertexAttribArray(UV_LOC);
    glVertexAttribPointer(
        UV_LOC,
        2,
        GL_FLOAT,
        GL_FALSE,
        8*sizeof(GLfloat),
        (const GLvoid*)(3*sizeof(GLfloat))
    );

    glGenBuffers(1, &vboHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLfloat)*indices.size(), &indices[0], GL_STATIC_DRAW);
    return vertexArrayHandle;
}

//probably replace this by a container with all non-special geometry to be drawn later(clear out separate global geometry VAOS)
void drawWorld() {
    basicshader.begin();
    basicshader.uniformMatrix4fv("viewMatrix", cam.view);
    std::stack<glm::mat4> ms;
    ms.push(glm::mat4(1.0));
    ms.push(ms.top()); //Floor
        ms.top() = glm::rotate(ms.top(), glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        ms.top() = glm::translate(ms.top(), glm::vec3(0.0, 0.0, -10.0));
        basicshader.uniformMatrix4fv("modelMatrix", ms.top());
        glBindVertexArray(floorVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
    ms.pop();
    basicshader.end();
}


int main(int argc, char *argv[]) {
    GLFWwindow *win;

    if (!glfwInit()) {
        exit (EXIT_FAILURE);
    }

    win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);
    if (!win) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    cam.win = win;

    glfwMakeContextCurrent(win);
    glewExperimental = GL_TRUE;
    GLenum status = glewInit();

    if(status != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(status));
    }

    const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString (GL_VERSION); // version as a string
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported %s\n", version);
    glfwSetKeyCallback(win, key_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    basicshader.setup();
    basicshader.begin();
    basicshader.uniformMatrix4fv("projectionMatrix", cam.projection);
    basicshader.uniformMatrix4fv("viewMatrix", cam.view);

    initGeom();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //create a vector containing (unique) pointers to our "paintings": they are initialized inside this makePaintings function
    auto paintings = makePaintings();
    double currentTime, dt, lastTime = 0;

    /* auto cylshader = make_unique<Cylinder>(); */
    /* cylshader->position = glm::vec3(0.f, 10.f, 30.f); */
    /* cylshader->angle = 90.; */

    auto dome = make_unique<HalfSphere>();
    dome->position = glm::vec3(0.f, 30.f, 20.f);
    dome->angle = 0.;



    while (!glfwWindowShouldClose(win)) {
        currentTime = glfwGetTime();
        dt = (currentTime - lastTime);
        lastTime = currentTime;
        cam.processInput(dt);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        drawWorld();

        for (const auto &p : paintings) {
            p->render(paintingVAO);
        }

        /* cylshader->render(cylinderVAO); */
        dome->render(domeVAO);


        glfwSwapBuffers(win);
        glfwPollEvents();
        usleep(1000);
    }
    //clear it out

    glfwTerminate();
    exit(EXIT_SUCCESS);

    return 0;
}
