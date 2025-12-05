// rocks3d.vert
varying vec3 fragNormal;

void main(void) {
    gl_Position = ftransform();
    fragNormal = normalize(gl_NormalMatrix * gl_Normal);
}