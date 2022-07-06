#define TINYOBJLOADER_IMPLEMENTATION 
#define STB_IMAGE_IMPLEMENTATION

#include "vulkan_model_viewer.h"

int main(int argc, char* argv[]) {

	VulkanModelViewer modelViewer{};
	modelViewer.run();
}
