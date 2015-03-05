#ifndef RN_FB_HPP
#define RN_FB_HPP

#include <vector>
#include <string>
#include <memory>

#include "../rn.hpp"
#include "types.hpp"
#include "tex2d.hpp"
#include "../util.hpp"

namespace rn
{

class FB
{
public:
	static std::vector<FB *> collection;
	static std::vector<FB *> activeStack;

	static void reloadAll();
	static void reloadSoftAll();

	struct Tex2DContainer
	{
		std::shared_ptr<rn::Tex2D> tex{};
		GLint level = 0;
	};

	GLuint id = 0;

	GLsizei width = 0;
	GLsizei height = 0;

	glm::vec4 clearColor{0.f, 0.f, 0.f, 0.f};
	GLfloat clearDepth = 1.f;

	std::vector<Tex2DContainer> colorContainers{};
	Tex2DContainer depthContainer{};

	std::string fbName = "Unnamed FB";

	explicit FB(size_t colorsSize = 0);
	FB(std::string &&name, size_t colorsSize = 0);
	~FB();

	void attachColor(size_t index, const std::shared_ptr<rn::Tex2D> &tex, GLint level = 0);
	void attachDepth(const std::shared_ptr<rn::Tex2D> &tex, GLint level = 0);

	rn::Tex2D * color(size_t index);
	rn::Tex2D * depth();

	void clear(BuffersMask mask);

	void blit(const FB *target, BuffersMask mask, MagFilter filter = MAG_NEAREST);

	void reload();
	void reloadSoft();

	void bind();
	void unbind();
};

class FBScoper
{
public:
	FB &fb;

	FBScoper(FB &fb) : fb(fb) { fb.bind(); }
	~FBScoper() { fb.unbind(); }
};

#define RN_FB_BIND(fbSource) rn::FBScoper UTIL_CONCAT2(FBScoper, __COUNTER__)(fbSource)

} // rn

#endif