#version 450

layout(set = 0, binding = 0) uniform CameraUniformObject {
    mat4 model;
	mat4 view;
	mat4 proj;
	vec3 pos;
} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in int inMaterialId;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outPosition;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out int outMaterialId;
layout(location = 5) out vec4 shadowTexCoord;

void main() {
	gl_Position = camera.proj * camera.view * camera.model * vec4(inPosition + inNormal * 0.001, 1.0);
	fragColor = inColor;
    fragTexCoord = inTexCoord;
    outPosition = inPosition;
    outNormal = inNormal;
	outMaterialId = inMaterialId;
}