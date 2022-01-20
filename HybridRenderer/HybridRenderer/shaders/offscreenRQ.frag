#version 460

layout(set = 1, binding = 1) uniform sampler2D meshTexture;

layout (location = 0) in vec2 inUV;

void main() 
{	
	float alpha = texture(meshTexture, inUV).a;
	if (alpha < 0.1) {
		discard;
	}
}