#include "fbo.hpp"

#include "../gl.hpp"
#include "../util.hpp"
#include "../util/align.hpp"
#include "../util/str.hpp"
#include "../src/mem.hpp"
#include "../ngn.hpp"
#include "../ngn/window.hpp"

#include <limits>
#include <string>
#include <cassert>
#include <iostream>
#include <algorithm>

namespace gl
{

using namespace std;

list<FBO *> FBO::collection{};
vector<FBO *> FBO::activeStack{};

gl::Mesh FBO::mesh{"FBO mesh"};
gl::Program FBO::prog{"FBO program"};

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

	prog.load("gl/fbo.frag", "gl/fbo.vert");
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

void FBO::setTex(size_t id, gl::Tex2D &&tex)
{
	if (colors.size() <= id)
	{
		colors.resize(id + 1);
	}

	colors[id].enabled = true;
	colors[id].tex = std::move(tex);
}

void FBO::setDepth(gl::Tex2D &&tex)
{
	depth.type = Texture;
	depth.buf.reset();
	depth.tex = std::move(tex);
}

void FBO::setDepth(gl::Renderbuffer &&buf)
{
	depth.type = Renderbuffer;
	depth.tex.reset();
	depth.buf = std::move(buf);
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
	resetStorages();

	colors.clear();

	depth.type = Renderbuffer;
}

void FBO::resetStorages()
{
	if (id)
	{
		GL_CHECK(glDeleteFramebuffers(1, &id));
		id = 0;
	}

	for (auto &color : colors)
	{
		if (color.enabled)
		{
			color.tex.reset();
		}
	}

	if (depth.type == Renderbuffer)
	{
		depth.buf.reset();
	}
	else // Texture
	{
		depth.tex.reset();
	}
}

void FBO::reload()
{
	double timer = ngn::time();

	resetStorages();

	for (auto &color : colors)
	{
		if ( ! color.enabled)
		{
			continue;
		}

		color.tex.create(width, height, GL_LINEAR, GL_CLAMP_TO_EDGE);
	}

	if (depth.type == Renderbuffer)
	{
		depth.buf.create(width, height);
	}
	else if (depth.type == Texture)
	{
		depth.tex.create(width, height, GL_NEAREST, GL_CLAMP_TO_EDGE);
	}

	reloadSoft();

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [FBO {" <<  fboName << "}:" << ngn::time() - timer << "s]" << endl;
		// for (size_t i = 0; i < colors.size(); i++)
		// {
		// 	clog << "    " << gl::getEnumName(GL_COLOR_ATTACHMENT0 + i) << endl;
		// }
	}
}

void FBO::reloadSoft()
{
	GL_CHECK(glGenFramebuffers(1, &id));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));

	if ( ! colors.empty())
	{
		const size_t enabledColors = count_if(begin(colors), end(colors), [] (const TexContainer &container)
		{
			return container.enabled;
		});

		std::vector<GLenum> drawBuffers(util::nextPowerOf2(enabledColors), GL_NONE);

		for (size_t i = 0, j = 0; i < colors.size(); i++)
		{
			if ( ! colors[i].enabled)
			{
				continue;
			}

			GL_CHECK(glBindTexture(GL_TEXTURE_2D, colors[i].tex.id));

			GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colors[i].tex.id, 0));

			drawBuffers[j] = GL_COLOR_ATTACHMENT0 + i;
			j++;
		}

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

		if ( ! drawBuffers.empty())
		{
			GL_CHECK(glDrawBuffers(drawBuffers.size(), drawBuffers.data()));
		}
		else
		{
			GL_CHECK(glDrawBuffer(GL_NONE));
		}
	}
	else
	{
		GL_CHECK(glDrawBuffer(GL_NONE));
	}

	if (depth.type == Renderbuffer)
	{
		GLenum attachment = depth.buf.getAttachmentType();

		GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, depth.buf.id));
	}
	else // Texture
	{
		GLenum attachment = depth.tex.getAttachmentType();

		GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, depth.tex.id, 0));
	}

	GLenum status = GL_NONE;
	GL_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw string{"gl::FBO{" + fboName + "}::reloadSoft() - failed to verify framebuffer (" + gl::getEnumName(status) + ")\n  colors: " + to_string(colors.size()) + "\n  depth: " + (depth.type == Renderbuffer ? "renderbuffer" : "texture") + " (" + gl::getEnumName(depth.getAttachmentType()) + ")"};
	}

	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FBO::blit(GLuint target, GLbitfield mask)
{
	GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, id));
	GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target));
	GL_CHECK(glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, mask, GL_NEAREST));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FBO::clear()
{
	GLbitfield mask = GL_NONE;

	if ( ! colors.empty())
	{
		GL_CHECK(glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a));

		mask |= GL_COLOR_BUFFER_BIT;
	}

	mask |= GL_DEPTH_BUFFER_BIT;

	if (depth.getAttachmentType() == GL_DEPTH_STENCIL_ATTACHMENT)
	{
		GL_CHECK(glClearStencil(0));

		mask |= GL_STENCIL_BUFFER_BIT;
	}

	GL_CHECK(glClear(mask));
}

void FBO::use()
{
	GL_CHECK(glViewport(0, 0, width, height));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));
	FBO::activeStack.push_back(this);
}

void FBO::release()
{
	FBO::activeStack.pop_back();

	if ( ! FBO::activeStack.empty())
	{
		FBO *fbo = FBO::activeStack.back();
		GL_CHECK(glViewport(0, 0, fbo->width, fbo->height));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo->id));
	}
	else
	{
		GL_CHECK(glViewport(0, 0, ngn::window::width, ngn::window::height));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
}

namespace
{
	const util::InitQAttacher attach{gl::initQ, []
	{
		gl::FBO::init();
	}};
}

}