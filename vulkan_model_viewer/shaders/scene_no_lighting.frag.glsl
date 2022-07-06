#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(set = 0, binding = 0) uniform CameraUniformObject {
    mat4 model;
	mat4 view;
	mat4 proj;
	vec3 pos;
} camera;

layout(set = 0, binding = 1) uniform LightUniformObject {
    vec3 pos;
	vec3 color;
	mat4 mvp;
} light;


layout(set = 0, binding = 2) uniform sampler2D shadow_texture;

layout(set = 1, binding = 0) uniform MaterialUniformObject {
     vec3 ambient;
	 vec3 diffuse;
	 vec3 specular;
	 vec3 transmittance;
	 vec3 emission;

	 int illumModelIndex;

	 float shininess;
	 float ior;
	 float dissolve;
	 float roughness;          
	 float metallic;           
	 float sheen;              
	 float clearcoat_thickness; 
	 float clearcoat_roughness; 
	 float anisotropy;         
	 float anisotropy_rotation; 

	 int ambient_texture_ind;
	 int diffuse_texture_ind;
	 int specular_texture_ind;
	 int specular_highlight_texture_ind;
	 int bump_texture_ind;
	 int displacement_texture_ind;
	 int alpha_texture_ind;
	 int reflection_texture_ind;

	 int roughness_texture_ind;
	 int metallic_texture_ind;
	 int sheen_texture_ind;
	 int emissive_texture_ind;
	 int normal_texture_ind;
} material;

layout(set = 1, binding = 1) uniform sampler2D ambient_texture;
layout(set = 1, binding = 2) uniform sampler2D diffuse_texture;
layout(set = 1, binding = 3) uniform sampler2D specular_texture;
layout(set = 1, binding = 4) uniform sampler2D specular_highlight_texture;
layout(set = 1, binding = 5) uniform sampler2D bump_texture;
layout(set = 1, binding = 6) uniform sampler2D displacement_texture;
layout(set = 1, binding = 7) uniform sampler2D alpha_texture;
layout(set = 1, binding = 8) uniform sampler2D reflection_texture;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 inPosition;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in flat int inMaterialId;
layout(location = 5) in vec4 inShadowCoord;

layout(location = 0) out vec4 outColor;

float shadowMap(vec4 shadowCoord, vec2 off)
{   
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadow_texture, shadowCoord.xy + off).r + 0.0000;
		if (dist < shadowCoord.z ) 
		{
			shadow = 0.0f;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadow_texture, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 2;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += shadowMap(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}


void main() {
	vec3 normal = normalize(inNormal);

	vec4 kd = {0.0f, 0.0f, 0.0f, 0.0f};
	if (material.diffuse_texture_ind != 0)
		kd = texture(diffuse_texture, fragTexCoord);
	else
		kd = vec4(material.diffuse, 1.0f);

	vec4 ks = {0.0f, 0.0f, 0.0f, 0.0f};
	if (material.specular_texture_ind != 0)
		ks = texture(specular_texture, fragTexCoord);
	else
		ks = vec4(material.specular, 1.0f);

	vec4 ka = {0.0f, 0.0f, 0.0f, 0.0f};
	if (material.ambient_texture_ind != 0)
		ka = texture(ambient_texture, fragTexCoord);
	else
		ka = vec4(material.ambient, 1.0f);

	float shadow = filterPCF(inShadowCoord / inShadowCoord.w);

    vec4 L_d = kd;
    vec4 L_a = ka * kd;
	outColor = L_d * shadow + L_a;
}