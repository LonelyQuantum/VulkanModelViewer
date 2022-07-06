#include "tools.h"

#include <iostream>
#include <fstream>

//--------------------------------------------------------------------------------------------------
//Add unique const char* into a vector of const char*
//
void add_unique_c_str(std::vector<const char*>& v, const char* c_str) {
	bool contains = false;
	for (const char* e : v) {
		if (!strcmp(e, c_str)) {
			contains = true;
			break;
		}
	}
	if (!contains) v.push_back(c_str);
}

//--------------------------------------------------------------------------------------------------
// Read file by its path
//
std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
