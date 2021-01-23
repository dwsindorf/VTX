
uniform sampler2DRect FBOTex1;

void main(void) {
	vec4 color=vec4(1.0);

	color.rgb=texture2DRect(FBOTex1, gl_FragCoord.xy).rgb;

	gl_FragData[0]=color;  // write to FBOTex3
}

