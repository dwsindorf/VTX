//#extension GL_ARB_gpu_shader_fp64 : enable
// notes:
// 1. This shader supports up to 8 textures in a single pass
//    (may be less if textures contain modulation attributes)
// 2. The shader uses preprocessor directives to enable only the components that
//    are actually used in a given texture
// 3. Texture and modulation attributes are packed into varying vec4 objects to
//    optimize the use of hardware resources (e.g. On NVidia 1400 only 8 varying
//    vec4s are supported)
// 4. In the current scheme, varying vec4 packing is most efficient if textures that
//    have modulation attributes are consecutive and start on even texture id boundaries
//    (The shader compiler/linker doesn't penalize for varying vec4 objects that are
//    not used but counts half-full vec4 units the same as full ones).

#include "attributes.h"
#include "textures.h"
#include "attributes.vert"
#include "common.h"

varying vec4 EyeDirection;
varying vec4 Normal;

void main(void) {
	// note: original vertex coords are in eye space (0,0,0=eye) but not rotated for view
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	gl_Position = ftransform();

	Normal.xyz = gl_NormalMatrix * gl_Normal;

#include "set_common.vert"

#include "set_tex.vert"
#include "set_attributes.vert"

}
