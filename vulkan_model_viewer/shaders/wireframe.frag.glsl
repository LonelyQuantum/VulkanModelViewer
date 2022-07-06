#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(set = 0, binding = 0) uniform CameraUniformObject {
    mat4 model;
	mat4 view;
	mat4 proj;
	vec3 pos;
} camera;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 inPosition;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in flat int inMaterialId;
layout(location = 5) in vec4 inShadowCoord;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}