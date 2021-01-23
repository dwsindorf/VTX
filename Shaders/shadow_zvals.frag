
//uniform sampler2DRect FBOTex1;
//uniform sampler2DRect FBOTex2;
//uniform sampler2DRect FBOTex3;
//uniform sampler2DRect FBOTex4;

void main(void) {
	vec4 color=vec4(0,0,0,1.0);
	//color.rgb=texture2DRect(FBOTex3, gl_FragCoord.xy).rgb;  // old test 
	gl_FragData[0]=color;
	gl_FragData[1]=vec4(1,0,0,1);	
}

