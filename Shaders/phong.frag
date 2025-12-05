#version 120

// Inputs from vertex shader
varying vec3 fragPosition;
varying vec3 fragNormal;

// Uniforms
uniform vec3 lightPosition;    // Light position in eye space
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

void main() {
    // Normalize the interpolated normal
    vec3 N = normalize(fragNormal);
    
    // Calculate light direction
    vec3 L = normalize(lightPosition - fragPosition);
    
    // Calculate view direction (camera is at origin in eye space)
    vec3 V = normalize(-fragPosition);
    
    // Calculate reflection direction
    vec3 R = reflect(-L, N);
    
    // Ambient component
    vec3 ambient = lightAmbient * materialAmbient;
    
    // Diffuse component
    float diffuseStrength = max(dot(N, L), 0.0);
    vec3 diffuse = lightDiffuse * materialDiffuse * diffuseStrength;
    
    // Specular component
    float specularStrength = 0.0;
    if (diffuseStrength > 0.0) {
        specularStrength = pow(max(dot(R, V), 0.0), materialShininess);
    }
    vec3 specular = lightSpecular * materialSpecular * specularStrength;
    
    // Combine all components
    vec3 finalColor = ambient + diffuse + specular;
    
    gl_FragColor = vec4(finalColor, 1.0);
}