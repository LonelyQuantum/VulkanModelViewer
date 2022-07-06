#ifndef TOOLS_COMMON
#define TOOLS_COMMON
#include <vector>
#include <string>

void add_unique_c_str(std::vector<const char*>& v, const char* c_str);

std::vector<char> readFile(const std::string& filename);

// Add unique element into a vector
template <typename T>
void add_unique(std::vector<T>& v, T element) {
	if (std::is_same_v<T, const char*>) {
		add_unique_c_str(v, element);
	}
	else {
		if (std::find(v.begin(), v.end(), element) == v.end())
		{
			v.push_back(element);
		}
	}
}
#endif // !TOOLS_COMMON
