uniform float ws1;
uniform float ws2;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;

void main(void) {
	vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy); // eye depth 

	float depth=gl_FragCoord.z;
	gl_FragData[0] = fcolor1;
	gl_FragData[1] = vec4(depth,fcolor1.r,0,1);
}

