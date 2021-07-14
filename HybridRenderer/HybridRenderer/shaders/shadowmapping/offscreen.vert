#version 450

layout (location = 0) in vec3 inPos;

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
	gl_Position =  light.space * model.matrix * vec4(inPos, 1.0);
}