#version 460

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec3 inNormal;

layout (set = 0, binding = 0) uniform CameraUBO
{
	mat4 vp;
	vec3 pos;
	float rayCullDistance;
} cam;

layout (set = 1, binding = 0) uniform ModelUBO
{
	mat4 matrix;
} model;

layout (set = 2, binding = 0) uniform LightUBO
{
	mat4 proj;
	mat4 view;
	vec4 size_clippingPlanes;
	vec3 pos;
	vec3 direction;
	ivec4 extra;
} light;


layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outCamPos;
layout (location = 3) out vec3 outLightPos;
layout (location = 4) out vec4 outShadowCoord;
layout (location = 5) out vec2 outUV;
layout (location = 6) out vec3 outWorldPos;
layout (location = 7) out float outCull;
layout (location = 8) out vec3 outLightDirection;
layout (location = 9) out vec2 outLightClippingPlanes;
layout (location = 10) out vec2 outLightSize;
layout (location = 11) out int outLightType;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);

void main() 
{

	outColor = inColor;
	//outNormal = inNormal;
	outUV = inUV;
	outWorldPos = vec3(model.matrix * vec4(inPos, 1.0));
	gl_Position = cam.vp * vec4(outWorldPos, 1.0);

	outNormal = vec3(model.matrix * vec4(inNormal, 1.0));
	outLightPos = light.pos;
	outCamPos = cam.pos;
	outCull = cam.rayCullDistance;
	outLightClippingPlanes = light.size_clippingPlanes.zw;
	outLightSize = light.size_clippingPlanes.xy;
	outLightDirection = light.direction;
	outLightType = light.extra.x;

	outShadowCoord = (light.proj * light.view) * vec4(outWorldPos, 1.0);
}

