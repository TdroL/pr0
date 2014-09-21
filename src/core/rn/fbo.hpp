#ifndef RN_FBO_HPP
#define RN_FBO_HPP

#include <list>
#include <vector>
#include <string>
#include <initializer_list>

#include "../rn.hpp"
#include "../util.hpp"
#include "types.hpp"
#include "mesh.hpp"
#include "tex2d.hpp"
#include "renderbuffer.hpp"
#include "program.hpp"

namespace rn
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
		rn::Tex2D tex{};

		void bind(GLsizei unit)
		{
			tex.bind(unit);
		}
	};

	struct DepthContainer
	{
		DepthType type{Renderbuffer};

		rn::Renderbuffer buf{
			/* .internalFormat= */ GL_DEPTH24_STENCIL8,
		};
		rn::Tex2D tex{
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
	static rn::Mesh mesh;
	static rn::Program prog;
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

	void setTex(size_t id, rn::Tex2D &&tex);
	void setDepth(rn::Tex2D &&tex);
	void setDepth(rn::Renderbuffer &&buf);

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

#define RN_FBO_USE(fboSource) rn::FBOScoper UTIL_CONCAT2(FBOScoper, __COUNTER__)(fboSource)

} // rn

#endif