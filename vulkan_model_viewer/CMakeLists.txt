#--------------------------------------------------------------------------------------------------
# Global setting
cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#--------------------------------------------------------------------------------------------------
# Project setting
get_filename_component(PROJNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJNAME ${PROJNAME})
project(${PROJNAME} LANGUAGES C CXX)
message(STATUS "-------------------------------")
message(STATUS "Processing Project ${PROJNAME}:")

#--------------------------------------------------------------------------------------------------
# Include macros and functions
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
# Get source, header and shader files
file(GLOB_RECURSE APP_SOURCE_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE SHADER_FILES *.glsl *.vert *.frag)
#print_list(SOURCE_FILES)
#print_list(SHADER_FILES)

#--------------------------------------------------------------------------------------------------
#add executable
add_executable(${PROJNAME} ${APP_SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_include_directories(${PROJNAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/headers" "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/shaders" ${PROJECT_BINARY_DIR})
#get_target_property(PROJ_INCLUDES ${PROJNAME} INCLUDE_DIRECTORIES)
#message("\nProejct ${PROJNAME} includes ${PROJ_INCLUDES}")


# function(compile_glsl22)
#   set(oneValueArgs DST VULKAN_TARGET HEADER DEPENDENCY FLAGS)
#   set(multiValueArgs SHADER_SOURCE_FILES SHADER_HEADER_FILES)
#   cmake_parse_arguments(COMPILE  "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

#   foreach(GLSL_SRC ${COMPILE_SHADER_SOURCE_FILES})

#     # Default compiler command, always adding debug information (Add and option to opt-out?)
#     set(COMPILE_CMD --target-env ${COMPILE_VULKAN_TARGET})

#     # Compilation to headers need a variable name, the output will be a .h
#     get_filename_component(FILE_NAME ${GLSL_SRC} NAME)
#     set(GLSL_OUT "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
#     list(APPEND _SPVS ${GLSL_OUT})


#     # Appending the output name and the file source
#     list(APPEND COMPILE_CMD  -o ${GLSL_OUT} ${GLSL_SRC} )

#     # The custom command is added to the build system, check for the presence of the output
#     # but also for changes done in GLSL headers 
#     add_custom_command(
#         PRE_BUILD
#         OUTPUT ${GLSL_OUT}
#         COMMAND echo ${GLSLANGVALIDATOR} ${COMPILE_CMD}
#         COMMAND ${GLSLANGVALIDATOR} ${COMPILE_CMD}
#         MAIN_DEPENDENCY ${GLSL_SRC}
#         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#         DEPENDS ${GLSL_DEPENDENCY}
#     )
# endforeach()
# set(SPV_OUTPUTS ${_SPVS} PARENT_SCOPE)
# endfunction()

#--------------------------------------------------------------------------------------------------
# GLSL to SPIR-V custom build
compile_glsl(
    SHADER_SOURCE_FILES ${SHADER_FILES} 
    DST ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
    VULKAN_TARGET vulkan1.2)

#--------------------------------------------------------------------------------------------------
# Group source files
source_group("Source Files" FILES ${APP_SOURCE_FILES})
source_group("Header Files" FILES ${HEADER_FILES})
source_group("Shader Files" FILES ${SHADER_FILES})

#--------------------------------------------------------------------------------------------------
# Setup configure header
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
configure_file(configFile.h.in configFile.h)

#--------------------------------------------------------------------------------------------------
# Link libraries
target_link_libraries(${PROJNAME} core)
foreach(CUSTOM_LIB ${CUSTOM_LIBS})
    target_link_libraries(${PROJNAME} ${CUSTOM_LIB})
endforeach()