
//  GLSupport.h

#ifndef _GLSUPPORT
#define _GLSUPPORT

#define SUPPORT_MULTI_TEX // enables ARB multi-texture extensions

#include "GLglue.h"
class GLSupport
{
	static int have_functions;
protected:
	static int ARB_multitexture_supported;
	static int ARB_texture_cube_map_supported;
	static int ARB_texture_env_combine_supported;
	static int ARB_texture_env_dot3_supported;
	static int ARB_shading_language_100_supported;
	static int ARB_shader_objects_supported;
	static int EXT_framebuffer_object_supported;
	static int ARB_geometry_shader4_supported;

	static int specflag;
	static GLuint white_texture;
	static void setContext();
public:
	static int gl_version;
	static int txchnls;
	static int extensionSupported(const char *extension);
	static int multiTextures();
	static int multiBumps();
	static int glsl();
	static int geometry_shader();
	static bool renderTextures();

	static void set_spectex();
	static void clr_spectex();
	static void set_spectex(int i);
	static int spectex();
	static int spec_two_pass();
	static void gl_init();
};
#endif
