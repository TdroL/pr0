#include <pch.hpp>

#include "fb.hpp"

#include "../ngn.hpp"
#include "../util.hpp"
#include "../ngn/window.hpp"
#include "../util/align.hpp"

#include <algorithm>

namespace rn
{

using namespace std;

vector<FB *> FB::collection{};
vector<FB *> FB::activeStack{};

void FB::reloadAll()
{
	for (FB *fb : FB::collection)
	{
		try
		{
			fb->reload();
		}
		catch (const string &e)
		{
			clog << endl << e << endl;
		}
	}
}

void FB::reloadSoftAll()
{
	for (FB *fb : FB::collection)
	{
		try
		{
			fb->reloadSoft();
		}
		catch (const string &e)
		{
			clog << endl << e << endl;
		}
	}
}

FB::FB(size_t colorsSize)
{
	FB::collection.push_back(this);

	colorContainers.resize(colorsSize);
}

FB::FB(string &&name, size_t colorsSize)
	: FB{colorsSize}
{
	fbName = move(name);
}

FB::~FB()
{
	// FB::collection.remove(this);

	FB::collection.erase(remove(begin(FB::collection), end(FB::collection), this), end(FB::collection));
	FB::activeStack.erase(remove(begin(FB::activeStack), end(FB::activeStack), this), end(FB::activeStack));
}

void FB::attachColor(size_t index, const shared_ptr<rn::Tex2D> &tex, GLint level)
{
	if (colorContainers.size() <= index)
	{
		colorContainers.resize(index + 1);
	}

	colorContainers[index].tex = static_pointer_cast<rn::Tex>(tex);
	colorContainers[index].level = level;

	if (tex)
	{
		width = tex->width;
		height = tex->height;
	}
}

void FB::attachColor(size_t index, const shared_ptr<rn::Tex2DArray> &tex, GLsizei layer, GLint level)
{
	if (colorContainers.size() <= index)
	{
		colorContainers.resize(index + 1);
	}

	colorContainers[index].tex = static_pointer_cast<rn::Tex>(tex);
	colorContainers[index].layer = layer;
	colorContainers[index].level = level;

	if (tex)
	{
		width = tex->width;
		height = tex->height;
	}
}

void FB::attachDepth(const shared_ptr<rn::Tex2D> &tex, GLint level)
{
	depthContainer.tex = static_pointer_cast<rn::Tex>(tex);
	depthContainer.level = level;

	if (tex)
	{
		width = tex->width;
		height = tex->height;
	}
}

void FB::attachDepth(const shared_ptr<rn::Tex2DArray> &tex, GLsizei layer, GLint level)
{
	depthContainer.tex = static_pointer_cast<rn::Tex>(tex);
	depthContainer.layer = layer;
	depthContainer.level = level;

	if (tex)
	{
		width = tex->width;
		height = tex->height;
	}
}

FB::TexContainer FB::detachColor(size_t index)
{
	FB::TexContainer tmpContainer;

	if (colorContainers.size() < index && colorContainers[index].tex)
	{
		tmpContainer = colorContainers[index];
		colorContainers[index] = {};
	}

	return tmpContainer;
}

FB::TexContainer FB::detachDepth()
{
	FB::TexContainer tmpContainer;

	if (depthContainer.tex)
	{
		tmpContainer = depthContainer;
		depthContainer = TexContainer{};
	}

	return tmpContainer;
}

rn::Tex * FB::color(size_t index)
{
	UTIL_DEBUG
	{
		if (colorContainers.size() < index || ! colorContainers[index].tex)
		{
			throw string{"rn::FB{" + fbName + "}::color(" + to_string(index) + ") - no color texture"};
		}
	}

	if (colorContainers.size() < index)
	{
		return nullptr;
	}

	return colorContainers[index].tex.get();
}

rn::Tex * FB::depth()
{
	UTIL_DEBUG
	{
		if ( ! depthContainer.tex)
		{
			throw string{"rn::FB{" + fbName + "}::depth() - no depth texture"};
		}
	}

	return depthContainer.tex.get();
}

shared_ptr<rn::Tex> FB::shareColor(size_t index)
{
	UTIL_DEBUG
	{
		if (colorContainers.size() < index || ! colorContainers[index].tex)
		{
			throw string{"rn::FB{" + fbName + "}::shareColor(" + to_string(index) + ") - no color texture"};
		}
	}

	if (colorContainers.size() < index)
	{
		return shared_ptr<rn::Tex>(nullptr);
	}

	return colorContainers[index].tex;
}

shared_ptr<rn::Tex> FB::shareDepth()
{
	UTIL_DEBUG
	{
		if ( ! depthContainer.tex)
		{
			throw string{"rn::FB{" + fbName + "}::shareDepth() - no depth texture"};
		}
	}

	return depthContainer.tex;
}

void FB::clear(BuffersMask mask)
{
	if (mask & BUFFER_COLOR)
	{
		RN_CHECK(glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a));
	}

	if (mask & BUFFER_DEPTH)
	{
		RN_CHECK(glClearDepth(clearDepth));
	}

	RN_CHECK(glClear(mask));

	if (mask & BUFFER_DEPTH)
	{
		RN_CHECK(glClearDepth(1.f));
	}

	if (mask & BUFFER_COLOR)
	{
		RN_CHECK(glClearColor(0.f, 0.f, 0.f, 0.f));
	}
}

void FB::blit(FB &target, BuffersMask mask, MagFilter filter)
{
	RN_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, id));
	RN_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.id));
	RN_CHECK(glBlitFramebuffer(0, 0, width, height, 0, 0, target.width, target.height, mask, filter));
	RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FB::blit(FB *target, BuffersMask mask, MagFilter filter)
{
	GLuint targetId;
	GLsizei targetWidth;
	GLsizei targetHeight;

	if (target != nullptr)
	{
		targetId = target->id;
		targetWidth = target->width;
		targetHeight = target->height;
	}
	else
	{
		targetId = 0;
		targetWidth = ngn::window::width;
		targetHeight = ngn::window::height;
	}

	RN_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, id));
	RN_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetId));
	RN_CHECK(glBlitFramebuffer(0, 0, width, height, 0, 0, targetWidth, targetHeight, mask, filter));
	RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FB::reload()
{
	reloadSoft();
}

void FB::reloadSoft()
{
	double timer = ngn::time();

	size_t enabledColors = count_if(begin(colorContainers), end(colorContainers), [] (const TexContainer &container)
	{
		return container.tex && container.tex->id;
	});

	if ( ! enabledColors && ( ! depthContainer.tex || ! depthContainer.tex->id))
	{
		return;
	}

	RN_CHECK(glGenFramebuffers(1, &id));
	RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));

	vector<GLenum> drawBuffers{};

	if (enabledColors)
	{
		size_t drawBuffersSize = util::nextPowerOf2(enabledColors);

		drawBuffers.reserve(drawBuffersSize);

		for (size_t i = 0; i < colorContainers.size(); i++)
		{
			if ( ! colorContainers[i].tex || ! colorContainers[i].tex->id)
			{
				continue;
			}

			// RN_CHECK(glBindTexture(colorContainers[i].tex->targetType(), colorContainers[i].tex->id));

			switch (colorContainers[i].tex->targetType()) {
				case GL_TEXTURE_2D:
					RN_CHECK_PARAM(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colorContainers[i].tex->id, colorContainers[i].level), colorContainers[i].level);
				break;
				case GL_TEXTURE_2D_ARRAY:
					RN_CHECK_PARAM(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colorContainers[i].tex->id, colorContainers[i].level, colorContainers[i].layer), colorContainers[i].level << ", " << colorContainers[i].layer);
				break;
			}

			drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);

			// RN_CHECK(glBindTexture(colorContainers[i].tex->targetType(), 0));
		}

		// fill rest of the draw buffers list
		drawBuffers.resize(drawBuffersSize, GL_NONE);
	}

	if ( ! drawBuffers.empty())
	{
		RN_CHECK_PARAM(glDrawBuffers(drawBuffers.size(), drawBuffers.data()), drawBuffers.size());
	}
	else
	{
		RN_CHECK(glDrawBuffer(GL_NONE));
	}

	if (depthContainer.tex && depthContainer.tex->id && depthContainer.tex->isDepth())
	{
		GLenum attachment = depthContainer.tex->isDepthStencil() ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;

		// RN_CHECK(glBindTexture(depthContainer.tex->targetType(), depthContainer.tex->id));

		switch (depthContainer.tex->targetType()) {
			case GL_TEXTURE_2D:
				RN_CHECK_PARAM(glFramebufferTexture(GL_FRAMEBUFFER, attachment, depthContainer.tex->id, depthContainer.level), depthContainer.level);
			break;
			case GL_TEXTURE_2D_ARRAY:
				RN_CHECK_PARAM(glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, depthContainer.tex->id, depthContainer.level, depthContainer.layer), depthContainer.level << ", " << depthContainer.layer);
			break;
		}

		// RN_CHECK(glBindTexture(depthContainer.tex->targetType(), 0));
	}

	GLenum status = GL_NONE;
	RN_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw string{
			"rn::FB{" + fbName + "}::reloadSoft() - failed to verify framebuffer (" + rn::getEnumName(status) + ")" +
			"\n  colors: " + to_string(enabledColors) + "/" + to_string(colorContainers.size()) +
			"\n  depth: " + (depthContainer.tex ? string{"texture "} + (depthContainer.tex->isDepthStencil() ? "depth, stencil" : "depth") : "none")
		};
	}

	RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [FB \"" << fbName << "\" {" << enabledColors << "/" << colorContainers.size() << ", " << (depthContainer.tex ? string{"texture "} + (depthContainer.tex->isDepthStencil() ? "depth, stencil" : "depth") : "none") << "}:" << (ngn::time() - timer) << "s]" << endl;
		clog.unsetf(ios::floatfield);
	}
}

void FB::bind()
{
	RN_CHECK(glViewport(0, 0, width, height));
	RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));

	FB::activeStack.push_back(this);
}

void FB::unbind()
{
	FB::activeStack.pop_back();

	if ( ! FB::activeStack.empty())
	{
		FB *fb = FB::activeStack.back();
		RN_CHECK(glViewport(0, 0, fb->width, fb->height));
		RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fb->id));
	}
	else
	{
		RN_CHECK(glViewport(0, 0, ngn::window::width, ngn::window::height));
		RN_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
}

} // rn