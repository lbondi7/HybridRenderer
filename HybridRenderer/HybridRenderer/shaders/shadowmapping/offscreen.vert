#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec2 outUV;

// layout (set = 0, binding = 0) uniform LightUBO
// {
// 	mat4 space;
// 	vec3 pos;
// } light;

layout (set = 0, binding = 0) uniform LightUBO
{
	mat4 proj;
	mat4 view;
	vec4 size_clippingPlanes;
	vec3 pos;
	vec3 direction;
	ivec4 extra;
} light;

layout (set = 1, binding = 0) uniform ModelUBO
{
	mat4 matrix;
} model;

out gl_PerVertex 
{
    vec4 gl_Position;   
};


void main()
{
	outUV = inUV;
	gl_Position =  (light.proj * light.view * model.matrix) * vec4(inPos, 1.0);
}