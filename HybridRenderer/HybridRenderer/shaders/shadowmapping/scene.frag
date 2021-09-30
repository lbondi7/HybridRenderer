#version 460

#extension GL_EXT_ray_query : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout (set = 1, binding = 1) uniform sampler2D sampledTexture;

layout (set = 3, binding = 0) uniform sampler2D shadowMap;
layout (set = 3, binding = 1) uniform ShadowUBO
{
	int shadowMap;
	int vertexRotate;
	int texRotate;
	int baryRotate;
} shadowUBO;

struct Vertex
{
	vec3 position;
	vec2 uvCoord;
	vec3 colour;
	vec3 normal;
};

struct ObjDesc
{
	int textureIndex;  // Address of the triangle material index buffer
	uint64_t vertexAddress;
	uint64_t indicesAddress;
};

layout(buffer_reference, scalar) buffer Vertices {
	Vertex v[]; 
}; // Positions of an object
layout(buffer_reference, std430, buffer_reference_align = 16) buffer Indices {
	ivec3 i[]; 
}; // Triangle indices
//layout(buffer_reference, scalar) buffer TexIndices {int i; }; // Material ID for each triangle
layout (set = 4, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 4, binding = 1, scalar) buffer ObjDesc_ { 
	ObjDesc i[]; 
} objDesc;
layout(set = 4, binding = 2) uniform sampler2D textureSamplers[];

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

float textureProj(vec4 shadowCoord, vec2 off);

////////////////////////////////////////////////////////////////////////////////////////////

///////// This code was externally sourced and can be found in the link below //////////////
///////// https://graphics.rwth-aachen.de:9000/Glow/glow-extras/blob/cfce9b4c4f5b806e4981c0ccad7de0e46edac68c/material/shader/glow-material/material-ggx.glsl //////////

////////////////////////////////////////////////////////////////////////////////////////////

// see http://www.filmicworlds.com/2014/04/21/optimizing-ggx-shaders-with-dotlh/
vec3 shadingSpecularGGX(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0);

// specular and diffuse contribution of a single light direction
vec3 shadingGGX(vec3 N, vec3 V, vec3 L, vec3 color, float roughness, float metallic);
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

float random (vec2 st);

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

	if(shadow < 0.5 || shadowUBO.shadowMap == 2)
	{
		if(distance(inCamPos, fragVert) < inCull)
		{

			{
				// rayQueryEXT rayQuery;
				// vec4 tempCol;
				// int lightSample = 1;
				// int vertexSample = 1;
				// for(int i = 0; i < lightSample; ++i)
				// {
				// 	vec4 tempCol2 = vec4(0.0);
				// 	vec3 lPos = inLightPos + vec3(offsets[i], 0);
				// 	for(int j = 0; j < vertexSample; ++j){
				// 		vec3 vPos = fragVert + vec3(offsets[j], 0);
				// 		vec3 iLD = normalize(lPos - vPos);
				// 		rayQueryInitializeEXT(rayQuery, 
				// 		topLevelAS, 
				// 		gl_RayFlagsTerminateOnFirstHitEXT, 
				// 		0xFF, 
				// 		vPos, 
				// 		0.01, 
				// 		iLD, 
				// 		1000.0);
				// 		// Start the ray traversal, rayQueryProceedEXT returns false if the traversal is complete
				// 		while (rayQueryProceedEXT(rayQuery)) { 
				// 		}
				// 		// If the intersection has hit a triangle, the fragment is shadowed
				// 		if (rayQueryGetIntersectionTypeEXT(rayQuery, true) == gl_RayQueryCommittedIntersectionTriangleEXT ) 
				//     	{
				// 			vec2 coords = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
				// 			tempCol2 += vec4(texture(sampledTexture, coords).a);
				// 			// if(texture(sampledTexture, coords).a > 0.5)
				// 			// {
				// 			// 	tempCol2 += vec4(texture(sampledTexture, coords).a);
				// 			// }
				// 		}
				// 		else
				// 		{
				// 			tempCol2 = outColour * 0.1;
				// 		}
				// 	}
				// 	tempCol2 /= vertexSample;
				// 	tempCol += tempCol2;
				// }
				// tempCol /= lightSample;
				// outColour = tempCol;
			}

			rayQueryEXT rayQuery;

			// initialise the ray to query but doesn't start the traversal
			rayQueryInitializeEXT(rayQuery, 
			topLevelAS, 
			gl_RayFlagsOpaqueEXT, 
			0xFF, 
			fragVert, 
			0.01, 
			invLightDir, 
			1000.0);

			// Start the ray traversal, rayQueryProceedEXT returns false if the traversal is complete
			while (rayQueryProceedEXT(rayQuery)) 
			{ 
			
			// If the intersection has hit a triangle, the fragment is shadowed
			}
				if (rayQueryGetIntersectionTypeEXT(rayQuery, true) == gl_RayQueryCommittedIntersectionTriangleEXT ) 
				{

					// Indices indices     = Indices(objResource.indicesAddress);
					// Vertices vertices    = Vertices(objResource.vertexAddress);

					//outColour.r = 1.0;
					//if (rayQueryGetIntersectionFrontFaceEXT(rayQuery, false)) 
					//{
						int objIndex = rayQueryGetIntersectionInstanceIdEXT(rayQuery, true);
						ObjDesc    objResource = objDesc.i[objIndex];
						Indices indices = Indices(objResource.indicesAddress);
						Vertices vertices = Vertices(objResource.vertexAddress);
						// Indices of the triangle
						ivec3 ind = indices.i[rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, true)];

						// // Vertex of the triangle
						Vertex v0 = vertices.v[ind.x];
						Vertex v1 = vertices.v[ind.y];
						Vertex v2 = vertices.v[ind.z];
						if(shadowUBO.vertexRotate == 0){
							v0 = vertices.v[ind.x];
							v1 = vertices.v[ind.y];
							v2 = vertices.v[ind.z];
						}
						if(shadowUBO.vertexRotate == 1){
							v0 = vertices.v[ind.x];
							v1 = vertices.v[ind.z];
							v2 = vertices.v[ind.y];
						}
						if(shadowUBO.vertexRotate == 2){
							v0 = vertices.v[ind.y];
							v1 = vertices.v[ind.x];
							v2 = vertices.v[ind.z];
						}
						if(shadowUBO.vertexRotate == 3){
							v0 = vertices.v[ind.y];
							v1 = vertices.v[ind.z];
							v2 = vertices.v[ind.x];
						}
						if(shadowUBO.vertexRotate == 4){
							v0 = vertices.v[ind.z];
							v1 = vertices.v[ind.x];
							v2 = vertices.v[ind.y];
						}
						if(shadowUBO.vertexRotate == 5){
							v0 = vertices.v[ind.z];
							v1 = vertices.v[ind.y];
							v2 = vertices.v[ind.x];
						}

						vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
						// const vec3 barycentrics = vec3(attribs.x, 1.0 - attribs.y - attribs.x, attribs.y);

						vec3 barycentrics = vec3(0);
						if(shadowUBO.baryRotate == 0){
							barycentrics = vec3(attribs.x, attribs.y, 1.0 - attribs.y - attribs.x);
						}
						if(shadowUBO.baryRotate == 1){
							barycentrics = vec3(attribs.x, 1.0 - attribs.y - attribs.x, attribs.y);
						}
						if(shadowUBO.baryRotate == 2){
							barycentrics = vec3(attribs.y, attribs.x, 1.0 - attribs.y - attribs.x);
						}
						if(shadowUBO.baryRotate == 3){
							barycentrics = vec3(attribs.y, 1.0 - attribs.y - attribs.x, attribs.x);
						}
						if(shadowUBO.baryRotate == 4){
							barycentrics = vec3(1.0 - attribs.y - attribs.x, attribs.x, attribs.y);
						}
						if(shadowUBO.baryRotate == 5){
							barycentrics = vec3(1.0 - attribs.y - attribs.x, attribs.y, attribs.x);
						}

						//vec2 texCoord = v0.uvCoord * barycentrics.x + v1.uvCoord * barycentrics.y + v2.uvCoord * barycentrics.z;
						vec2 texCoord = vec2(0);
						if(shadowUBO.texRotate == 0){
							texCoord = v0.uvCoord * barycentrics.x + v1.uvCoord * barycentrics.y + v2.uvCoord * barycentrics.z;
						}
						if(shadowUBO.texRotate == 1){
							texCoord = v0.uvCoord * barycentrics.x + v1.uvCoord * barycentrics.z + v2.uvCoord * barycentrics.y;
						}
						if(shadowUBO.texRotate == 2){
							texCoord = v0.uvCoord * barycentrics.y + v1.uvCoord * barycentrics.x + v2.uvCoord * barycentrics.z;
						}
						if(shadowUBO.texRotate == 3){
							texCoord = v0.uvCoord * barycentrics.y + v1.uvCoord * barycentrics.z + v2.uvCoord * barycentrics.x;
						}
						if(shadowUBO.texRotate == 4){
							texCoord = v0.uvCoord * barycentrics.z + v1.uvCoord * barycentrics.x + v2.uvCoord * barycentrics.y;
						}
						if(shadowUBO.texRotate == 5){
							texCoord = v0.uvCoord * barycentrics.z + v1.uvCoord * barycentrics.y + v2.uvCoord * barycentrics.x;
						}

						float alpha = texture(textureSamplers[objResource.textureIndex], texCoord).a;
						//float alpha = texture(textureSamplers[objResource.textureIndex], attribs).a;
						if(alpha > 0.5){
							outColour *= (0.1);
						}
						// else{
						// 	outColour *= 0.1;
						// }
						
					//}
					// else{
					// 	outColour *= 0.1;
					//}
				}
		}
		else if(shadowUBO.shadowMap != 2)
		{
			//outColour.g = 1.0;
			outColour *= 0.1;
		}
	}

	outFragColor = outColour;
}

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

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