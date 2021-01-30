//########################### GLSLMgr Class ########################

#include <math.h>
#include <NoiseFuncs.h>
#include <GLSLMgr.h>
#include <Effects.h>
#include "FileUtil.h"
#include "MapClass.h"

extern Point MapPt;

#define PERMTEX  "permTexture"
#define PERLINTEX  "PerlinTexture"
#define GRADTEX  "gradTexture"
#define N3DTEX   "noise3DTexture"
#define DEPTHTEX "depthTexture"

#define FBOREAD  "fbo_read"
#define FBOWRITE "fbo_write"

#define MINGLSLVERSION  120
#define SIMPLEX_NOISE
#define DEBUG_SHADERS
//#define DEBUG_FBO
//#define DEBUG_CLR

//#define FBO_DEPTH
#define FORCEMINGLVERSION true
#define PERLIN_1D_TEX
static char *TEXNAME[6]={"FBOTex1","FBOTex2","FBOTex3","FBOTex4","FBOTex5","FBOTex6"};

char 			GLSLMgr::shader_dir[256]="shaders";
double 			GLSLMgr::gls_version=0;
char 			GLSLMgr::defString[4096]="";
char 			GLSLMgr::verString[256]="";
char 			GLSLMgr::extString1[256]="";
char 			GLSLMgr::extString2[256]="";
GLint 			GLSLMgr::TexCoordsID=-1;
GLint 			GLSLMgr::CommonID=-1;
GLint 			GLSLMgr::attributes3ID=-1;
GLint 			GLSLMgr::attributes4ID=-1;
GLint 			GLSLMgr::position1ID=-1;
GLint 			GLSLMgr::position2ID=-1;

GLuint          GLSLMgr::noise3DTextureID=0;
GLuint          GLSLMgr::permTextureID=0;
GLuint          GLSLMgr::PerlinTextureID=0;
GLuint          GLSLMgr::gradTextureID=0;
GLSLProgram    *GLSLMgr::program=0;
GLuint			GLSLMgr::fbotexs[NUMFBOTEXS]={0};
GLuint			GLSLMgr::fbo_rect=0;
GLuint			GLSLMgr::dbo_rect=0;

int 			GLSLMgr::width=0;
int 			GLSLMgr::height=0;
bool 	        GLSLMgr::using3DNoiseTexture=false;
bool            GLSLMgr::renderToFrameBuffer=true; // while false rendering -> FBO
bool 	        GLSLMgr::show_warnings=false;
NameList<GLSLProgram*> GLSLMgr::shaders;
GLSLVarMgr      GLSLMgr::vars;
double 			GLSLMgr::wscale=0;
double 			GLSLMgr::pscale=1.0/256;
int 			GLSLMgr::pass=0;
int 			GLSLMgr::unused_tex_units=0;

int 			GLSLMgr::input_type=GL_POINTS;
int 			GLSLMgr::output_type=GL_TRIANGLE_STRIP;
int 			GLSLMgr::max_output=1;
int 			GLSLMgr::tesslevel=0;


static bool using_fbo=false;

//-------------------------------------------------------------
// GLSLMgr::constructor()
//-------------------------------------------------------------
GLSLMgr::GLSLMgr(){
	setDefString("");
	setVerString("");
}
//-------------------------------------------------------------
// GLSLMgr::destructor()
//-------------------------------------------------------------
GLSLMgr::~GLSLMgr(){
	shaders.free();
	glDeleteFramebuffers(1, &fbo_rect);

	glDeleteRenderbuffers(1, &dbo_rect);
	for(int i=0;i<NUMFBOTEXS;i++){
		if(fbotexs[i]>0)
			glDeleteTextures(1,fbotexs+i);
	}
	if(PerlinTextureID>0)
		glDeleteTextures(1,&PerlinTextureID);
	if(permTextureID>0)
		glDeleteTextures(1,&permTextureID);
	if(gradTextureID>0)
		glDeleteTextures(1,&gradTextureID);
	if(noise3DTextureID>0)
		glDeleteTextures(1,&noise3DTextureID);
}

//-------------------------------------------------------------
// GLSLMgr::setShaderDir() set shader source directory
//-------------------------------------------------------------
bool GLSLMgr::setShaderDir(char *path){
	strcpy(shader_dir,path);
	return true;
}
//-------------------------------------------------------------
// GLSLMgr::setDefString() set #defines
//-------------------------------------------------------------
void GLSLMgr::setDefString(char *s){
	strcpy(defString,s);
}

//-------------------------------------------------------------
// GLSLMgr::clrDefString() set #defines
//-------------------------------------------------------------
void GLSLMgr::clrDefString(){
	defString[0]=0;
}

//-------------------------------------------------------------
// GLSLMgr::setVerString() set #defines
//-------------------------------------------------------------
void GLSLMgr::setVerString(char *s){
	strcpy(verString,s);
}

//-------------------------------------------------------------
// GLSLMgr::makeTexDefsFile() set texture #defines
// - generates set_tex.frag and set_tex.vert in Shaders directory
//-------------------------------------------------------------
void GLSLMgr::makeTexDefsFile(){
	char dir[256];
	char path[256];
  	File.getBaseDirectory(dir);

   	strcat(dir,File.separator);
   	strcat(dir,File.shaders);
   	strcat(dir,File.separator);
   	strcpy(path,dir);
   	strcat(path,"set_tex.frag");
	FILE *fp=fopen(path,"wb");
	if(!fp){
		cout << "file write error"<< endl;
	    return;
	}
	fprintf(fp,"#if NTEXS >0\n");
	fprintf(fp,"    TEX_VARS\n");
	fprintf(fp,"#endif\n");
	fprintf(fp,"#if NBUMPS >0\n");
	fprintf(fp,"    BUMP_VARS\n");
	fprintf(fp,"#endif\n");
	fprintf(fp,"#if NVALS >0\n");
	fprintf(fp,"    NOISE_VARS\n");
	fprintf(fp,"#endif\n\n");
	int n=unused_tex_units>16?16:unused_tex_units;
	for(int i=0;i<n;i++){
		fprintf(fp,"#ifdef TX%d\n",i);
		if(i<8){
			fprintf(fp,"#ifdef A%d\n",i);
			fprintf(fp,"    SET_ATTRIB(A%d)\n",i);
			fprintf(fp,"#else\n");
			fprintf(fp,"    SET_ATTRIB(1.0)\n");
			fprintf(fp,"#endif\n");
		}
		fprintf(fp,"INIT_TEX(%d,C%d)\n",i,i);
		fprintf(fp,"#ifdef N%d\n",i);
		fprintf(fp,"    SET_NOISE(N%d)\n",i);
		fprintf(fp,"#endif\n");
		fprintf(fp,"#ifdef X%d\n",i);
		fprintf(fp,"    SET_TEX(X%d)\n",i);
		fprintf(fp,"#endif\n");
		fprintf(fp,"#ifdef M%d\n",i);
		fprintf(fp,"    BGN_ORDERS\n");
		fprintf(fp,"#endif\n");
		fprintf(fp,"    APPLY_TEX\n");
		fprintf(fp,"#ifdef T%d\n",i);
		fprintf(fp,"    SET_COLOR\n");
		fprintf(fp,"#endif\n");
		fprintf(fp,"#ifdef B%d\n",i);
		fprintf(fp,"    SET_BUMP\n");
		fprintf(fp,"#endif\n");
		fprintf(fp,"#ifdef M%d\n",i);
		fprintf(fp,"    NEXT_ORDER\n");
		fprintf(fp,"    END_ORDERS\n");
		fprintf(fp,"#endif\n");
		fprintf(fp,"#endif\n\n");
	}
	fclose(fp);

	// make textures.h

   	strcpy(path,dir);
   	strcat(path,"textures.h");
	fp=fopen(path,"wb");
	if(!fp){
		cout << "file write error"<< endl;
	    return;
	}
	for(int i=0;i<n/2;i++){
		fprintf(fp,"#define CS%d gl_TexCoord[%d].xy\n",i*2,i);
		fprintf(fp,"#define CS%d gl_TexCoord[%d].zw\n",i*2+1,i);
	}
	fclose(fp);

	// make set_tex.vert

   	strcpy(path,dir);
   	strcat(path,"set_tex.vert");
	fp=fopen(path,"wb");
	if(!fp){
		cout << "file write error"<< endl;
	    return;
	}
	for(int i=0;i<n;i++){
		fprintf(fp,"#ifdef TX%d\n",i);
		fprintf(fp,"    CS%d=gl_MultiTexCoord%d.xy;\n",i,i);
		fprintf(fp,"#endif\n");
	}
	fclose(fp);

	// make set_attributes.vert

	char *swizzle="xyzw";
	strcpy(path,dir);
   	strcat(path,"set_attributes.vert");
	fp=fopen(path,"wb");
	if(!fp){
		cout << "file write error"<< endl;
	    return;
	}
	for(int i=0,j=0;i<4;i++,j++){
		fprintf(fp,"#ifdef A%d\n",i);
		fprintf(fp,"    AT%d = Attributes3.%c;\n",i,swizzle[j]);
		fprintf(fp,"#endif\n");
	}
	for(int i=0,j=0;i<4;i++,j++){
		fprintf(fp,"#ifdef A%d\n",i+4);
		fprintf(fp,"    AT%d = Attributes4.%c;\n",i+4,swizzle[j]);
		fprintf(fp,"#endif\n");
	}
	fclose(fp);
}
//-------------------------------------------------------------
// GLSLMgr::programValid() return true if program object is compiled and linked
//-------------------------------------------------------------
//bool GLSLMgr::programValid(char *defs,char *vshader,char *fshader){
//	char key[256];
//	sprintf(key,"%s %s %s",defs,vshader,fshader);
//	GLSLProgram::setKeyStr(key);
//	GLSLProgram *pgm=shaders.inlist(key);
//	if(pgm>0 && pgm->compiled&& pgm->linked)
//		return true;
//	return false;
//}

#define SET_COLOR_TEX \
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); \
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); \
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR); \
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR); \
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

#define SET_FLOAT_TEX \
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); \
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); \
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR); \
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);\
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA16F_ARB,  width, height, 0, GL_RGBA, GL_FLOAT, 0);

#define SET_DEPTH_TEX \
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); \
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); \
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); \
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); \
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE ); \
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE ); \
	glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT24_ARB,width, height,0, GL_DEPTH_COMPONENT, GL_FLOAT, 0 );

//-------------------------------------------------------------
// GLSLMgr::setVertexAttributes() set shader position attributes
//-------------------------------------------------------------
void GLSLMgr::setVertexAttributes(Point pm,double depth) {
	if (position1ID < 0 /*&& position2ID<0*/)
		return;
	double dfactor=0.5*wscale/depth;
	float max_orders =log2(dfactor);
	// passing in vertex as doubles doesn't work (even if supported by GPU)
	// because varying doubles (e.g. dvec3) aren't interpolated in fragment shader (flat shading only)

#ifdef DBLS_TEST
#define NSCALE 1024.0 // used for noise shader
	float vec2[2][3];
	double *pt=(double*)(&pm);
	for(int i=0;i<3;i++){
		double xpos=pt[i];
		vec2[0][i] = (float)xpos;
		vec2[1][i] = xpos - vec2[0][i];
	}
	glVertexAttrib4f(GLSLMgr::position1ID, vec2[0][0], vec2[0][1], vec2[0][2], max_orders); // high part
	glVertexAttrib4f(GLSLMgr::position2ID, vec2[1][0], vec2[1][1], vec2[1][2], depth);      // low part
	return;
	Point pv = MapPt.normalize(); // pivot point in triangle render loop projected onto unit sphere
	pv = pv * 0.5 + 0.5; // make axis signs all positive (0..1)
	Point pf = pm; // passed in with 0..1 values as for pv
	// increase separation between vertexes
	pf = pf * NSCALE;
	pv = pv * NSCALE;
	pv=pv.floor();
	// remove integer part to increase dynamic range of residuals
	pf = pf - pv; // remaining values 0..1 (but scaled * NSCALE)
	pf=pf/NSCALE; // restore original global scale
    glVertexAttrib4d(GLSLMgr::position2ID, pf.x, pf.y, pf.z, depth);
#endif
	glVertexAttrib4d(GLSLMgr::position1ID, pm.x, pm.y, pm.z, max_orders);
}

//-------------------------------------------------------------
// GLSLMgr::drawFrameBuffer() draw entire screen as two triangles
//-------------------------------------------------------------
void GLSLMgr::drawFrameBuffer(){
	//FrameBuffer::IsValid();
	glDepthMask(GL_FALSE);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POINT_SPRITE);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_3D);
	glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);

	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-1.0, -1.0, -0.5); // lower left

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-1.0, 1.0, -0.5);  // upper left

		glTexCoord2f(1.0f, 1.0f);     // upper right
		glVertex3f(1.0, 1.0, -0.5);

		glTexCoord2f(1.0f, 0.0f);     // lower right
		glVertex3f(1.0f, -1.0f, -0.5);
	glEnd();
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}



//Vertex1=s*(Vertex1_G[2]-Vertex1_G[0]) + t*(Vertex1_G[1]-Vertex1_G[0])+Vertex1_G[0];
static void test_geometry(int tesslevel){
	int triangle=1;
	double dt = 1.0 / double( tesslevel );
	double t_top = 1.0;
	int j=0;
	double f0=0,s=0,t;
	int v=0;
	int numv=tesslevel*(tesslevel+1);
	int ntop=0,nbot=1;
	for( int it = 0; it < tesslevel; it++ ){
		double t_bot = t_top - dt;
		double smax_top = 1.0 - t_top;
		double smax_bot = 1.0 - t_bot;
		int nums = it + 1;\
		double ds_top = smax_top / double(nums - 1 );
		double ds_bot = smax_bot / double(nums);
		double s_top = 0.0;
		double s_bot = 0.0;

		for( int is = 0; is < nums; is++ )
		{
			t=s_bot;s=t_bot;
			v=nbot+is;
            printf("%-3d %-3d s:%-1.2f t:%-1.2f\n",j++,v,s,t);

			v=ntop+is;
			t=s_top;s=t_top,f0=-t-s+1;

			printf("%-3d %-3d s:%-1.2f t:%-1.2f\n",j++,v,s,t);
			s_top += ds_top;
			s_bot += ds_bot;
		}
		v=nbot+nums;
		ntop=nbot;
		nbot+=nums+1;
		t=s_bot;s=t_bot,f0=-t-s+1;
		printf("%-3d %-3d s:%-1.2f t:%-1.2f\n",j++,v,f0,t);
		t_top = t_bot;
		t_bot -= dt;
	}
}

static void test_indexes(int tesslevel){
	int triangle=1;
	int index=0;
	int numr=tesslevel+1;
	int numv=tesslevel*(tesslevel+1);
	double V0=0, V2=numv-1,V1=V2-tesslevel;

	for( int row = 0; row <= tesslevel; row++ ){
		int nums = row+1 ;
		printf("\nROW %d :",row);
		for( int is = 0; is < nums; is++ )
		{
			printf(" %d ",index++);
		}
	}
	printf("\n");
}

static double test_data[50][3];

static void test_lookup(int tesslevel){
	int j=0;
	double s=0,t;
	int v=0;
	int ntop=0,nbot=1;
	for( int it = 0; it < tesslevel; it++ ){
		int nums = it + 1;\
		for( int is = 0; is < nums; is++ )
		{
			v=nbot+is;
			s=test_data[v][0];
			t=test_data[v][1];
            printf("%-3d %-3d s:%-1.2f t:%-1.2f\n",j++,v,s,t);
			v=ntop+is;
			s=test_data[v][0];
			t=test_data[v][1];
			printf("%-3d %-3d s:%-1.2f t:%-1.2f\n",j++,v,s,t);
		}
		v=nbot+nums;
		ntop=nbot;
		nbot+=nums+1;
		s=test_data[v][0];
		t=test_data[v][1];
		printf("%-3d %-3d s:%-1.2f t:%-1.2f\n",j++,v,s,t);
	}
}
static void test_process(int tesslevel){
	int index=0;
	int numr=tesslevel+1;
	int numv=tesslevel*(tesslevel+1);
	double s=1,t=0;
	double df=1.0/tesslevel;

	for( int row = 0; row <= tesslevel; row++,s-=df){
		int nums = row+1 ;
		t=0;
		for( int is = 0; is < nums; is++,t+=df)
		{
			test_data[index][0]=s;
			test_data[index][1]=t;
			index++;
		}
	}
}

//-------------------------------------------------------------
// GLSLMgr::InitGL() initialize openGL environment
//-------------------------------------------------------------
void GLSLMgr::initGL(int w, int h){
	//test_geometry(4);
	test_process(4);
	test_lookup(4);

	GLSupport::gl_init();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
	using_fbo=false;
    width=w;
    height=h;
    int max_tex_units=maxFragmentTextureUnits();
    int max_tex_id=max_tex_units-1;
    for(int i=0;i<NUMFBOTEXS;i++)
    	vars.newIntVar(TEXNAME[NUMFBOTEXS-i-1],max_tex_id--);
	vars.newIntVar(PERMTEX,max_tex_id--);
#ifdef PERLIN_1D_TEX
	vars.newIntVar(PERLINTEX,max_tex_id--);
#endif
	vars.newIntVar(N3DTEX,max_tex_id--);

#ifdef SIMPLEX_NOISE
	vars.newIntVar(GRADTEX,max_tex_id--);
#endif

	const GLubyte *glsl_version=glGetString(GL_SHADING_LANGUAGE_VERSION);
	gls_version=100*(glsl_version[0]-'0')+10*(glsl_version[2]-'0');
	//cout << glsl_version << endl;

	if(gls_version<=MINGLSLVERSION || FORCEMINGLVERSION){
		//strcat(verString,"#version 130\n");
		strcat(extString1,"#extension GL_ARB_texture_rectangle : enable\n");
		if(GLSupport::geometry_shader())
			strcat(extString2,"#extension GL_EXT_geometry_shader4 : enable\n");
	}
	cout << verString;
	if(strlen(extString1)>0)
		cout << extString1;
	if(strlen(extString2)>0)
		cout << extString2;

	vars.newBoolVar(FBOREAD,false);
	vars.newBoolVar(FBOWRITE,false);
	//float fmax = pow(2.0,13.0);
	unused_tex_units=max_tex_id;
	cout << "total  tex units:"<< max_tex_units << endl;
	cout << "unused tex units:"<< max_tex_id << endl;
	GLint maxAttach = 0;
	glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &maxAttach );
	cout << "max color buffer attachments:"<< maxAttach << endl;
	int maxVaryingFloats;
	glGetIntegerv (GL_MAX_VARYING_FLOATS_ARB, &maxVaryingFloats );
	printf("max varying vec4s = %d\n",maxVaryingFloats/4);
	GLint maxGeomVertices;
	glGetIntegerv( GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &maxGeomVertices );
	printf("max geometry output vetices = %d\n",maxGeomVertices);

	//gl_MaxGeometryOutputVertices

	makeTexDefsFile();
	vars.newFloatVar("fmax",pow(2.0,14.0));
	//vars.newFloatVar("nscale",2.0/NSCALE);
    initFrameBuffer();
	initPermTexture();
#ifdef PERLIN_1D_TEX
	initPerlinTexture();
#endif
#ifdef SIMPLEX_NOISE
	initGradTexture();
#endif
	setTessLevel(Map::tessLevel());
}

//-------------------------------------------------------------
// GLSLMgr::initFrameBuffer() initialize FBO textures
//-------------------------------------------------------------
void GLSLMgr::initFrameBuffer()
{
	GLint    v[4];
	glGetIntegerv(GL_VIEWPORT,v);
	width=v[2];
	height=v[3];

	if(fbo_rect>0)
		glDeleteFramebuffers(1, &fbo_rect);
	glGenFramebuffers(1, &fbo_rect);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo_rect);

	glGenRenderbuffers(1, &dbo_rect);
	glBindRenderbuffer(GL_RENDERBUFFER_EXT, dbo_rect);
	glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, dbo_rect);

	for(int i=0;i<NUMFBOTEXS;i++){
		glActiveTextureARB(GL_TEXTURE0+vars.getIntValue(TEXNAME[i]));
		if(fbotexs[i]>0)
			glDeleteTextures(1,fbotexs+i);
		glGenTextures(1, fbotexs+i);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, fbotexs[i]);
		//SET_FLOAT_TEX;
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#if MAXRGB==16
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA16F_ARB,  width, height, 0, GL_RGBA, GL_FLOAT, 0);
#else
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB,  width, height, 0, GL_RGBA, GL_FLOAT, 0);
#endif
		glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT+i, GL_TEXTURE_RECTANGLE_ARB, fbotexs[i], 0);
	}

	GLenum mrt[] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT};
	glDrawBuffers(2,mrt);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
		cout << "frameBuffer not valid" << endl;

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
	glActiveTextureARB(GL_TEXTURE0);
}

//-------------------------------------------------------------
// GLSLMgr::initPerlinTexture() initialize noise texture for 3D
//-------------------------------------------------------------
void GLSLMgr::initPerlinTexture()
{
    // Create and load the textures (generated, not read from a file)
    if(PerlinTextureID>0)
    	return;
     cout << "GLSLMgr::initPerlinTexture" << endl;

    glActiveTextureARB(GL_TEXTURE0+vars.getIntValue(PERLINTEX));
    glGenTextures(1, &PerlinTextureID); // Generate a unique texture ID
    glBindTexture(GL_TEXTURE_1D, PerlinTextureID); // Bind the texture

    char *pixels=(char *)NoiseFunc::makePerlinTexureImage();
    //GLfloat *pixels=(GLfloat *)NoiseFunc::makePerlinTexureImage();

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    ::free(pixels);
     glActiveTextureARB(GL_TEXTURE0); // Switch active texture unit back to 0 again
}

//-------------------------------------------------------------
// GLSLMgr::initPermTexture() initialize noise texture for 3D
//-------------------------------------------------------------
void GLSLMgr::initPermTexture()
{
    // Create and load the textures (generated, not read from a file)
    if(permTextureID>0)
    	return;

    cout << "GLSLMgr::initPermTexture" << endl;

    glActiveTextureARB(GL_TEXTURE0+vars.getIntValue(PERMTEX));

    /*
     * initPermTexture(GLuint *texID) - create and load a 2D texture for
     * a combined index permutation and gradient lookup table.
     * This texture is used for 2D and 3D noise, both classic and simplex.
     */
    glGenTextures(1, &permTextureID); // Generate a unique texture ID
    glBindTexture(GL_TEXTURE_2D, permTextureID); // Bind the texture

    char *pixels=(char*)NoiseFunc::makePermTexureImage();
    // GLFW texture loading functions won't work here - we need GL_NEAREST lookup.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /*
     * initGradTexture(GLuint *texID) - create and load a 2D texture
     * for a 4D gradient lookup table. This is used for 4D noise only.
     */
    ::free(pixels);
     glActiveTextureARB(GL_TEXTURE0); // Switch active texture unit back to 0 again
}

//-------------------------------------------------------------
// GLSLMgr::initGradTexture() initialize noise textures for 3D & 4D
//-------------------------------------------------------------
void GLSLMgr::initGradTexture()
{
    // Create and load the textures (generated, not read from a file)
    if(gradTextureID>0)
    	return;
    char *pixels;
    cout << "GLSLMgr::initGradTexture" << endl;
    pixels=NoiseFunc::makeGradTexureImage();

    glActiveTextureARB(GL_TEXTURE0+vars.getIntValue(GRADTEX)); // Activate a different texture unit
    glGenTextures(1, &gradTextureID); // Generate a unique texture ID
    glBindTexture(GL_TEXTURE_2D, gradTextureID); // Bind the texture to texture unit

    // GLFW texture loading functions won't work here - we need GL_NEAREST lookup.
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    ::free(pixels);
    glActiveTextureARB(GL_TEXTURE0); // Switch active texture unit back to 0 again
}

//-------------------------------------------------------------
// GLSLMgr::init3DNoiseTexture() initialize 3D noise texture
//-------------------------------------------------------------
void GLSLMgr::init3DNoiseTexture()
{
    if(noise3DTextureID==0){
	    void *pixels;
	    //glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, -1.0);
	    //glTexParameteri(GL_TEXTURE_3D, GL_GENERATE_MIPMAP, GL_TRUE);
	    glGenTextures(1, &noise3DTextureID); // Generate a unique texture ID
	    glActiveTexture(GL_TEXTURE0+vars.getIntValue(N3DTEX)); // Activate a different texture unit
	    glBindTexture(GL_TEXTURE_3D, noise3DTextureID); // Bind the texture

	    int size=128;
        //cout << "GLSLMgr::init3DNoiseTexture" << endl;
#define RGBA_3D_TEXTURE
#ifdef RGBA_3D_TEXTURE
	    pixels=NoiseFunc::makeNoise3DVectorTexureImage(size);
	    glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, size, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
#else
	    pixels=NoiseFunc::makeNoise3DTexureImage(size);
	    glTexImage3D( GL_TEXTURE_3D, 0, 1, size, size, size, 0, GL_RED, GL_UNSIGNED_BYTE, pixels );
#endif


		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    ::free(pixels);
    }
    glActiveTexture(GL_TEXTURE0+vars.getIntValue(N3DTEX)); // Activate a different texture unit
    glBindTexture(GL_TEXTURE_3D, noise3DTextureID); // Bind the texture

    using3DNoiseTexture=true;
 	glEnable(GL_TEXTURE_3D);
    glActiveTextureARB(GL_TEXTURE0); // Switch active texture unit back to 0 again
}

//-------------------------------------------------------------
// GLSLMgr::showShaderLog() show shader build status
//-------------------------------------------------------------
void GLSLMgr::showShaderLog(char *hdr, GLhandleARB id)
{
	char tmp[4096];
	glGetInfoLogARB(id, sizeof(tmp), 0, tmp);
	if(strlen(tmp)>0)
		cout << hdr <<  tmp << endl;
}

//-------------------------------------------------------------
// GLSLMgr::loadProgram() set active shader program
//-------------------------------------------------------------
bool GLSLMgr::loadProgram(char *vshader,char *fshader){
	buildProgram(vshader,fshader,"");
	return true;
}
//-------------------------------------------------------------
// GLSLMgr::loadProgram() set active shader program
//-------------------------------------------------------------
bool GLSLMgr::loadProgram(char *vshader,char *fshader,char *gshader){
	buildProgram(vshader,fshader,gshader);
	return true;
}
//-------------------------------------------------------------
// GLSLMgr::setMaxOutput() set max vertexs in geometry shader
//-------------------------------------------------------------
void GLSLMgr::setTessLevel(int n){
	max_output=(n+1)*(n+3);
	if(n != tesslevel)
		cout <<"GLSLMgr::setTessLevel "<<n<<" max_output="<<max_output<<endl;
	tesslevel=n;
}
//-------------------------------------------------------------
// GLSLMgr::buildProgram() set active shader program
//-------------------------------------------------------------
bool GLSLMgr::buildProgram(char *vshader,char *fshader,char *gshader){
	static char tmp[4096];
	bool first=false;
	bool geom=strlen(gshader)>0?true:false;
	sprintf(tmp," %s%s %s",defString,vshader,fshader);
	if(geom)
		sprintf(tmp+strlen(tmp)," %s",gshader);

	GLSLProgram::setKeyStr(tmp);
	position1ID=-1;
	position2ID=-1;
	TexCoordsID=-1;
	CommonID=-1;
	program=shaders.inlist(tmp);

	if(!program){
		if(geom)
			program=new GLSLProgram(defString,vshader,fshader,gshader);
		else
			program=new GLSLProgram(defString,vshader,fshader);
#ifdef DEBUG_SHADERS
		cout << "building shader :"<< program->name() << endl;
#endif
		shaders.add(program);
		shaders.sort();
		first=true;
	}
	else if(!program->compiled && !program->linked && !program->errors)
		first=true;

	if(strcmp(fshader,"noise3D.frag")==0){
		init3DNoiseTexture();
	}

	if(!program->compiled && !program->errors)
		compile();
	if(geom && !program->errors){
		//cout<<"GLSLMgr - setting tesslevel:"<<tesslevel<<" max_outputs:"<<max_output<<endl;
		glProgramParameteriEXT(program->program, GL_GEOMETRY_VERTICES_OUT_EXT, max_output);
		glProgramParameteriEXT(program->program,GL_GEOMETRY_INPUT_TYPE_EXT,input_type);
		glProgramParameteriEXT(program->program,GL_GEOMETRY_OUTPUT_TYPE_EXT,output_type);
	}

	if(!program->linked && !program->errors){
		link();
	}
	if(program->errors){
		if(first){
			cout << "ERRORS in: "<< program->name() << endl;
			showShaderLog("vertex program: ", program->vertex_shader);
			showShaderLog("fragment program: ",program->fragment_shader);
		}
		if(geom)
			showShaderLog("geometry program: ",program->geometry_shader);
	    glUseProgramObjectARB(0);
	}
	else{
		glUseProgramObjectARB(programHandle());
#ifdef DEBUG_SHADER_ERRORS
		if(first){
			showShaderLog("vertex program: ", program->vertex_shader);
			showShaderLog("fragment program: ",program->fragment_shader);
		}
#endif
	}
	return program->errors?false:true;
}

//-------------------------------------------------------------
// GLSLMgr::setProgram() set common program attributes
//-------------------------------------------------------------
void GLSLMgr::setProgram(){
	GLhandleARB program=programHandle();
 	CommonID=glGetAttribLocation(program,"CommonAttributes"); // Constants
	TexCoordsID=glGetAttribLocation(program,"TextureAttributes"); // Tangent
	position1ID=glGetAttribLocation(program,"Position1");  // vertex 1
	position2ID=glGetAttribLocation(program,"Position2");  // vertex 2
 	attributes3ID=glGetAttribLocation(program,"Attributes3"); // texture attribs
 	attributes4ID=glGetAttribLocation(program,"Attributes4"); // texture attribs

}

//-------------------------------------------------------------
// GLSLMgr::reloadShaders() reload shaders
//-------------------------------------------------------------
void GLSLMgr::reloadShaders()
{
	for(int i=0;i<shaders.size;i++)
		shaders[i]->compiled=shaders[i]->linked=shaders[i]->errors=false;
}
void GLSLMgr::loadVars(){
	GLint    v[4];
	glGetIntegerv(GL_VIEWPORT,v);
	GLhandleARB handle=programHandle();
	GLint loc = glGetUniformLocation(handle, "screen"); // height
	if(loc>=0)
		glUniform2f(loc,1.0/(float)v[2],1.0/(float)v[3]);
	vars.loadProgram();
	vars.loadVars();
}

//-------------------------------------------------------------
// GLSLMgr::beginRender() start render pass
//-------------------------------------------------------------
void GLSLMgr::beginRender(){
	glUseProgramObjectARB(0);
	//using_fbo=false;
	using3DNoiseTexture=false;
	setDefString("");
}
//-------------------------------------------------------------
// GLSLMgr::endRender() finish render pass
//-------------------------------------------------------------
void GLSLMgr::endRender(){
	//setFBORenderPass();
	if(!using3DNoiseTexture){
	    glActiveTexture(GL_TEXTURE0+vars.getIntValue(N3DTEX));
	    glBindTexture(GL_TEXTURE_3D, 0);
		glDisable(GL_TEXTURE_3D);
		glActiveTexture(GL_TEXTURE0);
	}
	//setFBONoPass();
	glUseProgramObjectARB(0);
}

void GLSLMgr::clrDepthBuffer(){
	glClear(GL_DEPTH_BUFFER_BIT);
#ifdef DEBUG_CLR
	cout << "GLSLMgr::clrDepthBuffer"<<endl;
#endif

}
void GLSLMgr::clrColorBuffer(){
	glClear(GL_COLOR_BUFFER_BIT);
#ifdef DEBUG_CLR
	cout << "GLSLMgr::clrColorBuffer"<<endl;
#endif
}
void GLSLMgr::clrBuffers(){
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

#ifdef DEBUG_CLR
	cout << "GLSLMgr::clrBuffers"<<endl;
#endif
}

void GLSLMgr::clrFBODepthBuffers(){
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo_rect);
	float v0[1]={1.0};
	glClearBufferfv(GL_DEPTH,0,v0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

}
void GLSLMgr::clrFBOColorBuffer(int i){
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo_rect);
#ifdef DEBUG_FBO
	cout << "GLSLMgr::clrBuffer("<<i<<")"<<endl;
#endif
	//glClear(GL_COLOR_BUFFER_BIT);
	float v1[4]={0,0,0,0};
	glClearBufferfv(GL_COLOR,i,v1);
}

//-------------------------------------------------------------
// GLSLMgr::clearTexs() clear texture bindings
//-------------------------------------------------------------
void GLSLMgr::clearTexs(){
#ifdef DEBUG_CLR
	cout << "GLSLMgr::clearTexs"<<endl;
#endif
	for(int i=0;i<8;i++){
	 	glActiveTexture(GL_TEXTURE0+i);
	 	glBindTexture(GL_TEXTURE_2D, 0);
	 	glBindTexture(GL_TEXTURE_1D, 0);
	 	glDisable(GL_TEXTURE_2D);
	 	glDisable(GL_TEXTURE_1D);
	}
	glDisable(GL_TEXTURE_3D);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glActiveTexture(GL_TEXTURE0);
}

void GLSLMgr::setFBORenderPass(){
	if(renderToFrameBuffer){
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
		using_fbo=false;
	}
	vars.setValue(FBOWRITE,false);
	vars.setValue(FBOREAD,false);

#ifdef DEBUG_FBO
	cout << "GLSLMgr::setFBORenderPass("<< using_fbo << ")"<<endl;
#endif
}
void GLSLMgr::setFBOReadPass(){
	if(renderToFrameBuffer){
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
		using_fbo=false;
	}
	vars.setValue(FBOWRITE,false);
	vars.setValue(FBOREAD,true);
#ifdef DEBUG_FBO
	cout << "GLSLMgr::setFBOReadPass("<<using_fbo<<")"<< endl;
#endif
}

void GLSLMgr::setFBOReset(){
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo_rect);

#ifdef DEBUG_FBO
	cout << "GLSLMgr::setFBOReset()"<<endl;
#endif
	glClearColor(0,0,0,0);
	//clrBuffers();
	float v0[1]={1.0};
	float v1[4]={0,0,0,1};
	glClearBufferfv(GL_DEPTH,0,v0);
	for(int i=0;i<NUMFBOTEXS;i++){
		glClearBufferfv(GL_COLOR,i,v1);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // enable transparency
	pass=0;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
	glUseProgramObjectARB(0);
	using_fbo=false;
	vars.setValue(FBOWRITE,false);
	vars.setValue(FBOREAD,false);
}

void GLSLMgr::setFBOWritePass(){
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo_rect);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
	glDisable(GL_BLEND);
	//glBindTexture(GL_TEXTURE_2D, 0);
	using_fbo=true;
	vars.setValue(FBOWRITE,true);
	vars.setValue(FBOREAD,false);
#ifdef DEBUG_FBO
	cout << "GLSLMgr::setFBOWritePass("<<using_fbo<<")"<< endl;
#endif
}
void GLSLMgr::setFBOReadWritePass(){
	if(pass==0){
		pass++;
		setFBOWritePass();
		return;
	}
	pass++;
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo_rect);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

	using_fbo=true;

	vars.setValue(FBOWRITE,true);
	vars.setValue(FBOREAD,true);
#ifdef DEBUG_FBO
	cout << "GLSLMgr::setFBOReadWritePass("<<using_fbo<<")"<< endl;
#endif
}

