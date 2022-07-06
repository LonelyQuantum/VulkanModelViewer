#version 450

layout(binding = 0) uniform LightUniformObject {
    vec3 pos;
	vec3 color;
	mat4 mvp;
} light;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in int inMaterialId;

void main (){
    gl_Position = light.mvp * vec4(inPosition, 1.0f);
}