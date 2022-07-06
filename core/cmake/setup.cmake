### Project configuration
macro(_begin_project_setup)
  unset(CUSTOM_INCLUDE_DIRS)
  unset(CUSTOM_LIBS)

  set(GLSLANGVALIDATOR "glslangValidator")
  set(GLSLC "glslc")
endmacro()
 

### Vulkan SDK
# Add VulkanSDK
macro(_add_package_VulkanSDK)
  message(">>>>>>Add Vulkan_SDK")
  if(DEFINED  ENV{VULKAN_SDK})
    set(VULKAN_PATH $ENV{VULKAN_SDK})
    string(REPLACE "\\" "/" VULKAN_PATH ${VULKAN_PATH})
  endif()
  set(VULKAN_INCLUDE_DIR ${VULKAN_PATH}/Include)
  set(VULKAN_LIB_DIR ${VULKAN_PATH}/Lib)
  file(GLOB VULKAN_LIBS ${VULKAN_LIB_DIR}/*.lib)

  if(NOT TARGET vulkan)
    add_library(vulkan INTERFACE)
    target_include_directories(vulkan INTERFACE ${VULKAN_INCLUDE_DIR})
    foreach(VULKAN_LIB ${VULKAN_LIBS})
      target_link_libraries(vulkan INTERFACE ${VULKAN_LIB})
    endforeach() 
  endif()
  list(APPEND CUSTOM_LIBS vulkan)
endmacro()

### Third-party pre-built libraries
# Add GLFW
macro(_add_package_GLFW)
  message(">>>>>>Add GLFW")
  set(GLFW_PATH ${BASE_DIRECTORY_OWN}/core/third_party/glfw-3.3.6.bin.WIN64)
  set(GLFW_INCLUDE_DIR ${GLFW_PATH}/include)
  set(GLFW_LIB_DIR ${GLFW_PATH}/lib-vc2022)
  file(GLOB GLFW_LIBS ${GLFW_LIB_DIR}/*.lib)

  if(NOT TARGET glfw)
    add_library(glfw INTERFACE)
    target_include_directories(glfw INTERFACE ${GLFW_INCLUDE_DIR})
    foreach(GLFW_LIB ${GLFW_LIBS})
      target_link_libraries(glfw INTERFACE ${GLFW_LIB})
    endforeach() 
  endif()

  list(APPEND CUSTOM_LIBS glfw)
  set(USING_GLFW ON)
endmacro()

### Third-party sources libraries
# Add ImGui
macro(_add_package_ImGUI)
  message(">>>>>>Add ImGui")
  
  if(NOT TARGET glfw)
    _add_package_GLFW()
  endif()
  
  set(IMGUI_DIR ${BASE_DIRECTORY_OWN}/core/third_party/imgui-master)

  set(IMGUI_SOURCE 
      ${IMGUI_DIR}/imgui.cpp
      ${IMGUI_DIR}/imgui_draw.cpp
      ${IMGUI_DIR}/imgui_demo.cpp
      ${IMGUI_DIR}/imgui_widgets.cpp
      ${IMGUI_DIR}/imgui_tables.cpp
      ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
      ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
      )
  
  if(NOT TARGET ImGuiOwn)
    add_library(ImGuiOwn ${IMGUI_SOURCE})
    set_property(TARGET ImGuiOwn PROPERTY FOLDER "ThirdParty")
    target_include_directories(ImGuiOwn PUBLIC ${GLFW_INCLUDE_DIR} ${IMGUI_DIR} ${VULKAN_INCLUDE_DIR})
  endif()
  list(APPEND CUSTOM_LIBS ImGuiOwn)

  set(USING_IMGUI ON)
endmacro()

### Third-party header-only libraries
# Add glm
macro(_add_package_glm)
  message(">>>>>>Add glm")
  set(GLM_DIR ${BASE_DIRECTORY_OWN}/core/third_party/glm)
  if(NOT TARGET glm)
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE ${GLM_DIR})
  endif()
  list(APPEND CUSTOM_LIBS glm)
  set(USING_GLM ON)
endmacro()

# Add stb
macro(_add_package_stb)
  message(">>>>>>Add stb")
  set(STB_DIR ${BASE_DIRECTORY_OWN}/core/third_party/stb-master)
  if(NOT TARGET stb)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${STB_DIR})
  endif()
  list(APPEND CUSTOM_LIBS stb)
  set(USING_STB ON)
endmacro()

# Add tinyobjtloader
macro(_add_package_tinyobjloader)
  message(">>>>>>Add tinyobjloader")
  set(TINYOBJLOADER_DIR ${BASE_DIRECTORY_OWN}/core/third_party/tinyobjloader-master)
  if(NOT TARGET tinyobjloader)
    add_library(tinyobjloader INTERFACE)
    target_include_directories(tinyobjloader INTERFACE ${TINYOBJLOADER_DIR})
  endif()
  list(APPEND CUSTOM_LIBS tinyobjloader)
  set(USING_TINYOBJLOADER ON)
endmacro()

# Add imgui-filebrowser
macro(_add_package_imgui_filebrowser)
  message(">>>>>>Add imgui-filebrowser")
  set(IMGUI_FILEBROWSER_DIR ${BASE_DIRECTORY_OWN}/core/third_party/imgui-filebrowser-master)
  if(NOT TARGET ImGuiOwn)
    _add_package_ImGUI()
  endif()
  if(NOT TARGET imgui-filebrowser)
    add_library(imgui-filebrowser INTERFACE)
    target_include_directories(imgui-filebrowser INTERFACE ${IMGUI_FILEBROWSER_DIR})
  endif()
  list(APPEND CUSTOM_LIBS imgui-filebrowser)
  set(USING_IMGUI_FILEBROWSER ON)
endmacro()