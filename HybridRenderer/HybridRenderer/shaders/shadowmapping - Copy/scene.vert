#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec3 inNormal;

//layout (binding = 0) uniform UBO
//{
//	mat4 projection;
//	mat4 view;
//	mat4 model;
//	mat4 lightSpace;
//	vec3 lightPos;
//	vec3 camPos;
//} ubo;

layout (set = 0, binding = 0) uniform CameraUBO
{
	mat4 projection;
	mat4 view;
	vec3 pos;
} cam;

layout (set = 0, binding = 0) uniform ModelUBO
{
	mat4 matrix;
} model;

layout (set = 2, binding = 0) uniform LightUBO
{
	mat4 space;
	vec3 pos;
} light;


layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outCamPos;
layout (location = 3) out vec3 outLightPos;
layout (location = 4) out vec4 outShadowCoord;
layout (location = 5) out vec2 outUV;
layout (location = 6) out vec3 outPos;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() 
{

	outColor = inColor;
	outNormal = inNormal;
	outUV = inUV;

	gl_Position = cam.projection * cam.view * model.matrix * vec4(inPos.xyz, 1.0);

	outPos = vec3(model.matrix * vec4(inPos, 1.0));
	outNormal = inNormal;
	outLightPos = light.pos;
	//outLightVec = normalize(ubo.lightPos - inPos);
	outCamPos = cam.pos;
	//outViewVec = -pos.xyz;

	outShadowCoord = (biasMat * light.space * model.matrix ) * vec4(inPos, 1.0);

//	outColor = inColor;
//	outNormal = inNormal;
//	outUV = inUV;
//
//	gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPos.xyz, 1.0);
//
//	outPos = vec3(ubo.model * vec4(inPos, 1.0));
//	outNormal = inNormal;
//	outLightVec = ubo.lightPos;
//	//outLightVec = normalize(ubo.lightPos - inPos);
//	outViewVec = ubo.camPos;
//	//outViewVec = -pos.xyz;
//
//	outShadowCoord = (biasMat * ubo.lightSpace * ubo.model ) * vec4(inPos, 1.0);
}

