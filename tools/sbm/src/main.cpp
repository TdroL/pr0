#include <iostream>
#include <fstream>
#include <string>
#include <core/src.hpp>
#include <core/src/obj.hpp>
#include "args.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	try
	{
		args::parse(argc, argv);

		auto values = args::get('i');

		if ( ! values.size())
		{
			cout << "Invalid input file" << endl;
			return EXIT_SUCCESS;
		}

		string input{values[0]};
		string output;

		values = args::get('o');

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

		mesh->use();

		ofstream outputFile(output, ios::out | ios::binary);

		char code[4] = {'S', 'B', 'M', ' '};
		outputFile.write(reinterpret_cast<char *>(code), 4);

		uint32_t version = 0x0001;
		outputFile.write(reinterpret_cast<char *>(&version), sizeof(version));

		uint32_t size, overall = 4 + sizeof(version);
		uint32_t typeVertices = 0x0001;
		uint32_t typeIndices =  0x0002;
		uint32_t typeLayouts =  0x0003;
		uint32_t typeArrays =   0x0004;

		// vertices
		size = sizeof(typeVertices) + mesh->vertexData.size;
		overall += size;
		outputFile.write(reinterpret_cast<char *>(&size), sizeof(size));
		outputFile.write(reinterpret_cast<char *>(&typeVertices), sizeof(typeVertices));
		outputFile.write(reinterpret_cast<char *>(mesh->vertexData.data), mesh->vertexData.size);

		// indices
		size = sizeof(typeIndices) + mesh->indexData.size;
		overall += size;
		outputFile.write(reinterpret_cast<char *>(&size), sizeof(size));
		outputFile.write(reinterpret_cast<char *>(&typeIndices), sizeof(typeIndices));
		outputFile.write(reinterpret_cast<char *>(mesh->indexData.data), mesh->indexData.size);

		// layouts
		size = sizeof(typeLayouts) + mesh->layouts.size() * sizeof(decltype(mesh->layouts)::value_type);
		overall += size;
		outputFile.write(reinterpret_cast<char *>(&size), sizeof(size));
		outputFile.write(reinterpret_cast<char *>(&typeLayouts), sizeof(typeLayouts));

		for (auto &layout : mesh->layouts)
		{
			outputFile.write(reinterpret_cast<char *>(&layout), sizeof(decltype(mesh->layouts)::value_type));
		}

		// arrays
		size = sizeof(typeArrays) + mesh->layouts.size() * sizeof(decltype(mesh->arrays)::value_type);
		overall += size;
		outputFile.write(reinterpret_cast<char *>(&size), sizeof(size));
		outputFile.write(reinterpret_cast<char *>(&typeArrays), sizeof(typeArrays));

		for (auto &array : mesh->arrays)
		{
			outputFile.write(reinterpret_cast<char *>(&array), sizeof(decltype(mesh->arrays)::value_type));
		}

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