#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <limits>
#include <core/rn/types.hpp>
#include <core/src.hpp>
#include <core/src/obj.hpp>
#include "args.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	try
	{
		args::parse(argc, argv);

		if (args::has('h'))
		{
			cout << "Usage: sbm [-o output.sbm] input.obj" << endl;
			return EXIT_SUCCESS;
		}

		char * last = args::last();

		if ( ! last)
		{
			cout << "Invalid input file" << endl;
			cout << "Usage: sbm [-o output.sbm] input.obj" << endl;
			return EXIT_SUCCESS;
		}

		string input{last};
		string output;

		auto values = args::get('o');

		if ( ! values.size())
		{
			if (input.compare(input.size() - 4, 4, ".obj") == 0)
			{
				output = input.substr(0, input.size() - 4) + ".sbm";
			}
			else
			{
				output = input + ".sbm";
			}
		}
		else
		{
			output = values[0];
		}

		cout << "input: " << input << endl;
		cout << "output: " << output << endl;

		auto mesh = src::obj::mesh(input);

		SRC_MESH_OPEN(mesh);

		ofstream outputFile(output, ios::out | ios::binary);

		char code[] = {'S', 'B', 'M', ' '};
		outputFile.write(reinterpret_cast<char *>(code), sizeof(code));

		uint32_t version = 0x0001;
		outputFile.write(reinterpret_cast<char *>(&version), sizeof(version));

		uint32_t size;
		uint32_t overall = sizeof(code) + sizeof(version);
		uint32_t typeVertices       = 0x0001;
		uint32_t typeIndices        = 0x0002;
		uint32_t typeLayouts        = 0x0003;
		uint32_t typeArrays         = 0x0004;
		uint32_t typeBounds         = 0x0005; // [squared radius, AABB:[MinX, MinY, MinZ, MaxX, MaxY, MaxZ]]

		// vertices
		size = sizeof(typeVertices) + mesh->vertexData.size;
		overall += size;
		outputFile.write(reinterpret_cast<char *>(&size), sizeof(size));
		outputFile.write(reinterpret_cast<char *>(&typeVertices), sizeof(typeVertices));
		outputFile.write(reinterpret_cast<char *>(mesh->vertexData.data), mesh->vertexData.size);

		// layouts
		size = sizeof(typeLayouts) + mesh->layouts.size() * sizeof(decltype(mesh->layouts)::value_type);
		overall += size;
		outputFile.write(reinterpret_cast<char *>(&size), sizeof(size));
		outputFile.write(reinterpret_cast<char *>(&typeLayouts), sizeof(typeLayouts));

		for (auto &layout : mesh->layouts)
		{
			outputFile.write(reinterpret_cast<char *>(&layout), sizeof(decltype(mesh->layouts)::value_type));
		}

		// indices
		if (mesh->indexData.size)
		{
			size = sizeof(typeIndices) + mesh->indexData.size;
			overall += size;
			outputFile.write(reinterpret_cast<char *>(&size), sizeof(size));
			outputFile.write(reinterpret_cast<char *>(&typeIndices), sizeof(typeIndices));
			outputFile.write(reinterpret_cast<char *>(mesh->indexData.data), mesh->indexData.size);
		}

		// arrays
		if (mesh->arrays.size())
		{
			size = sizeof(typeArrays) + mesh->layouts.size() * sizeof(decltype(mesh->arrays)::value_type);
			overall += size;
			outputFile.write(reinterpret_cast<char *>(&size), sizeof(size));
			outputFile.write(reinterpret_cast<char *>(&typeArrays), sizeof(typeArrays));

			for (auto &array : mesh->arrays)
			{
				outputFile.write(reinterpret_cast<char *>(&array), sizeof(decltype(mesh->arrays)::value_type));
			}
		}

		GLfloat sphereRadius2 = 0.f;
		GLfloat boundingBox[6] = {
			+numeric_limits<GLfloat>::max(), +numeric_limits<GLfloat>::max(), +numeric_limits<GLfloat>::max()
			-numeric_limits<GLfloat>::max(), -numeric_limits<GLfloat>::max(), -numeric_limits<GLfloat>::max()
		};

		for (auto &layout : mesh->layouts)
		{
			if (layout.index == rn::LayoutLocation::vert) {

				if (layout.type != GL_FLOAT) {
					clog << "Warning: bounding volumes calculation supports only float data types" << endl;
					break;
				}

				GLbyte *data = reinterpret_cast<GLbyte *>(mesh->vertexData.data);
				GLint step = layout.stride + layout.size * sizeof(GLfloat);

				for (GLint i = layout.stride; i < mesh->vertexData.size; i += step) {
					GLfloat radius2 = 0.f;

					for (GLint j = 0; j < layout.size; j++)
					{
						GLfloat v = reinterpret_cast<GLfloat *>(data + i)[j];

						boundingBox[j + 0] = min(boundingBox[j + 0], v);
						boundingBox[j + 3] = max(boundingBox[j + 3], v);
						radius2 += v * v;
					}

					sphereRadius2 = max(sphereRadius2, radius2);
				}

				break;
			}
		}

		size = sizeof(typeBounds) + sizeof(sphereRadius2) + sizeof(boundingBox);
		overall += size;
		outputFile.write(reinterpret_cast<char *>(&size), sizeof(size));
		outputFile.write(reinterpret_cast<char *>(&typeBounds), sizeof(typeBounds));
		outputFile.write(reinterpret_cast<char *>(&sphereRadius2), sizeof(sphereRadius2));
		outputFile.write(reinterpret_cast<char *>(&boundingBox), sizeof(boundingBox));

		cout << "done [" << overall << " bytes]" << endl;
	}
	catch (const exception &e)
	{
		cerr << "Exception: " << e.what() << endl;
		return EXIT_FAILURE;
	}
	catch (const string &e)
	{
		cerr << "Exception: " << e << endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		cerr << "Unknown exception" << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}