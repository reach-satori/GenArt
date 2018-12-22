#include "shader_util.h"
#include <glm/glm.hpp>
#include "globals.h"

class Geometry {
    private:
        shader_prog pshader;
        glm::vec3 position;
        float angle;
        GLuint VAO, numFaces;
        float scale;
        const glm::mat4 &projectionMatrix;
        const glm::mat4 &viewMatrix;
    public:
        Geometry(const char *objfile, const char *vshader, const char *fshader);
        void importMesh(const char *objfile);
        void render();
        void setScale(float scale);
        void setAngle(float angle);
        void setPos(glm::vec3 position);
};
