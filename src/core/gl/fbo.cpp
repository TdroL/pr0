#include "fbo.hpp"

#include "../gl.hpp"
#include "../src/mem.hpp"
#include "mesh.hpp"
#include "program.hpp"
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
	}

	if (depth)
	{
		GL_CHECK(glDeleteRenderbuffers(1, &depth));
	}

	if (stencil)
	{
		GL_CHECK(glDeleteTextures(1, &stencil));
	}

	if (colors.size())
	{
		GL_CHECK(glDeleteTextures(colors.size(), colors.data()));
	}

	id = depth = stencil = 0;

	colors.clear();
	colors.shrink_to_fit();
}

void FBO::reload()
{
	reset();

	GL_CHECK(glGenFramebuffers(1, &id));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));

	colors.clear();
	colors.resize(colorAttachments, 0);
	colors.shrink_to_fit();

	vector<GLenum> drawBuffers;
	drawBuffers.reserve(colorAttachments);

	GL_CHECK(glGenTextures(colorAttachments, colors.data()));

	for (GLuint i = 0; i < colorAttachments; i++)
	{
		GLuint color = colors[i];

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, color));

		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

		GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color, 0));

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

		drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

	if (depthAttachment)
	{
		GLenum component = GL_DEPTH_COMPONENT;
		GLenum attachment = GL_DEPTH_ATTACHMENT;

		if (stencilAttachment)
		{
			component = GL_DEPTH24_STENCIL8;
			attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		}

		GL_CHECK(glGenRenderbuffers(1, &depth));
		GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, depth));
		GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, component, width, height));
		GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, depth));
	}

	GLenum status = GL_NONE;
	GL_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw string{"gl::FBO{" + fboName + "}::init() - failed to verify framebuffer (" + gl::getEnumName(status) + ")"};
	}

	for (size_t i = 0; i < drawBuffers.size(); i++)
	{
		cout << gl::getEnumName(*(drawBuffers.data() + i)) << endl;
	}

	GL_CHECK(glDrawBuffers(drawBuffers.size(), drawBuffers.data()));

	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FBO::render()
{
	prog.use();

	// GL_CHECK(glDisable(GL_DEPTH_TEST));
	// GL_CHECK(glDisable(GL_CULL_FACE));

	for (GLint i = 0, c = colors.size(); i < c; i++)
	{
		prog.var("tex" + to_string(i), static_cast<GLint>(i));
		GL_CHECK(glActiveTexture(GL_TEXTURE0 + i));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, colors[i]));
	}

	mesh.render();

	// GL_CHECK(glEnable(GL_DEPTH_TEST));
	// GL_CHECK(glEnable(GL_CULL_FACE));

	// GL_CHECK(glBindVertexArray(vaoQuad));
	// GL_CHECK(glDisable(GL_DEPTH_TEST));
	// GL_CHECK(glUseProgram(screenShaderProgram));

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, texColorBuffer);

	// glDrawArrays(GL_TRIANGLES, 0, 6);
}

void FBO::use()
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));
}

void FBO::release()
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

}