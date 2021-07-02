#version 450

layout (set = 3, binding = 0) uniform sampler2D shadowMap;

layout (set = 1, binding = 1) uniform sampler2D samp;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inCamPos;
layout (location = 3) in vec3 inLightPos;
layout (location = 4) in vec4 inShadowCoord;
layout (location = 5) in vec2 inUV;
layout (location = 6) in vec3 fragVert;

layout (location = 0) out vec4 outFragColor;

#define ambient 0.1

float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 )
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = ambient;
		}
	}
	return shadow;
}

//void main()
//{
////	float shadow = textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));
////
////	vec3 N = normalize(inNormal);
////	vec3 L = normalize(inLightVec);
////	vec3 V = normalize(inViewVec);
////	vec3 R = normalize(-reflect(L, N));
////	vec3 diffuse = max(dot(N, L), ambient) * inColor;
////
////
////	outFragColor = vec4(diffuse * shadow, 1.0);
////	//outFragColor = vec4(diffuse * shadow, 1.0) * texture(samp, inUV);
//
//	float shadow = textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));
//
//}


////////////////////////////////////////////////////////////////////////////////////////////

///////// This code was externally sourced and can be found in the link below //////////////
///////// https://graphics.rwth-aachen.de:9000/Glow/glow-extras/blob/cfce9b4c4f5b806e4981c0ccad7de0e46edac68c/material/shader/glow-material/material-ggx.glsl //////////

////////////////////////////////////////////////////////////////////////////////////////////

// see http://www.filmicworlds.com/2014/04/21/optimizing-ggx-shaders-with-dotlh/
vec3 shadingSpecularGGX(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0)
{
	vec3 H = normalize(V + L);

	float dotLH = max(dot(L, H), 0.0);
	float dotNH = max(dot(N, H), 0.0);
	float dotNL = max(dot(N, L), 0.0);
	float dotNV = max(dot(N, V), 0.0);

	float alpha = roughness * roughness;

	// D (GGX normal distribution)
	float alphaSqr = alpha * alpha;
	float denom = dotNH * dotNH * (alphaSqr - 1.0) + 1.0;
	float D = alphaSqr / (denom * denom);
	// no pi because BRDF -> lighting

	// F (Fresnel term)
	float F_a = 1.0;
	float F_b = pow(1.0 - dotLH, 5); // manually?
	vec3 F = mix(vec3(F_b), vec3(F_a), F0);

	// G (remapped hotness, see Unreal Shading)
	float k = (alpha + 2 * roughness + 1) / 8.0;
	float G = dotNL / (mix(dotNL, 1, k) * mix(dotNV, 1, k));

	return D * F * G / 4.0;
}

// specular and diffuse contribution of a single light direction
vec3 shadingGGX(vec3 N, vec3 V, vec3 L, vec3 color, float roughness, float metallic)
{
	vec3 diffuse = color * (1 - metallic); // metals have no diffuse
	vec3 specular = mix(vec3(0.04), color, metallic); // fixed spec for non-metals

	float dotNL = max(dot(N, L), 0.0);

	return diffuse * dotNL + shadingSpecularGGX(N, V, L, roughness, specular);
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////



void main() {

	float shadow = textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));

	vec3 normal = normalize(inNormal);
	vec3 viewVec = fragVert - inCamPos;
	vec3 viewDir = normalize(fragVert - inCamPos);
	vec3 invViewDir = normalize(inCamPos - fragVert);

	vec3 totalLight = vec3(0.0);
	vec3 lightPos = inLightPos;
	vec3 lightVec = fragVert - lightPos;
	vec3 lightDir = normalize(fragVert - lightPos);
	vec3 invLightDir = normalize(lightPos - fragVert);
	totalLight = shadingGGX(normal, invViewDir, invLightDir, vec3(0.5), 0.5, 0.5);

	outFragColor = texture(samp, inUV) * vec4(totalLight, 1.0) * shadow;

//	float shadow = textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));
//
//	vec3 normal = normalize(inNormal);
//	vec3 viewVec = fragVert - inViewVec;
//	vec3 viewDir = normalize(fragVert - inViewVec);
//	vec3 invViewDir = normalize(inViewVec - fragVert);
//
//	vec3 totalLight = vec3(0.0);
//	vec3 lightPos = inLightPos;
//	vec3 lightVec = fragVert - lightPos;
//	vec3 lightDir = normalize(fragVert - lightPos);
//	vec3 invLightDir = normalize(lightPos - fragVert);
//	totalLight = shadingGGX(normal, invViewDir, invLightDir, vec3(0.5), 0.5, 0.5);
//
//	outFragColor = texture(samp, inUV) * vec4(totalLight, 1.0) * shadow;
}

//void main()
//{
//	float shadow = textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));
//
//	vec3 normal = normalize(inNormal);
//	vec3 viewVec = fragVert - inViewVec;
//	vec3 viewDir = normalize(fragVert - inViewVec);
//	vec3 invViewDir = normalize(inViewVec - fragVert);
//
//	vec3 totalLight = vec3(0.0);
//	vec3 lightPos = inLightPos;
//	vec3 lightVec = fragVert - lightPos;
//	vec3 lightDir = normalize(fragVert - lightPos);
//	vec3 invLightDir = normalize(lightPos - fragVert);
//	totalLight = shadingGGX(normal, invViewDir, invLightDir, vec3(0.5), 0.5, 0.5);
//
//	//outFragColor = vec4(diffuse * shadow, 1.0) * texture(samp, inUV) * vec4(totalLight, 1.0);
//	outFragColor = texture(samp, inUV) * vec4(totalLight, 1.0);
//	//outFragColor = vec4(diffuse * shadow, 1.0) * texture(samp, inUV);
//
//}
