#version 120

uniform mat4 smat;
varying vec4 ShadowCoord;
varying vec4 EyeDirection;
varying vec4 Normal;

void main() {
    vec3 p     = gl_Vertex.xyz;
    vec4 VertexPosition = gl_ModelViewMatrix * vec4(p, 1.0);
    //gl_Position  = gl_ProjectionMatrix * VertexPosition;
    ShadowCoord  = smat * VertexPosition;
    EyeDirection = VertexPosition;              // eye-space position as direction
    Normal       = vec4(gl_Normal, 0.0); // not used without _NORMALS_ but must be declared
    gl_Position=ftransform();
}
