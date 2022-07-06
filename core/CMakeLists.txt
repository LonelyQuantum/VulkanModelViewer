cmake_minimum_required(VERSION 3.15)

#--------------------------------------------------------------------------------------------------
# Project setting
get_filename_component(LIBNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${LIBNAME} LANGUAGES C CXX)
message(STATUS "-------------------------------")
message(STATUS "Processing Library ${LIBNAME}:")

#--------------------------------------------------------------------------------------------------
# Include macros
include(${BASE_DIRECTORY_OWN}/core/cmake/setup.cmake)
include(${BASE_DIRECTORY_OWN}/core/cmake/utilities.cmake)

#--------------------------------------------------------------------------------------------------
# Begin project setup
_begin_project_setup()

#--------------------------------------------------------------------------------------------------
# Add packages
_add_package_VulkanSDK()
_add_package_GLFW()
_add_package_ImGUI()
_add_package_glm()
_add_package_stb()
_add_package_tinyobjloader()
_add_package_imgui_filebrowser()

#--------------------------------------------------------------------------------------------------
# Add vulkan common and common tools
file(GLOB VULKAN_COMMON_SOURCES vulkan_common/*.cpp vulkan_common/*.h)
file(GLOB VULKAN_COMMON_HEADERS vulkan_common/*.h)
file(GLOB COMMON_TOOL_SOURCES common_tools/*.cpp common_tools/*.h)
file(GLOB COMMON_TOOL_HEADERS common_tools/*.h)

#--------------------------------------------------------------------------------------------------
#add library
add_library(${LIBNAME} ${VULKAN_COMMON_SOURCES} ${COMMON_TOOL_SOURCES})
target_include_directories(${LIBNAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/vulkan_common")
target_include_directories(${LIBNAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/common_tools")

source_group("vulkan_common" FILES ${VULKAN_COMMON_SOURCES})
source_group("common_tools" FILES ${COMMON_TOOL_SOURCES})

#print_list(CUSTOM_LIBS)
foreach(CUSTOM_LIB ${CUSTOM_LIBS})
    target_link_libraries(${LIBNAME} ${CUSTOM_LIB})
endforeach()

