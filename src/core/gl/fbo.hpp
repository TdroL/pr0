#ifndef GL_FBO_HPP
#define GL_FBO_HPP

#include <GL/glew.h>

#include "../util.hpp"
#include "types.hpp"
#include "mesh.hpp"
#include "tex2d.hpp"
#include "renderbuffer.hpp"
#include "program.hpp"

#include <list>
#include <vector>
#include <string>
#include <initializer_list>

namespace gl
{

class FBO
{
public:
	enum DepthType
	{
		Renderbuffer,
		Texture,
	};

	struct TexContainer
	{
		bool enabled{false};
		gl::Tex2D tex{};

		void bind(GLsizei unit)
		{
			tex.bind(unit);
		}
	};

	struct DepthContainer
	{
		DepthType type{Renderbuffer};

		gl::Renderbuffer buf{
			/* .internalFormat= */ GL_DEPTH24_STENCIL8,
		};
		gl::Tex2D tex{
			/* .internalFormat= */ GL_DEPTH24_STENCIL8,
			/* .format= */ GL_DEPTH_STENCIL,
			/* .type= */ GL_UNSIGNED_INT_24_8
		};

		GLenum getAttachmentType()
		{
			if (type == Renderbuffer)
			{
				return buf.getAttachmentType();
			}
			else
			{
				return tex.getAttachmentType();
			}
		}
	};

	static std::list<FBO *> collection;
	static std::vector<FBO *> activeStack;
	static gl::Mesh mesh;
	static gl::Program prog;
	static void reloadAll();
	static void reloadSoftAll();

	static void init();

	GLuint id = 0;

	std::vector<TexContainer> colors{};
	DepthContainer depth{};

	glm::vec4 clearColor{0.f, 0.f, 0.f, 0.f};

	int width = 0;
	int height = 0;

	std::string fboName = "Unnamed FBO";

	FBO();
	explicit FBO(std::string &&name);
	~FBO();

	void setTex(size_t id, gl::Tex2D &&tex);
	void setDepth(gl::Tex2D &&tex);
	void setDepth(gl::Renderbuffer &&buf);

	void create();
	void reset();
	void resetStorages();

	void reload();
	void reloadSoft();

	void blit(GLuint target, GLbitfield mask);

	void clear();

	void use();
	void release();
};

class FBOScoper
{
public:
	FBO &fbo;

	FBOScoper(FBO &fbo) : fbo(fbo) { fbo.use(); }
	~FBOScoper() { fbo.release(); }
};

#define GL_FBO_USE(fboSource) gl::FBOScoper UTIL_CONCAT2(FBOScoper, __COUNTER__)(fboSource)

} // gl

#endif