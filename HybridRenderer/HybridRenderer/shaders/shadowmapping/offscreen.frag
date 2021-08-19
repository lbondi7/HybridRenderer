#version 460

layout(set = 2, binding = 0) uniform sampler2D meshTexture;

layout (location = 0) in vec2 inUV;

void main() 
{	
	float alpha = texture(meshTexture, inUV).a;
	if (alpha < 0.5) {
		discard;
	}
}