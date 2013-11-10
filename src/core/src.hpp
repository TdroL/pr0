#ifndef SRC_HPP
#define SRC_HPP

#include <vector>
#include <memory>
#include <GL/glew.h>
#include "util.hpp"
#include "gl/types.hpp"

namespace src
{

class Mesh
{
public:
	gl::BufferData vertexData{};
	gl::BufferData indexData{};
	std::vector<gl::VertexLayout> layouts{};
	std::vector<gl::DrawIndex> indices{};
	std::vector<gl::DrawArray> arrays{};

	virtual void use() {}
	virtual void release() {}

	virtual ~Mesh() {}
};

class MeshScoper
{
public:
	Mesh &mesh;

	MeshScoper(Mesh &mesh) : mesh(mesh) { mesh.use(); }
	~MeshScoper() { mesh.release(); }
};

#define SRC_MESH_USE(meshSource) src::MeshScoper UTIL_CONCAT2(meshScoper, __COUNTER__){meshSource}

class Stream
{
public:
	std::vector<char> contents{};

	virtual void use() {}
	virtual void release() {}
	virtual std::string name() { return "Unknown"; };
	virtual ~Stream() {}

};

class StreamScoper
{
public:
	Stream &stream;

	StreamScoper(Stream &stream) : stream(stream) { stream.use(); }
	~StreamScoper() { stream.release(); }
};

#define SRC_STREAM_USE(streamSource) src::StreamScoper UTIL_CONCAT2(streamScoper, __COUNTER__){streamSource}

} // src

#endif