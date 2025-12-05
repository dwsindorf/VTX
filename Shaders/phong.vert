#version 120

// Vertex attributes
attribute vec3 position;
attribute vec3 normal;

// Uniforms
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

// Outputs to fragment shader
varying vec3 fragPosition;
varying vec3 fragNormal;

void main() {
    // Transform position to eye space
    vec4 eyePosition = modelViewMatrix * vec4(position, 1.0);
    fragPosition = eyePosition.xyz;
    
    // Transform normal to eye space
    fragNormal = normalize(normalMatrix * normal);
    
    // Transform to clip space
    gl_Position = projectionMatrix * eyePosition;
}