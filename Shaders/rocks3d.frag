#version 120

varying vec3 Normal;
varying vec3 EyeDir;

uniform vec4 Diffuse;
uniform vec4 Ambient;

vec3 setLighting(vec3 BaseColor) {
    vec3 diffuse = vec3(0.0);
    vec3 normal = normalize(Normal);
    
    for (int i = 0; i < NLIGHTS; i++) {
        vec3 light = normalize(gl_LightSource[i].position.xyz);
        float LdotN = max(dot(light, normal), 0.0);
        float intensity = 1.0 / gl_LightSource[i].constantAttenuation / float(NLIGHTS);
        diffuse += Diffuse.rgb * gl_LightSource[i].diffuse.rgb * LdotN * intensity;
    }
    
    vec3 TotalDiffuse = BaseColor * diffuse * Diffuse.a + Ambient.rgb * Ambient.a;
    return TotalDiffuse;
}

void main() {
    vec3 rockColor = vec3(0.6, 0.5, 0.4);  // Base rock color
    vec3 color = setLighting(rockColor);
    gl_FragColor = vec4(color, 1.0);
}