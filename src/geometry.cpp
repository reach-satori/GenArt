
#include "geometry.h"
#include "consts.h"
#include <stack>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


Geometry::Geometry(const char *objfile, const char* vshader, const char* fshader) :
    pshader(vshader, fshader),
    position(glm::vec3(0)),
    angle(0.f),
    VAO(1),
    projectionMatrix(cam.projection),
    viewMatrix(cam.view)

    {
        importMesh(objfile);
        pshader.setup();
        pshader.begin();

        //this uniform stays constant so it doesn't have to be changed after initial setup
        pshader.uniformMatrix4fv("projectionMatrix", projectionMatrix);
        pshader.end();
    };

void Geometry::importMesh(const char *objfile) {
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(objfile,
        aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);

    if (!scene){
        printf("Error importing a file: %s\n", importer.GetErrorString());
    }

    printf("Loaded mesh: %s \n", scene->mMeshes[0][0].mName.C_Str());

    aiMesh *imported = scene->mMeshes[0];
    printf("number of vertices: %d, number of faces: %d\n", imported->mNumVertices, imported->mNumFaces);
    numFaces = imported->mNumFaces;


    ////////////////////////////////////

    std::vector<GLfloat> vertexdata;
    vertexdata.reserve(imported->mNumVertices * 5);
    std::vector<GLuint> indices;

    for (unsigned int i = 0; i < imported->mNumVertices; i++) {
        vertexdata.push_back(imported->mVertices[i].x);
        vertexdata.push_back(imported->mVertices[i].y);
        vertexdata.push_back(imported->mVertices[i].z);
        vertexdata.push_back(imported->mTextureCoords[0][i].x);
        vertexdata.push_back(imported->mTextureCoords[0][i].y);
        vertexdata.push_back(imported->mNormals[i].x);
        vertexdata.push_back(imported->mNormals[i].y);
        vertexdata.push_back(imported->mNormals[i].z);

    }
    // we know its triangles
    for (unsigned int i = 0; i < imported->mNumFaces; i++) {
        for (unsigned int j = 0; j < imported->mFaces[i].mNumIndices; j++){
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
        3,
        GL_FLOAT,
        GL_FALSE,
        8*sizeof(GLfloat),
        (const GLvoid*)(0*sizeof(GLfloat))
    );

    //vx, vy, vz, u, v, nx, ny, nz
    glEnableVertexAttribArray(NORMAL_LOC);
    glVertexAttribPointer(
        NORMAL_LOC,
        3,                 // number of elements per vertex, here
        GL_FLOAT,          // the type of each element
        GL_FALSE,          // take our values as-is
        8*sizeof(GLfloat), // stride
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

    VAO = vertexArrayHandle;
}

void Geometry::setScale(float scalein) {
    scale = scalein;
}
void Geometry::setAngle(float anglein) {
    angle = anglein;
}
void Geometry::setPos(glm::vec3 positionin) {
    position = positionin;
}

void Geometry::render() {
    std::stack<glm::mat4> ms;
    ms.push(glm::mat4(1.0));

    //set up the shaders, uniforms
    //rendering is as usual, but beginning and ending their own shaders, as well as updating necessary uniforms
    pshader.begin();
    pshader.uniformMatrix4fv("viewMatrix", viewMatrix);
    glUniform1f(TIME_LOC, (float)glfwGetTime());

    ms.push(ms.top());
        ms.top() = glm::translate(ms.top(), position);
        ms.top() = glm::rotate(ms.top(), glm::radians(angle), glm::vec3(0., 1., 0.));
        ms.top() = glm::rotate(ms.top(), glm::radians(-90.f), glm::vec3(1., 0., 0.));
        ms.top() = glm::scale(ms.top(), glm::vec3(scale));
        //maybe we can even set up the modelMatrix only once in constructor as well if they don't move around
        pshader.uniformMatrix4fv("modelMatrix", ms.top());
        glDisable(GL_CULL_FACE);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, numFaces * 3, GL_UNSIGNED_INT, 0);
        glEnable(GL_CULL_FACE);
    ms.pop();
    pshader.end();
};

