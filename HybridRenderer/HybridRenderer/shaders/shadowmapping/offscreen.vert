#version 460

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec2 outUV;

layout (set = 0, binding = 0) uniform LightUBO
{
	mat4 space;
	vec3 pos;
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
	gl_Position =  light.space * model.matrix * vec4(inPos, 1.0);
}