#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

layout (set = 3, binding = 0) uniform sampler2D shadowMap;
layout (set = 1, binding = 1) uniform sampler2D sampledTexture;
layout (set = 4, binding = 0) uniform accelerationStructureEXT topLevelAS;

layout (set = 3, binding = 1) uniform ShadowUBO
{
	uint shadowMap;
} shadowUBO;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inCamPos;
layout (location = 3) in vec3 inLightPos;
layout (location = 4) in vec4 inShadowCoord;
layout (location = 5) in vec2 inUV;
layout (location = 6) in vec3 fragVert;
layout (location = 7) in float inCull;

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


float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

float offsetAmount = 0.02;
vec2 offsets[17] = {vec2(0), vec2(offsetAmount), vec2(-offsetAmount), 
vec2(-offsetAmount, offsetAmount), vec2(offsetAmount, -offsetAmount), 
vec2(offsetAmount, 0), vec2(0, offsetAmount), vec2(-offsetAmount, 0), 
vec2(0, -offsetAmount), 
vec2(offsetAmount / 2), vec2(-offsetAmount / 2), 
vec2(-offsetAmount / 2, offsetAmount/ 2), vec2(offsetAmount/ 2, -offsetAmount/ 2), 
vec2(offsetAmount/ 2, 0), vec2(0, offsetAmount/ 2), vec2(-offsetAmount / 2, 0), 
vec2(0, -offsetAmount/ 2)};

void main() {

	vec4 col = texture(sampledTexture, inUV);
	if(col.a < 0.2)
		discard;

	float shadow = textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));

	vec4 outColour;
	vec3 normal = normalize(inNormal);
	vec3 viewVec = fragVert - inCamPos;
	vec3 viewDir = normalize(fragVert - inCamPos);
	vec3 invViewDir = normalize(inCamPos - fragVert);

	vec3 totalLight = vec3(0.0);
	vec3 lightPos = inLightPos;
	vec3 lightVec = fragVert - lightPos;
	//vec3 lightDir = normalize(fragVert - lightPos);
	vec3 invLightDir = normalize(lightPos - fragVert);
	totalLight = shadingGGX(normal, invViewDir, invLightDir, vec3(0.5), 0.8, 0.05);
	outColour = col * vec4(totalLight, 1.0);

	if(shadowUBO.shadowMap == 0){
		outColour *= shadow;	
	}
	else if(shadowUBO.shadowMap == 2 || shadow < 0.5)
	{
		if(distance(inCamPos, fragVert) < inCull)
		{
			rayQueryEXT rayQuery;

			vec4 tempCol;
			int lightSample = 9;
			int vertexSample = 9;
			for(int i = 0; i < lightSample; ++i)
			{
				vec4 tempCol2 = vec4(0.0);
				vec3 lPos = inLightPos + vec3(offsets[i], 0);
				for(int j = 0; j < vertexSample; ++j){
					vec3 vPos = fragVert + vec3(offsets[j], 0);

					vec3 iLD = normalize(lPos - vPos);
					rayQueryInitializeEXT(rayQuery, 
					topLevelAS, 
					gl_RayFlagsTerminateOnFirstHitEXT, 
					0xFF, 
					vPos, 
					0.01, 
					iLD, 
					1000.0);

					// Start the ray traversal, rayQueryProceedEXT returns false if the traversal is complete
					while (rayQueryProceedEXT(rayQuery)) { 
					}
					// If the intersection has hit a triangle, the fragment is shadowed
					if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionTriangleEXT ) {
						//outColour *= shadow;
						tempCol2 += outColour;
					}
				}
				tempCol2 /= vertexSample;
				tempCol += tempCol2;
			}
			tempCol /= lightSample;
			outColour = tempCol;

			// rayQueryEXT rayQuery;

			// // initialise the ray to query but doesn't start the traversal
			// rayQueryInitializeEXT(rayQuery, 
			// topLevelAS, 
			// gl_RayFlagsCullBackFacingTrianglesEXT, 
			// 0xFF, 
			// fragVert, 
			// 0.01, 
			// invLightDir, 
			// 1000.0);

			// // Start the ray traversal, rayQueryProceedEXT returns false if the traversal is complete
			// while (rayQueryProceedEXT(rayQuery)) 
			// { 
			// }
			// // If the intersection has hit a triangle, the fragment is shadowed
			// if (rayQueryGetIntersectionTypeEXT(rayQuery, true) == gl_RayQueryCommittedIntersectionTriangleEXT ) 
			// {
			// 	outColour.r = 1.0;
			// 	// if (rayQueryGetIntersectionFrontFaceEXT(rayQuery, true)) 
			// 	// {

			// 	// 	vec2 coords = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
			// 	// 	outColour = vec4(texture(sampledTexture, coords).a);
			// 	// }
			// 	// else{
			// 	// 	outColour = vec4(0);
			// 	// }
			// }
		}
		else{
			//outColour.g = 1.0;
			outColour *= shadow;
		}
	}

	outFragColor = outColour;
}
