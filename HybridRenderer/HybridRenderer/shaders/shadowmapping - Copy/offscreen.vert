#version 450

layout (location = 0) in vec3 inPos;

//layout (binding = 0) uniform UBO
//{
//	mat4 depthMVP;
//} ubo;

layout (set = 0, binding = 0) uniform LightUBO
{
	mat4 space;
	vec3 pos;
} light;

//layout (binding = 3) uniform UB
//{
//	mat4 projection;
//	mat4 view;
//	mat4 model;
//	mat4 lightSpace;
//	vec3 lightPos;
//} ubo2;

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
//	gl_Position =  ubo.depthMVP * model.matrix * vec4(inPos, 1.0);
}