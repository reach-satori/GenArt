#version 400
#extension GL_ARB_explicit_uniform_location : enable

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;
out vec3 interpolatedColor;
out vec2 fraguv;

void main(void) {
    interpolatedColor = vec3(0.5, 0.5, 0.5) * abs(dot(normal , normalize(vec3(10., 10., 10.))));
    fraguv = uv;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
