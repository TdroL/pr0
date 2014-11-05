#include "fbo.hpp"

#include "../rn.hpp"
#include "../util.hpp"
#include "../util/align.hpp"
#include "../util/str.hpp"
#include "../src/mem.hpp"
#include "../ngn.hpp"
#include "../rn/ext.hpp"
#include "../ngn/window.hpp"

#include <limits>
#include <string>
#include <cassert>
#include <iostream>
#include <algorithm>

namespace rn
{

using namespace std;

list<FBO *> FBO::collection{};
vector<FBO *> FBO::activeStack{};

rn::Mesh FBO::mesh{"FBO mesh"};
rn::Program FBO::prog{"FBO program"};

void FBO::reloadAll()
{
	for (FBO *fbo : FBO::collection)
	{
		try
		{
			fbo->reload();
		}
		catch (const string &e)
		{
			clog << endl << e << endl;
		}
	}
}

void FBO::reloadSoftAll()
{
	for (FBO *fbo : FBO::collection)
	{
		try
		{
			fbo->reloadSoft();
		}
		catch (const string &e)
		{
			clog << endl << e << endl;
		}
	}
}

void FBO::init()
{
	auto &&source = src::mem::mesh({
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
	});

	assert(source.get() != nullptr);

	source->arrays.emplace_back(GL_TRIANGLE_STRIP, 0, 4);
	source->layouts.emplace_back(0, 4, GL_FLOAT, 0, 0);

	reinterpret_cast<src::mem::Mesh *>(source.get())->setName("FBO quad");

	mesh.load(move(source));

	prog.load("rn/fbo.frag", "rn/fbo.vert");
}

FBO::FBO()
{
	FBO::collection.push_back(this);
}

FBO::FBO(string &&name)
	: FBO{}
{
	fboName = move(name);
}

FBO::~FBO()
{
	FBO::collection.remove(this);

	FBO::activeStack.erase(remove(begin(FBO::activeStack), end(FBO::activeStack), this), end(FBO::activeStack));
}

void FBO::setColorTex(GLint internalFormat, size_t i)
{
	if (colors.size() <= i)
	{
		colors.resize(i + 1);
	}

	if (internalFormat)
	{
		colors[i].enabled = true;
		colors[i].internalFormat = internalFormat;
	}
	else
	{
		colors[i].enabled = false;
	}
}

void FBO::setDepthTex(GLint internalFormat)
{
	resetDepthStorage();

	if (internalFormat)
	{
		depth.type = Texture;
		depth.internalFormat = internalFormat;
	}
	else
	{
		depth.type = None;
	}
}

void FBO::setDepthBuf(GLint internalFormat)
{
	resetDepthStorage();

	if (internalFormat)
	{
		depth.type = Renderbuffer;
		depth.internalFormat = internalFormat;
	}
	else
	{
		depth.type = None;
	}
}

void FBO::clone(const FBO &fbo)
{
	reset();

	colors = fbo.colors;

	for (auto &color : colors)
	{
		color.id = 0;
	}

	depth.internalFormat = fbo.depth.internalFormat;
	depth.type = fbo.depth.type;

	clearColor = fbo.clearColor;

	width = fbo.width;
	height = fbo.height;
}

void FBO::create()
{
	if ( ! width)
	{
		width = ngn::window::width;
	}

	if ( ! height)
	{
		height = ngn::window::height;
	}

	reload();
}

void FBO::reset()
{
	resetColorStorages();
	resetDepthStorage();

	if (id)
	{
		RN_CHECK(glDeleteFramebuffers(1, &id));
		id = 0;
	}
}

void FBO::resetColorStorages()
{
	for (auto &color : colors)
	{
		if (color.id)
		{
			RN_CHECK(glDeleteTextures(1, &color.id));
			color.id = 0;
		}
	}
}

void FBO::resetDepthStorage()
{
	if (depth.id)
	{
		switch (depth.type)
		{
			case Renderbuffer:
				RN_CHECK(glDeleteRenderbuffers(1, &depth.id));
			break;
			case Texture:
				RN_CHECK(glDeleteTextures(1, &depth.id));
			break;
			default:
			break;
		}

		depth.id = 0;
	}
}

void FBO::reload()
{
	double timer = ngn::time();

	reset();

	for (auto &color : colors)
	{
		if ( ! color.enabled)
		{
			continue;
		}

		RN_CHECK(glGenTextures(1, &color.id));

		RN_CHECK(glBindTexture(GL_TEXTURE_2D, color.id));

		RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

		RN_CHECK_PARAM(glTexStorage2D(GL_TEXTURE_2D, 1, color.internalFormat, width, height), rn::getEnumName(color.internalFormat));

		RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}

	switch (depth.type)
	{
		case Renderbuffer:
			RN_CHECK(glGenRenderbuffers(1, &depth.id));

			RN_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, depth.id));

			RN_CHECK_PARAM(glRenderbufferStorage(GL_RENDERBUFFER, depth.internalFormat, width, height), rn::getEnumName(depth.internalFormat));

			RN_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));
		break;
		case Texture:
			RN_CHECK(glGenTextures(1, &depth.id));

			RN_CHECK(glBindTexture(GL_TEXTURE_2D, depth.id));

			RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

			RN_CHECK_PARAM(glTexStorage2D(GL_TEXTURE_2D, 1, depth.internalFormat, width, height), rn::getEnumName(depth.internalFormat));

			RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		break;
		default:
		break;
	}

	reloadSoft();

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [FBO {" <<  fboName << "}:" << (ngn::time() - timer) << "s]" << endl;
		clog.unsetf(ios::floatfield);
		// for (size_t i = 0; i < colors.size(); i++)
		// {
		// 	clog << "    " << rn::getEnumName(GL_COLOR_ATTACHMENT0 + i) << endl;
		// }
	}
}

void FBO::reloadSoft()
{
	if ( ! colors.size() && depth.type == None)
	{
		return;
	}

	RN_CHECK(glGenFramebuffers(1, &id));
	RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));

	if ( ! colors.empty())
	{
		size_t enabledColors = count_if(begin(colors), end(colors), [] (const TexContainer &color)
		{
			return color.enabled;
		});

		size_t drawBuffersSize = util::nextPowerOf2(enabledColors);

		vector<GLenum> drawBuffers{};
		drawBuffers.reserve(drawBuffersSize);

		for (size_t i = 0; i < colors.size(); i++)
		{
			if ( ! colors[i].enabled)
			{
				continue;
			}

			RN_CHECK(glBindTexture(GL_TEXTURE_2D, colors[i].id));

			RN_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colors[i].id, 0));

			drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
		}

		// fill rest of the draw buffers list
		drawBuffers.resize(drawBuffersSize, GL_NONE);

		RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

		if ( ! drawBuffers.empty())
		{
			RN_CHECK(glDrawBuffers(drawBuffers.size(), drawBuffers.data()));
		}
		else
		{
			RN_CHECK(glDrawBuffer(GL_NONE));
		}
	}
	else
	{
		RN_CHECK(glDrawBuffer(GL_NONE));
	}

	GLenum attachment;

	switch (depth.internalFormat)
	{
		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32F:
			attachment = GL_DEPTH_ATTACHMENT;
		break;

		case GL_DEPTH24_STENCIL8:
		case GL_DEPTH32F_STENCIL8:
			attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		break;

		default:
			attachment = GL_NONE;
	}

	if (attachment)
	{
		switch (depth.type)
		{
			case Renderbuffer:
				RN_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, depth.id));
			break;
			case Texture:
				RN_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, depth.id, 0));
			break;
			default:
			break;
		}
	}

	GLenum status = GL_NONE;
	RN_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw string{"rn::FBO{" + fboName + "}::reloadSoft() - failed to verify framebuffer (" + rn::getEnumName(status) + ")\n  colors: " + to_string(colors.size()) + "\n  depth: " + (depth.type == Renderbuffer ? "renderbuffer" : "texture") + " (" + rn::getEnumName(attachment) + ")"};
	}

	RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

GLsizei FBO::bindColorTex(GLsizei unit, size_t i)
{
	RN_CHECK(glActiveTexture(GL_TEXTURE0 + unit));

	if (i < colors.size() && colors[i].enabled)
	{
		RN_CHECK(glBindTexture(GL_TEXTURE_2D, colors[i].id));
	}
	else
	{
		RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}

	return unit;
}

GLsizei FBO::bindDepthTex(GLsizei unit)
{
	RN_CHECK(glActiveTexture(GL_TEXTURE0 + unit));

	if (depth.type == Texture)
	{
		RN_CHECK(glBindTexture(GL_TEXTURE_2D, depth.id));
	}
	else
	{
		RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}

	return unit;
}

void FBO::blit(GLuint target, GLbitfield mask, GLint filter, GLsizei targetWidth, GLsizei targetHeight)
{
	if ( ! targetWidth) {
		targetWidth = width;
	}

	if ( ! targetHeight) {
		targetHeight = height;
	}

	RN_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, id));
	RN_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target));
	RN_CHECK(glBlitFramebuffer(0, 0, width, height, 0, 0, targetWidth, targetHeight, mask, filter));
	RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FBO::blit(FBO &fbo, GLbitfield mask, GLint filter)
{
	RN_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, id));
	RN_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo.id));
	RN_CHECK(glBlitFramebuffer(0, 0, width, height, 0, 0, fbo.width, fbo.height, mask, filter));
	RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FBO::clear()
{
	GLbitfield mask = GL_NONE;

	if ( ! colors.empty())
	{
		RN_CHECK(glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a));
		mask |= GL_COLOR_BUFFER_BIT;
	}

	if (depth.type != None)
	{
		mask |= GL_DEPTH_BUFFER_BIT;

		if (depth.internalFormat == GL_DEPTH24_STENCIL8 || depth.internalFormat == GL_DEPTH32F_STENCIL8)
		{
			RN_CHECK(glClearStencil(0));
			mask |= GL_STENCIL_BUFFER_BIT;
		}
	}

	RN_CHECK(glClear(mask));
}

void FBO::use()
{
	RN_CHECK(glViewport(0, 0, width, height));
	RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));

	FBO::activeStack.push_back(this);
}

void FBO::forgo()
{
	FBO::activeStack.pop_back();

	if ( ! FBO::activeStack.empty())
	{
		FBO *fbo = FBO::activeStack.back();
		RN_CHECK(glViewport(0, 0, fbo->width, fbo->height));
		RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo->id));
	}
	else
	{
		RN_CHECK(glViewport(0, 0, ngn::window::width, ngn::window::height));
		RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
}

namespace
{
	const util::InitQAttacher attach(rn::initQ(), []
	{
		if ( ! rn::ext::ARB_texture_storage) {
			throw string{"rn::FBO initQ - rn::FBO requires GL_ARB_texture_storage"};
		}

		rn::FBO::init();
	});
}

}