#include "fbo.hpp"

#include "../gl.hpp"
#include "../util.hpp"
#include "../util/align.hpp"
#include "../util/str.hpp"
#include "../src/mem.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "../ngn.hpp"
#include "../ngn/window.hpp"

#include <string>
#include <cassert>
#include <iostream>
#include <algorithm>

namespace gl
{

using namespace std;

namespace {
	gl::Mesh mesh{"FBO mesh"};
	gl::Program prog{"FBO program"};
}

list<FBO *> FBO::collection{};
vector<GLuint> FBO::activeStack{};

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
}

void FBO::setTexParams(size_t id, GLint internalFormat, GLenum format, GLenum type)
{
	if (texParams.size() <= id)
	{
		texParams.resize(id + 1);
	}

	texParams[id].internalFormat = internalFormat;
	texParams[id].format = format;
	texParams[id].type = type;
}

void FBO::setDSParams(GLint internalFormat, GLenum format, GLenum type)
{
	depthStencilParams.internalFormat = internalFormat;
	depthStencilParams.format = format;
	depthStencilParams.type = type;
}

void FBO::create(unsigned int colorAttachments, DSType depthStencilType)
{
	if ( ! width)
	{
		width = ngn::window::width;
	}

	if ( ! height)
	{
		height = ngn::window::height;
	}

	this->colorAttachments = colorAttachments;
	this->depthStencilType = depthStencilType;

	reload();
}

void FBO::create(vector<string> &&colorNames, DSType depthStencilType)
{
	this->colorNames = move(colorNames);

	for_each(begin(this->colorNames), end(this->colorNames), util::str::trim);

	create(this->colorNames.size(), depthStencilType);
}

void FBO::reset()
{
	resetStorages();

	colorAttachments = 0;
	depthStencilType = DSType::None;

	colors.clear();
	colorNames.clear();
	texParams.clear();
}

void FBO::resetStorages()
{
	if (id)
	{
		GL_CHECK(glDeleteFramebuffers(1, &id));
		id = 0;
	}

	if (depthStencilType == DSType::Buffer)
	{
		GL_CHECK(glDeleteRenderbuffers(1, &depthStencil));
		depthStencil = 0;
	}
	else if (depthStencilType == DSType::Texture)
	{
		GL_CHECK(glDeleteTextures(1, &depthStencil));
		depthStencil = 0;
	}

	if (colors.size())
	{
		GL_CHECK(glDeleteTextures(colors.size(), colors.data()));

		fill(begin(colors), end(colors), 0);
	}
}

void FBO::reload()
{
	double timer = ngn::time();

	resetStorages();

	if (colorAttachments)
	{
		colors.clear();
		colors.resize(colorAttachments, 0);

		GL_CHECK(glGenTextures(colorAttachments, colors.data()));

		for (GLuint i = 0; i < colorAttachments; i++)
		{
			GLuint color = colors[i];

			GL_CHECK(glBindTexture(GL_TEXTURE_2D, color));

			GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GL_CHECK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4{1.f})));

			if (texParams.size() > i)
			{
				GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, texParams[i].internalFormat, width, height, 0, texParams[i].format, texParams[i].type, nullptr));
			}
			else
			{
				GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
			}

			GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		}
	}

	if (depthStencilType == DSType::Buffer)
	{
		GL_CHECK(glGenRenderbuffers(1, &depthStencil));
		GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, depthStencil));
		GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, depthStencilParams.internalFormat, width, height));
	}
	else if (depthStencilType == DSType::Texture)
	{
		GL_CHECK(glGenTextures(1, &depthStencil));

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, depthStencil));

		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GL_CHECK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4{1.f, 1.f, 1.f, 0.f})));

		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, depthStencilParams.internalFormat, width, height, 0, depthStencilParams.format, depthStencilParams.type, nullptr));

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
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

	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FBO::reloadSoft()
{
	if ( ! (colorAttachments > 0 || depthStencilType != DSType::None))
	{
		return;
	}

	GL_CHECK(glGenFramebuffers(1, &id));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));

	std::vector<GLenum> drawBuffers;
	drawBuffers.resize(util::nextPowerOf2(colors.size()), GL_NONE);

	for (size_t i = 0; i < colors.size(); i++)
	{
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, colors[i]));

		GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colors[i], 0));

		drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}

	GL_CHECK(glDrawBuffers(drawBuffers.size(), drawBuffers.data()));

	if (depthStencilType == DSType::Buffer)
	{
		GLenum attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		if (depthStencilParams.format == GL_DEPTH_COMPONENT)
		{
			attachment = GL_DEPTH_ATTACHMENT;
		}

		GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, depthStencil));
	}
	else if (depthStencilType == DSType::Texture)
	{
		GLenum attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		if (depthStencilParams.format == GL_DEPTH_COMPONENT)
		{
			attachment = GL_DEPTH_ATTACHMENT;
		}

		GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, depthStencil, 0));
	}

	GLenum status = GL_NONE;
	GL_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw string{"gl::FBO{" + fboName + "}::reloadSoft() - failed to verify framebuffer (" + gl::getEnumName(status) + ")"};
	}

	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FBO::render()
{
	render(prog);
}

void FBO::render(gl::Program &prog)
{
	prog.use();

	for (size_t i = 0; i < colors.size(); i++)
	{
		if (colorNames.size() > i && ! colorNames[i].empty())
		{
			prog.var(colorNames[i], static_cast<GLint>(i));
		}
		else
		{
			prog.var("tex" + to_string(i), static_cast<GLint>(i));
		}

		GL_CHECK(glActiveTexture(GL_TEXTURE0 + i));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, colors[i]));
	}

	if (depthStencilType == DSType::Texture)
	{
		prog.var("texDS", static_cast<GLint>(colors.size()));
		GL_CHECK(glActiveTexture(GL_TEXTURE0 + colors.size()));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, depthStencil));
	}

	mesh.render();
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

	if (colorAttachments)
	{
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

		mask |= GL_COLOR_BUFFER_BIT;
	}

	if (depthStencilType != DSType::None)
	{
		GL_CHECK(glClearDepth(1.0f));

		mask |= GL_DEPTH_BUFFER_BIT;

		if (depthStencilParams.format == GL_DEPTH_STENCIL)
		{
			GL_CHECK(glClearStencil(0));

			mask |= GL_STENCIL_BUFFER_BIT;
		}
	}

	GL_CHECK(glClear(mask));
}

void FBO::use()
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));
	FBO::activeStack.push_back(id);
}

void FBO::release()
{
	FBO::activeStack.pop_back();

	if ( ! FBO::activeStack.empty())
	{
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, FBO::activeStack.back()));
	}
	else
	{
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