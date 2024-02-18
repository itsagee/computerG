#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the input locations of attributes
layout(location = 0) in vec3 vertCoordinates_in;
layout(location = 1) in vec3 vertColor_in;

// Specify the Uniforms of the vertex shader
uniform mat4 modelTransform;
uniform mat4 projectionTransform;

// Specify the output of the vertex stage
out vec3 vertColor;

void main() {
  // gl_Position is the output (a vec4) of the vertex shader
  // we need to multiply the vertex with the matrices to reach the wished transformation
    // Assuming modelTranformation is a QMatrix4x4
  vec4 trans = modelTransform* vec4(vertCoordinates_in, 1.0F);
  gl_Position = projectionTransform*trans;
  vertColor = vertColor_in;
}

//vec4(vertCoordinates_in, 1.0F);
