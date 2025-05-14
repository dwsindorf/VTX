/////////////////////////////////////////////////////////////////////////////
// Name:        GLSLProgram.cpp
/////////////////////////////////////////////////////////////////////////////

#define FUNCTION_WIDTH  4096
//#define MAKE_FADE_TEXTURE
#define EXPAND_INCLUDES // Expand all #includes explicitly
						// (avoids glsl's intermittent support for #include)
#include <GLSLMgr.h>
#include "FileUtil.h"


//#define DEBUG_SHADER_WARNINGS

//########################### GLSLProgram Class ########################

GLSLProgram::GLSLProgram(){
	linked=compiled=errors=false;
	strcpy(shader_name,"");
	strcpy(fragment_file,"");
	strcpy(vertex_file,"");
	strcpy(geometry_file,"");
	strcpy(shader_defs,"");
	vertex_shader=0;
	fragment_shader=0;
	geometry_shader=0;
	program=0;
}

GLSLProgram::GLSLProgram(char *p,char *v, char *f){
	linked=compiled=errors=false;

	strncpy(shader_name,p,1024);
	setKeyStr(shader_name);
	strcat(shader_name," ");
	strcat(shader_name,v);
	strcat(shader_name," ");
	strcat(shader_name,f);
	//sprintf(shader_name,"%s %s %s",p,v,f);

	strncpy(shader_defs,p,1024);
	strncpy(fragment_file,f,256);
	strncpy(vertex_file,v,256);
	geometry_file[0]=0;
	geometry_shader=0;


	vertex_shader=glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fragment_shader=glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	program=glCreateProgramObjectARB();
	glAttachObjectARB(program, vertex_shader);
	glAttachObjectARB(program, fragment_shader);
}

GLSLProgram::GLSLProgram(char *p,char *v, char *f,char *g){
	linked=compiled=errors=false;

	strncpy(shader_name,p,1024);
	setKeyStr(shader_name);
	strcat(shader_name," ");
	strcat(shader_name,v);
	strcat(shader_name," ");
	strcat(shader_name,f);
	strcat(shader_name," ");
	strcat(shader_name,g);

	strncpy(shader_defs,p,1024);
	strncpy(fragment_file,f,256);
	strncpy(vertex_file,v,256);
	strncpy(geometry_file,g,256);

	vertex_shader=glCreateShaderObjectARB(GL_VERTEX_SHADER);
	fragment_shader=glCreateShaderObjectARB(GL_FRAGMENT_SHADER);
	geometry_shader=glCreateShaderObjectARB(GL_GEOMETRY_SHADER_EXT);
	program=glCreateProgramObjectARB();

	glAttachObjectARB(program, vertex_shader);
	glAttachObjectARB(program, fragment_shader);
	glAttachObjectARB(program, geometry_shader);

}

GLSLProgram::~GLSLProgram(){
	if(program)
		glDeleteObjectARB(program);
	if(vertex_shader)
		glDeleteObjectARB(vertex_shader);
	if(fragment_shader)
		glDeleteObjectARB(fragment_shader);
	if(geometry_shader)
		glDeleteObjectARB(geometry_shader);
}

void GLSLProgram::setKeyStr(char *str){
	int n=strlen(str);
	int j=0;
	for(int i=0;i<n;i++){
		if(str[i]=='\n')
			i++;
		if (str[i]=='#'){
			while(str[i]!=' ')
				i++;
		}
		str[j++]=str[i];
	}
	str[j]=0;
}
char* GLSLProgram::name(){
	return shader_name;
}

#ifdef EXPAND_INCLUDES
// return text up to next \n
static int getline(char *str){
	int i=0;
	while(str[i]!=0){
		if(str[i]=='\n'|| str[i]=='\r')
			return i+1;
		i++;
	}
	return i;
}
// return text up to next "
static int getname(char *str,char*name){
	int i=0;
	while(str[i]!='\"'){
		name[i]=str[i];
		i++;
	}
	name[i]=0;
	return i;
}
#endif
/*
 * readShaderFile(filename) - read a shader source string from a file
 */
unsigned char* GLSLProgram::readShaderFile(char *defines, char *filename) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		cout << "ERROR: Cannot open shader file <" << filename << ">"<<endl;
		return 0;
	}

	fseek( file, 0, SEEK_END);
	int bytesinfile=ftell( file);
	fseek( file, 0, SEEK_SET);

	int buffsize=strlen(defines)+bytesinfile+1;
	char *buffer = (char*)malloc(buffsize);
	int start_index=0;
	int n=0;

	char *fbuffer=(char*)malloc(bytesinfile+1);
	int bytesread = fread(fbuffer, 1, bytesinfile, file);// file text
	fclose(file);

	n=getline(fbuffer); // read first line (n chars)
	if(strncmp("#version",fbuffer,8)==0){
	   start_index=n;
	   strncpy(buffer,fbuffer,start_index);
	}
	strcpy(buffer+start_index,defines);
	strncat(buffer,fbuffer+start_index,bytesinfile-start_index);
	buffer[bytesread+strlen(defines)] = 0; // Terminate the string with 0
	::free(fbuffer);

#ifdef EXPAND_INCLUDES // Expand all #includes
	char *lptr=buffer+strlen(defines)+start_index;
	char *start=buffer;
	n=0;
	int count=strlen(defines)+start_index;
	while((n=getline(buffer+count))>0){
		char name[256];
		lptr=buffer+count;
		if(strncmp("#include \"",lptr,10)==0){
			getname(lptr+10,name);
			lptr[0]=0;        // remove include line from start section
			char *rem=lptr+n; // append text after include line
			// recursive call to this function
			char *add=(char*)readShaderFile(start, name);
			// add=start+expanded include
			// combine start and rem parts
			char *newbuffer=(char*)malloc(strlen(add)+strlen(rem)+1);
			strcpy(newbuffer,add);
			strcat(newbuffer,rem);
			::free(add);
			::free(buffer);
			buffer=start=newbuffer;
			n=0;
		}
		count+=n;
	}
#endif
	buffer[strlen(buffer)]=0;
	return (unsigned char*)buffer;
}

/*
 * buildShader() - create, load, compile and link the GLSL shader objects.
 */
#define DEBUG_SHADER_FILES
bool GLSLProgram::buildShader(int id, char *defs, char* file) {
	GLint shaderCompiled;
	const char *shaderStrings[4];
	unsigned char *shaderAssembly = readShaderFile(defs,file);
	//shaderStrings[0] = (char*)GLSLMgr::verString;
	//shaderStrings[1] = (char*)GLSLMgr::extString1;
	//shaderStrings[2] = (char*)GLSLMgr::extString2;
	shaderStrings[0] = (char*)shaderAssembly;
	glShaderSourceARB(id, 1, shaderStrings, 0);

	glCompileShaderARB(id);
#ifdef DEBUG_SHADER_FILES
	char name[512];
	sprintf(name,"%s.debug",file);
	FILE *fp=fopen(name,"wb");
	if(!fp){
		cout << "could not save:" << name<< endl;
	    return false;
	}
	//	cout << "saving:" << name<< endl;
 	fprintf(fp,"%s\n",shaderAssembly);
	fclose(fp);

	//cout << shaderAssembly << endl;
#endif
#ifdef DEBUG_SHADER_WARNINGS
	free((void *)shaderAssembly);
	int bufflen;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &bufflen);
	    if (bufflen > 1)
	    {
	        GLchar* log_string = new char[bufflen + 1];
	        glGetShaderInfoLog(id, bufflen, 0, log_string);
	        cout << "Log found:"<<file<< "\n"<<log_string << endl;

	        delete log_string;
	    }
#endif
	glGetObjectParameterivARB(id, GL_OBJECT_COMPILE_STATUS_ARB,
			&shaderCompiled);
	if (shaderCompiled == GL_FALSE) {
		errors=true;
		//glGetInfoLogARB(id, sizeof(str), NULL, str);
		//GLSLMgr::printError("Program object linking error", str);
		cout << "Shader compile error ["<< id << "]" <<endl;
		return false;
	}
	return true;
}

/*
 * linkProgram() - create, load, compile and link the GLSL shader objects.
 */
bool GLSLProgram::linkProgram(GLhandleARB program) {
	GLint shadersLinked;
	// Link the program object and print out the info log.
	glLinkProgramARB(program);
	glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB,
			&shadersLinked);

	if (shadersLinked == GL_FALSE) {
		errors=true;
		cout << "Shader link error ["<< program << "]"<<endl;
		return false;
	}
	return true;
}

/*
 * link() - link the GLSL shader objects.
 */
bool GLSLProgram::link() {
	linked=linkProgram(program);
	return linked;
}

/*
 * compile() - compile the GLSL shader objects.
 */
bool GLSLProgram::compile() {

    bool result1=buildShader(vertex_shader,shader_defs,vertex_file);
    bool result2=buildShader(fragment_shader,shader_defs,fragment_file);
    compiled=(result1 && result2);
    if(geometry_shader && compiled){
    	compiled=compiled&buildShader(geometry_shader,shader_defs,geometry_file);
    }

	return compiled;
}

void GLSLProgram::initVars(){

}
void GLSLProgram::loadVars(){

}
