#ifndef GLSLMGR_H_
#define GLSLMGR_H_

#include "GLSupport.h"
#include <GLSLProgram.h>
#include <PointClass.h>
#include <ListClass.h>

#define NUMFBOTEXS 4
#define NUMDRAWBUFS 2
class GLSLMgr {
	static void makeTexDefsFile();
	static void initPermTexture();
	static void initPerlinTexture();
	static void initGradTexture();
	static void initShadowTexture();
	static bool compile(){
		return program->compile();
	}

	static bool link(){
		return program->link();
	}

	static bool linked(){
		return program->linked;
	}

	static bool compiled(){
		return program->compiled;
	}

	static bool errors(){
		return program->errors;
	}
public:
	static int width,height;
	static GLSLProgram *program;
	static GLSLVarMgr vars;
	static int input_type;
	static int output_type;
	static int max_output;
	static double gls_version;
	GLSLMgr();
	~GLSLMgr();

	static void init3DNoiseTexture();

	static void showShaderLog(char *hdr, GLhandleARB);
	static char defString[4096];
	static char verString[256];
	static char extString1[256];
	static char extString2[256];
	static bool show_warnings;
	static bool using3DNoiseTexture;
	static bool renderToFrameBuffer;


	static void beginRender();
	static void endRender();
	static void clearTexs();

	static UniformBool fboRead;
	static UniformBool fboWrite;
	static int unused_tex_units;

	static char shader_dir[256]; // relative path to shader files
	static GLuint		   fbotexs[NUMFBOTEXS];  // FBO textures
	static GLuint		   fbo_rect; // FBO texture used as a color array
	static GLuint		   dbo_rect; // FBO texture used as a data array

	static NameList<GLSLProgram*> shaders;
	static double wscale;
	static double pscale;
	static GLuint noise3DTextureID;
	static GLuint permTextureID;
	static GLuint PerlinTextureID;
	static GLuint gradTextureID;
	static GLint TexCoordsID;
	static GLint CommonID;
	static GLint attributes3ID;
	static GLint attributes4ID;
	static GLint position1ID;
	static GLint position2ID;

	static int pass;
	static void initGL(int w, int h);
	static void reloadShaders();

	static void loadVars();
	static void drawFrameBuffer();
	static void setVertexAttributes(Point pp, double depth);
	static bool loadProgram(char *vshader,char *fshader);
	static bool loadProgram(char *vshader,char *fshader,char *gshader);
	static bool buildProgram(char *vshader,char *fshader,char *gshader);
	static bool setShaderDir(char *rpath);
	static GLhandleARB programHandle(){
		return program->program;
	}
	static void setDefString(char *);
	static void clrDefString();
	static void setVerString(char *);
	static void setProgram();
	static void clrDepthBuffer();
	static void clrColorBuffer();
	static void clrBuffers();

	static void clrBuffers(int i);

	static void clrFBOColorBuffer(int i);
	static void clrFBODepthBuffers();
	static void setFBOReset();
	static void setFBORenderPass();
	static void setFBOWritePass();
	static void setFBOReadPass();
	static void setFBOReadWritePass();

	static void initFrameBuffer();

	static void setFunction(int id, float *data);

	static void printVersion() {
		const char
				* slVer = (const char *) glGetString (GL_SHADING_LANGUAGE_VERSION_ARB );
		printf("GLSL Version: %s\n", slVer);
	}

	static int maxVertexUniformComponents() {
		int maxVertexUniformComponents;
		glGetIntegerv (GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB,
				&maxVertexUniformComponents );
		return maxVertexUniformComponents;
	}

	static int maxVertexAttribs() {
		int maxVertexAttribs;
		glGetIntegerv (GL_MAX_VERTEX_ATTRIBS_ARB, &maxVertexAttribs );
		return maxVertexAttribs;
	}

	static int maxFragmentTextureUnits() {
		int maxFragmentTextureUnits;
		glGetIntegerv (GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &maxFragmentTextureUnits );
		return maxFragmentTextureUnits;
	}

	static int maxFragmentProgramTextureUnits() {
		return unused_tex_units; // top indexes reserved
	}

	static int maxVertexTextureUnits() {
		int maxVertexTextureUnits;
		glGetIntegerv (GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB,
				&maxVertexTextureUnits );
		return maxVertexTextureUnits;
	}

	static int maxCombinedTextureUnits() {
		int maxCombinedTextureUnits;
		glGetIntegerv (GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB,
				&maxCombinedTextureUnits );
		return maxCombinedTextureUnits;
	}

	static int maxVaryingFloats() {
		int maxVaryingFloats;
		glGetIntegerv (GL_MAX_VARYING_FLOATS_ARB, &maxVaryingFloats );
		return maxVaryingFloats;
	}

	static int maxFragmentUniformComponents() {
		int maxFragmentUniformComponents;
		glGetIntegerv (GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB,
				&maxFragmentUniformComponents );
		return maxFragmentUniformComponents;
	}

	static int maxTextureCoords() {
		int maxTextureCoords;
		glGetIntegerv (GL_MAX_TEXTURE_COORDS_ARB, &maxTextureCoords );
		return maxTextureCoords;
	}

};
#endif /*GLSLMGR_H_*/
