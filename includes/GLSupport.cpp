// GLSupport.cpp
#include "GLSupport.h"
#include <stdio.h>

//#define SHOW_ALL_EXTENSIONS // get a printout of all supported extensions

int GLSupport::ARB_multitexture_supported=0;
int GLSupport::ARB_texture_cube_map_supported=0;
int GLSupport::ARB_texture_env_combine_supported=0;
int GLSupport::ARB_texture_env_dot3_supported=0;
int GLSupport::ARB_shading_language_100_supported=0;
int GLSupport::ARB_shader_objects_supported=0;
int GLSupport::ARB_geometry_shader4_supported=0;
int GLSupport::EXT_framebuffer_object_supported=0;
int GLSupport::gl_version=0;
int GLSupport::specflag=1;
int GLSupport::txchnls=0;
int GLSupport::have_functions=0;
GLuint GLSupport::white_texture=0;

//************************************************************
// GLSupport class
//************************************************************

int GLSupport::extensionSupported(const char *extension)
{
  static const GLubyte *extensions = NULL;
  const GLubyte *start;
  GLubyte *where, *terminator;

  where = (GLubyte *) strchr(extension, ' ');
  if (where || *extension == '\0')
    return 0;

  if (!extensions)
    extensions = glGetString(GL_EXTENSIONS);

  start = extensions;
  for (;;) {
    where = (GLubyte *) strstr((const char *) start, extension);
    if (!where)
      break;
    terminator = where + strlen(extension);
    if (where == start || *(where - 1) == ' ') {
      if (*terminator == ' ' || *terminator == '\0') {
        return 1;
      }
    }
    start = terminator;
  }
  return 0;
}

//-------------------------------------------------------------
// GLSupport::set_spectex() load white texture for ogl1.1 specular pass
//-------------------------------------------------------------
void GLSupport::setContext()
{
}
int GLSupport::multiTextures()
{
#ifdef SUPPORT_MULTI_TEX
	return ARB_multitexture_supported;
#else
	return 0;
#endif
}

int GLSupport::multiBumps()
{
#ifdef SUPPORT_MULTI_TEX
	return ARB_multitexture_supported
		&& ARB_texture_env_combine_supported
		&& ARB_texture_cube_map_supported
		&& ARB_texture_env_dot3_supported;
#else
    return 0;
#endif
}

int GLSupport::glsl()
{
	return ARB_shading_language_100_supported
		&& ARB_shader_objects_supported;
}

int GLSupport::geometry_shader()
{
	return ARB_shading_language_100_supported
		&& ARB_geometry_shader4_supported;
}

bool  GLSupport::renderTextures(){
    return EXT_framebuffer_object_supported;
}
//-------------------------------------------------------------
// GLSupport::gl_init() initialize
//-------------------------------------------------------------
void GLSupport::gl_init()
{
	static int initialized=0;
	if(!initialized){
		const GLubyte *version=glGetString(GL_VERSION);
		const GLubyte *glsl_version=glGetString(GL_SHADING_LANGUAGE_VERSION);
		gl_version=10*(version[0]-'0');
		gl_version+=version[2]-'0';
		ARB_multitexture_supported=extensionSupported("GL_ARB_multitexture");
		ARB_texture_env_combine_supported=extensionSupported("GL_ARB_texture_env_combine");
		ARB_texture_cube_map_supported=extensionSupported("GL_ARB_texture_cube_map");
		ARB_texture_env_dot3_supported=extensionSupported("GL_ARB_texture_env_dot3");
		ARB_shading_language_100_supported=extensionSupported("GL_ARB_shading_language_100");
		ARB_shader_objects_supported=extensionSupported("GL_ARB_shader_objects");
		EXT_framebuffer_object_supported=extensionSupported("GL_EXT_framebuffer_object");
		ARB_geometry_shader4_supported=extensionSupported("GL_ARB_geometry_shader4");
		printf("OGL  version : %s (%d)\n",version,gl_version);
		printf("GLSL version : %s \n",glsl_version);
#ifdef SHOW_ALL_EXTENSIONS
		const GLubyte *exts=glGetString(GL_EXTENSIONS);
        printf("%s\n",exts);
#endif
		initialized=1;
	}

	setContext();

	// make white texture for ogl1.1 specular pass
	//if(gl_version==11){
	//	glGenTextures (1, &white_texture);
	//	glBindTexture (GL_TEXTURE_2D, white_texture);
	//}
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	//glHint(GL_POINT_SMOOTH_HINT, GL_NICEST );

	//glPolygonOffset(0.1,1);
	//glEnable(GL_POLYGON_OFFSET_FILL);
	int auxbufs=0;

	glGetIntegerv(GL_SAMPLE_BUFFERS_ARB,&auxbufs);

	//glEnable(GL_POLYGON_SMOOTH);
 	//glEnable(GL_NORMALIZE);

   // unsigned char pixels[2*2*4];
    //memset (pixels, 0xff, 2*2*4);
    //glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 2, 2,
     //             0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	txchnls=1;
	if(multiTextures()){
#ifdef GL_MAX_TEXTURE_UNITS_ARB
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&txchnls);
#else
#ifdef GL_MAX_ACTIVE_TEXTURES_ARB
		glGetIntegerv(GL_MAX_ACTIVE_TEXTURES_ARB,&txchnls);
#endif
#endif
	    printf("max multi-texture channels = %d\n",txchnls);
	}
	else
		printf(" multi-textures support disabled\n");

	if(glsl()){
		printf("GLSL supported\n");
		if(geometry_shader())
			printf("Geometry shader supported\n");
		else
			printf("Geometry shader NOT supported\n");
		if(renderTextures())
			printf("Render to Texture supported\n");
		else
			printf("Render to Texture NOT supported\n");
	}
	else
		printf("GLSL NOT supported\n");

	set_spectex(1);
}
//-------------------------------------------------------------
// GLSupport::set_spectex() load white texture for ogl1.1 specular pass
//-------------------------------------------------------------
void GLSupport::set_spectex()
{
	if(gl_version==11){
		//glBlendFunc (GL_ONE, GL_ONE);
		//glColor3d (0, 0, 0);
		//glEnable (GL_BLEND);
		//glEnable(GL_TEXTURE_2D);
		//glBindTexture (GL_TEXTURE_2D, white_texture);
	}
}

//-------------------------------------------------------------
// GLSupport::clr_spectex() clr white texture
//-------------------------------------------------------------
void GLSupport::clr_spectex()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPopAttrib();
}

//-------------------------------------------------------------
// GLSupport::set_spectex() apply specular highlights to textures
//           note: supported in ogl 1.2 or later
//-------------------------------------------------------------
void GLSupport::set_spectex(int f)
{
	specflag=f;
	if(f)
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	else
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
}

//-------------------------------------------------------------
// GLSupport::spec_two_pass() return if ogl needs two pass textures
//-------------------------------------------------------------
int GLSupport::spec_two_pass()
{
    return 0; // ogl 1.1 two spec pass no longer supported
}

//-------------------------------------------------------------
// GLSupport::spectex() return state of specular textures flag
//-------------------------------------------------------------
int GLSupport::spectex()
{
	return specflag;
}
