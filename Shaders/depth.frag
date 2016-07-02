uniform float ws1;
uniform float ws2;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;

void main(void) {
	float depth=gl_FragCoord.z;
	gl_FragData[0] = vec4(depth,0,0,1);
}

