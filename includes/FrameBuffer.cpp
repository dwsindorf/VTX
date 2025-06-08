
#define USE_RECT_2D
#include <GLheaders.h>
#include <FrameBuffer.h>
#include <iostream>
using namespace std;

FrameBuffer::FrameBuffer()
		: m_fboId(_GenerateFboId()),
		  m_savedFboId(0)
{
	// Bind this FBO so that it actually gets created now
	_GuardedBind();
	_GuardedUnbind();
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffersEXT(1, &m_fboId);
}

void FrameBuffer::Bind()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboId);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
}

void FrameBuffer::Disable()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
}

void
FrameBuffer::AttachTexture( GLenum attachment, GLenum texType,
								  GLuint texId, int mipLevel, int zSlice)
{
	_GuardedBind();

#ifndef NDEBUG
	if( GetAttachedId(attachment) != texId ) {
#endif

	_FramebufferTextureND( attachment, texType,
						   texId, mipLevel, zSlice );

#ifndef NDEBUG
	}
	else {
		cerr << "FrameBuffer::AttachTexture PERFORMANCE WARNING:\n"
			 << "\tRedundant bind of texture (id = " << texId << ").\n"
			 << "\tHINT : Compile with -DNDEBUG to remove this warning.\n";
	}
#endif

	_GuardedUnbind();
}

void
FrameBuffer::AttachRenderBuffer( GLenum attachment, GLuint buffId )
{
	_GuardedBind();

#ifndef NDEBUG
	if( GetAttachedId(attachment) != buffId ) {
#endif

	glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, attachment,
								   GL_RENDERBUFFER_EXT, buffId);

#ifndef NDEBUG
	}
	else {
		cerr << "FrameBuffer::AttachRenderBuffer PERFORMANCE WARNING:\n"
			 << "\tRedundant bind of Renderbuffer (id = " << buffId << ")\n"
			 << "\tHINT : Compile with -DNDEBUG to remove this warning.\n";
	}
#endif

	_GuardedUnbind();
}

void
FrameBuffer::Unattach( GLenum attachment )
{
	_GuardedBind();
	GLenum type = GetAttachedType(attachment);

	switch(type) {
	case GL_NONE:
	break;
	case GL_RENDERBUFFER_EXT:
		AttachRenderBuffer( attachment, 0 );
	break;
	case GL_TEXTURE:
		AttachTexture( attachment, GL_TEXTURE_2D, 0 );
	break;
	default:
		cerr << "FrameBuffer::unbind_attachment ERROR: Unknown attached resource type\n";
	}
	_GuardedUnbind();
}

GLint FrameBuffer::GetMaxColorAttachments()
{
	GLint maxAttach = 0;
	glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &maxAttach );
	return maxAttach;
}

GLuint FrameBuffer::_GenerateFboId()
{
	GLuint id = 0;
	glGenFramebuffers(1, &id);
	return id;
}

void FrameBuffer::_GuardedBind()
{
	// Only binds if m_fboId is different than the currently bound FBO
	glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &m_savedFboId );
	if (m_fboId != (GLuint)m_savedFboId) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboId);
	}
}

void FrameBuffer::_GuardedUnbind()
{
	// Returns FBO binding to the previously enabled FBO
	if ((GLuint)m_savedFboId != m_fboId) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)m_savedFboId);
	}
}

void
FrameBuffer::_FramebufferTextureND( GLenum attachment, GLenum texType,
										  GLuint texId, int mipLevel,
										  int zSlice )
{
#ifdef WIN32
	if (texType == GL_TEXTURE_1D) {
		glFramebufferTexture1DEXT( GL_FRAMEBUFFER_EXT, attachment,
								   GL_TEXTURE_1D, texId, mipLevel );
	}
	else
#endif
if (texType == GL_TEXTURE_3D) {
		glFramebufferTexture3DEXT( GL_FRAMEBUFFER_EXT, attachment,
								   GL_TEXTURE_3D, texId, mipLevel, zSlice );
	}
	else {
		// Default is GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB, or cube faces
		glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, attachment,
								   texType, texId, mipLevel );
	}
}

#ifndef NDEBUG
bool FrameBuffer::IsValid( ostream& ostr )
{
  _GuardedBind();

  bool isOK = false;

  GLenum status;
  status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(status) {
  case GL_FRAMEBUFFER_COMPLETE_EXT: // Everything's OK
    isOK = true;
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
    ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
		 << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n";
    isOK = false;
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
    ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
		 << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT\n";
    isOK = false;
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
    ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
		 << "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n";
    isOK = false;
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
    ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
		 << "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n";
    isOK = false;
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
    ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
	          << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n";
    isOK = false;
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
    ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
		 << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n";
    isOK = false;
    break;
  case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
    ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
		 << "GL_FRAMEBUFFER_UNSUPPORTED_EXT\n";
    isOK = false;
    break;
  default:
    ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
		 << "Unknown ERROR\n";
    isOK = false;
  }

  _GuardedUnbind();
  return isOK;
}
#endif // NDEBUG

/// Accessors
GLenum FrameBuffer::GetAttachedType( GLenum attachment )
{
	// Returns GL_RENDERBUFFER_EXT or GL_TEXTURE
	_GuardedBind();
	GLint type = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, attachment,
											 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT,
											 &type);
	_GuardedUnbind();
	return GLenum(type);
}

GLuint FrameBuffer::GetAttachedId( GLenum attachment )
{
	_GuardedBind();
	GLint id = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, attachment,
											 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT,
											 &id);
	_GuardedUnbind();
	return GLuint(id);
}

GLint FrameBuffer::GetAttachedMipLevel( GLenum attachment )
{
	_GuardedBind();
	GLint level = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, attachment,
											 GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT,
											 &level);
	_GuardedUnbind();
	return level;
}

GLint FrameBuffer::GetAttachedCubeFace( GLenum attachment )
{
	_GuardedBind();
	GLint level = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, attachment,
											 GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT,
											 &level);
	_GuardedUnbind();
	return level;
}

GLint FrameBuffer::GetAttachedZSlice( GLenum attachment )
{
	_GuardedBind();
	GLint slice = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, attachment,
											 GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT,
											 &slice);
	_GuardedUnbind();
	return slice;
}



