#ifndef GL_UBO_HPP
#define GL_UBO_HPP

#include "../gl.hpp"
#include <string>
#include <vector>
#include <algorithm>

#include "../util/align.hpp"

namespace gl
{

class UBO
{
public:
	static GLint bufferAlignSize; // = -1
	static GLuint nextIndex; // = 1

	void init();
	/*
	if (bufferAlignSize < 0)
	{
		GL_CHECK(glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &bufferAlignSize));
	}
	*/

	GLuint index = 0;
	GLuint buffer = 0;
	std::string name{};
	std::vector<GLubyte> bufferData{};
	size_t blockSize = 0;
	size_t typeSize = 0;

	UBO()
	{
		index = nextIndex;
		nextIndex++;
	}

	template<typename T>
	void attachData(const T *data)
	{
		typeSize = sizeof(T);
		blockSize = sizeof(T);
		bufferData.resize(typeSize);

		if ( ! buffer)
		{
			GL_CHECK(glGenBuffers(1, &buffer));
		}

		GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, buffer));
		GLubyte *dataPtr = nullptr;

		if (data != nullptr)
		{
			dataPtr = bufferData.data();
			std::copy_n(data, 1, reinterpret_cast<T*>(dataPtr));
		}

		GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, blockSize, reinterpret_cast<GLvoid*>(dataPtr), GL_DYNAMIC_DRAW));

		// bind();
	}

	template<typename T>
	void attachData(const T &data)
	{
		attachData<T>(&data);
	}

	template<typename T>
	void updateData(const T *data)
	{
		GLubyte *dataPtr = nullptr;

		if (data != nullptr)
		{
			dataPtr = bufferData.data();
			std::copy_n(data, 1, reinterpret_cast<T*>(dataPtr));
		}

		GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, buffer));
		GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, blockSize, reinterpret_cast<GLvoid*>(dataPtr)));
		// bind();
	}

	template<typename T>
	void updateData(const T &data)
	{
		0assert(&data != nullptr);
		updateData<T>(&data);
	}

	template<typename T>
	void attachArrayData(const T *data, size_t dataSize)
	{
		typeSize = sizeof(T);
		blockSize = typeSize + bufferAlignSize - (typeSize % bufferAlignSize);

		size_t bufferSize = blockSize * dataSize;
		bufferData.resize(bufferSize);

		if ( ! buffer)
		{
			GL_CHECK(glGenBuffers(1, &buffer));
		}

		GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, buffer));
		GLubyte *dataPtr = nullptr;

		if (data != nullptr && dataSize != 0)
		{
			dataPtr = bufferData.data();

			for (size_t i = 0; i < dataSize; i++)
			{
				std::copy_n(&data[i], 1, reinterpret_cast<T*>(dataPtr + i * blockSize));
			}
		}

		GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, bufferSize, reinterpret_cast<GLvoid*>(dataPtr), GL_DYNAMIC_DRAW));

		// bind();
	}

	template<typename T>
	void attachArrayData(const std::vector<T> &data)
	{
		assert(&data != nullptr);
		attachArrayData<T>(data.data(), data.size());
	}

	template<typename T>
	void updateArrayData(const T *data, size_t dataSize)
	{
		GLubyte *dataPtr = nullptr;

		if (data != nullptr && dataSize != 0)
		{
			dataPtr = bufferData.data();
			for (size_t i = 0; i < dataSize; i++)
			{
				std::copy_n(&data[i], 1, reinterpret_cast<T*>(dataPtr + i * blockSize));
			}
		}

		GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, buffer));
		GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, blockSize * dataSize, reinterpret_cast<GLvoid*>(dataPtr)));

		// bind();
	}

	template<typename T>
	void updateArrayData(const std::vector<T> &data)
	{
		assert(&data != nullptr);
		updateArrayData<T>(data.data(), data.size());
	}

	void bind()
	{
		GL_CHECK(glBindBufferRange(GL_UNIFORM_BUFFER, index, buffer, 0, typeSize));
	}

	void bindSubBuffer(size_t dataIndex)
	{
		GLintptr offset = dataIndex * blockSize;
		GL_CHECK(glBindBufferRange(GL_UNIFORM_BUFFER, index, buffer, offset, typeSize));
	}
};

} // gl

#endif