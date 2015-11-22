#ifndef RN_FB_HPP
#define RN_FB_HPP

#include <vector>
#include <string>
#include <memory>

#include "../rn.hpp"
#include "types.hpp"
#include "tex.hpp"
#include "tex2d.hpp"
#include "tex2darray.hpp"
#include "../util.hpp"

namespace rn
{

class FB
{
public:
	static std::vector<FB *> collection;
	static std::vector<FB *> activeStack;

	static void resetAll();
	static void reloadAll();
	static void reloadSoftAll();

	struct TexContainer
	{
		std::shared_ptr<rn::Tex> tex{};
		GLint layer = 0;
		GLint level = 0;
	};

	GLuint id = 0;

	GLsizei width = 0;
	GLsizei height = 0;

	glm::vec4 clearColorValue{0.f, 0.f, 0.f, 0.f};
	GLfloat clearDepthValue = 0.f;
	GLint clearStencilValue = 0;

	std::vector<TexContainer> colorContainers{};
	TexContainer depthContainer{};

	std::string fbName = "Unnamed FB";

	explicit FB(size_t colorsSize = 0);
	FB(std::string &&name, size_t colorsSize = 0);
	~FB();

	void attachColor(size_t index, const std::shared_ptr<rn::Tex2D> &tex, GLint level = 0);
	void attachColor(size_t index, const std::shared_ptr<rn::Tex2DArray> &tex, GLsizei layer, GLint level = 0);
	void attachDepth(const std::shared_ptr<rn::Tex2D> &tex, GLint level = 0);
	void attachDepth(const std::shared_ptr<rn::Tex2DArray> &tex, GLsizei layer, GLint level = 0);

	TexContainer detachColor(size_t index);
	TexContainer detachDepth();

	rn::Tex * color(size_t index);
	rn::Tex * depth();

	std::shared_ptr<rn::Tex> shareColor(size_t index);
	std::shared_ptr<rn::Tex> shareDepth();

	void clear(BuffersMask mask);
	void clearColor(size_t layer);
	void clearDepthStencil();

	void blit(FB &target, BuffersMask mask, MagFilter filter = MAG_NEAREST);
	void blit(FB *target, BuffersMask mask, MagFilter filter = MAG_NEAREST);

	void reload();
	void reloadSoft();

	void reset();

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