cmake_minimum_required(VERSION 3.15)

# set the project name and version
project(VulkanModelViewer VERSION 0.1)

# include macros

# set cpp version
set (CMAKE_CXX_STANDARD 17)

#--------------------------------------------------------------------------------------------------
# look for a base directory
if(NOT BASE_DIRECTORY_OWN)
  find_path(BASE_DIRECTORY_OWN
    NAMES core/cmake/setup.cmake
    PATHS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. 
    REQUIRED
    DOC "Directory containing core"
    )
endif()
message("Base directory found : ${BASE_DIRECTORY_OWN}")

# add subdirectories
add_subdirectory("core")
add_subdirectory("vulkan_model_viewer")
add_subdirectory("vulkan_model_viewer_ref")