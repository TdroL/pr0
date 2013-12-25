#include "fbo.hpp"

#include "../gl.hpp"
#include "../src/mem.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "../sys.hpp"
#include "../sys/window.hpp"

#include <string>
#include <iostream>

namespace gl
{

using namespace std;

namespace {
	gl::Mesh mesh{"FBO mesh"};
	gl::Program prog{"FBO program"};
}

list<FBO *> FBO::collection{};

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

void FBO::create(unsigned int colorAttachments, bool depthAttachment, bool stencilAttachment)
{
	if ( ! width)
	{
		width = sys::window::width;
	}

	if ( ! height)
	{
		height = sys::window::height;
	}

	this->colorAttachments = colorAttachments;
	this->depthAttachment = depthAttachment;
	this->stencilAttachment = stencilAttachment;

	reload();
}

void FBO::reset()
{
	if (id)
	{
		GL_CHECK(glDeleteFramebuffers(1, &id));
		id = 0;
	}

	if (renderbuffer)
	{
		GL_CHECK(glDeleteRenderbuffers(1, &renderbuffer));
		renderbuffer = 0;
	}

	if (colors.size())
	{
		GL_CHECK(glDeleteTextures(colors.size(), colors.data()));

		decltype(colors) empty{};
		colors.swap(empty);
	}
}

void FBO::reload()
{
	double timer = sys::time();

	reset();

	decltype(colors) empty(colorAttachments, 0);
	colors.swap(empty);

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

		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}

	GLenum component = GL_NONE;
	GLenum attachment = GL_NONE;

	if (depthAttachment)
	{
		if (stencilAttachment)
		{
			component = GL_DEPTH24_STENCIL8;
			attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		}
		else
		{
			component = GL_DEPTH_COMPONENT;
			attachment = GL_DEPTH_ATTACHMENT;
		}
	}
	else if (stencilAttachment)
	{
		component = GL_STENCIL_INDEX8;
		attachment = GL_STENCIL_ATTACHMENT;
	}

	if (component != GL_NONE)
	{
		GL_CHECK(glGenRenderbuffers(1, &renderbuffer));
		GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer));
		GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, component, width, height));
	}

	reloadSoft();

	clog << fixed;
	clog << "  [FBO: " <<  fboName << ":" << sys::time() - timer << "s]" << endl;
	for (size_t i = 0; i < colors.size(); i++)
	{
		clog << "    " << gl::getEnumName(GL_COLOR_ATTACHMENT0 + i) << endl;
	}

	if (attachment != GL_NONE)
	{
		clog << "    " << gl::getEnumName(attachment) << endl;
	}

	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FBO::reloadSoft()
{
	GL_CHECK(glGenFramebuffers(1, &id));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));

	vector<GLenum> drawBuffers;
	drawBuffers.reserve(colors.size());

	for (size_t i = 0; i < colors.size(); i++)
	{
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, colors[i]));

		GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colors[i], 0));

		drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}

	GLenum attachment = GL_NONE;

	if (depthAttachment)
	{
		if (stencilAttachment)
		{
			attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		}
		else
		{
			attachment = GL_DEPTH_ATTACHMENT;
		}
	}
	else if (stencilAttachment)
	{
		attachment = GL_STENCIL_ATTACHMENT;
	}

	if (attachment != GL_NONE)
	{
		GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer));
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
	prog.use();

	for (GLint i = 0, c = colors.size(); i < c; i++)
	{
		prog.var("tex" + to_string(i), static_cast<GLint>(i));
		GL_CHECK(glActiveTexture(GL_TEXTURE0 + i));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, colors[i]));
	}

	mesh.render();
}

void FBO::clear()
{
	GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
	GL_CHECK(glClearDepth(1.0f));
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

void FBO::use()
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));
}

void FBO::release()
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

namespace
{
	const util::InitQAttacher attach{gl::initQ, []
	{
		gl::FBO::init();
	}};
}

}