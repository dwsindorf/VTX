varying vec3 fragNormal;

void main(void) {
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(vec3(1.0, 1.0, 1.0));  // Simple light direction
    
    float ambient = 0.3;
    float diffuse = max(dot(N, L), 0.0) * 0.7;
    
    vec3 color = vec3(0.6, 0.5, 0.4) * (ambient + diffuse);  // Brownish rock
    
    gl_FragColor = vec4(color, 1.0);
}