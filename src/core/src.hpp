#ifndef SRC_HPP
#define SRC_HPP

#include "rn.hpp"
#include "rn/types.hpp"
#include "util.hpp"

#include <vector>
#include <memory>
#include <string>

namespace src
{

class Mesh
{
public:
	rn::BufferData vertexData{};
	rn::BufferData indexData{};
	std::vector<rn::VertexLayout> layouts{};
	std::vector<rn::DrawIndex> indices{};
	std::vector<rn::DrawArray> arrays{};

	virtual void open() {}
	virtual void close() {}
	virtual std::string name() { return "Unknown"; }
	virtual ~Mesh() {}
};

class MeshScoper
{
public:
	Mesh &mesh;

	MeshScoper(Mesh &mesh) : mesh(mesh) { this->mesh.open(); }
	MeshScoper(std::unique_ptr<Mesh> &mesh) : mesh(*mesh) { this->mesh.open(); }
	MeshScoper(Mesh *mesh) : mesh(*mesh) { this->mesh.open(); }
	~MeshScoper() { mesh.close(); }
};

#define SRC_MESH_OPEN(meshSource) src::MeshScoper UTIL_CONCAT2(meshScoper, __COUNTER__)(meshSource)

class Tex2D
{
public:
	GLsizei width = 0;
	GLsizei height = 0;

	std::unique_ptr<GLubyte[]> data{nullptr};
	std::size_t size = 0;

	GLint levels = 1;

	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;

	GLint alignment = 1;

	virtual void open() {}
	virtual void close() {}
	virtual std::string name() { return "Unknown"; }
	virtual ~Tex2D() {}
};

class Tex2DScoper
{
public:
	Tex2D &tex2d;

	Tex2DScoper(Tex2D &tex2d) : tex2d(tex2d) { this->tex2d.open(); }
	Tex2DScoper(std::unique_ptr<Tex2D> &tex2d) : tex2d(*tex2d) { this->tex2d.open(); }
	Tex2DScoper(Tex2D *tex2d) : tex2d(*tex2d) { this->tex2d.open(); }
	~Tex2DScoper() { tex2d.close(); }
};

#define SRC_TEX2D_OPEN(tex2dSource) src::Tex2DScoper UTIL_CONCAT2(tex2dScoper, __COUNTER__)(tex2dSource)

class Stream
{
public:
	std::vector<char> contents{};

	virtual void open() {}
	virtual void close() {}
	virtual std::string name() { return "Unknown"; }
	virtual ~Stream() {}

};

class StreamScoper
{
public:
	Stream &stream;

	StreamScoper(Stream &stream) : stream(stream) { this->stream.open(); }
	StreamScoper(std::unique_ptr<Stream> &stream) : stream(*stream) { this->stream.open(); }
	StreamScoper(Stream *stream) : stream(*stream) { this->stream.open(); }
	~StreamScoper() { stream.close(); }
};

#define SRC_STREAM_OPEN(streamSource) src::StreamScoper UTIL_CONCAT2(streamScoper, __COUNTER__)(streamSource)

} // src

#endif