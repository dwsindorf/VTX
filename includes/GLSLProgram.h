#ifndef GLSLPROGRAM_H_
#define GLSLPROGRAM_H_

#include <GLSLVars.h>

class GLSLProgram
{
public:
	bool linked;
	bool compiled;
	bool errors;
	GLhandleARB  vertex_shader;
	GLhandleARB  fragment_shader;
	GLhandleARB  geometry_shader;
	GLhandleARB  program;

	char shader_name[2048];
	char shader_defs[2048];
	char fragment_file[512];
	char vertex_file[512];
	char geometry_file[512];
	char *name();
	GLSLProgram();
	GLSLProgram(char *p, char *v, char *f);
	GLSLProgram(char *p, char *v, char *f,char *g);
	~GLSLProgram();
	bool compile();
	bool link();

	static void setKeyStr(char *s);
	virtual void initVars();
	virtual void loadVars();

	unsigned char* readShaderFile(char *defines,char *filename);
	bool buildShader(int id, char *defs, char* file);
	bool linkProgram(GLhandleARB program);
};


#endif /*GLSLPROGRAM_H_*/
