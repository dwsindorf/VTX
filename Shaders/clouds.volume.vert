// ########## begin clouds.volume.vert #########################
// Full screen quad for ray-marched volumetric clouds (CLOUDS_VOLUME).
// The quad is drawn in normalized device coordinates (identity matrices);
// the fragment shader builds each pixel's view ray from the corner rays
// set by CloudLayer::setProgram().
varying vec2 ScreenUV;   // 0..1 across the viewport

void main(void) {
	gl_Position = vec4(gl_Vertex.xy,0.0,1.0);
	ScreenUV = gl_Vertex.xy*0.5+0.5;
}
// ########## end clouds.volume.vert #########################
