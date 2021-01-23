
uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

void main(void) {
	vec4 color=vec4(1.0);
//#ifdef SHADOWS
	color.rgb=texture2DRect(FBOTex3, gl_FragCoord.xy).rgb;
//#endif
	gl_FragData[0]=color;
}

