#version 450

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO 
{
	mat4 depthMVP;
} ubo;

layout (binding = 3) uniform UB
{
	mat4 projection;
	mat4 view;
	mat4 model;
	mat4 lightSpace;
	vec3 lightPos;
} ubo2;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

 
void main()
{
	gl_Position =  ubo.depthMVP * ubo2.model * vec4(inPos, 1.0);
}