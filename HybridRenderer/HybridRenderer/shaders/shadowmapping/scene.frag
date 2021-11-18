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
	int confirmIntersection;
	int terminateRay;
	float alphaThreshold;
	float bias;
	float blockerScale;
	int range;
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
layout(buffer_reference, scalar) buffer Indices {
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
layout (location = 8) in vec4 inShadowViewCoord;
layout (location = 9) in vec3 inLightDirection;
layout (location = 10) in vec2 inLightClippingPlanes;
layout (location = 11) in float inLightSize;

layout (location = 0) out vec4 outFragColor;

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

float random(in vec2 xy, in float seed);

float offsetAmount = 0.02;
vec2 offsets[17] = {vec2(0), vec2(offsetAmount), vec2(-offsetAmount), 
vec2(-offsetAmount, offsetAmount), vec2(offsetAmount, -offsetAmount), 
vec2(offsetAmount, 0), vec2(0, offsetAmount), vec2(-offsetAmount, 0), 
vec2(0, -offsetAmount), 
vec2(offsetAmount / 2), vec2(-offsetAmount / 2), 
vec2(-offsetAmount / 2, offsetAmount/ 2), vec2(offsetAmount/ 2, -offsetAmount/ 2), 
vec2(offsetAmount/ 2, 0), vec2(0, offsetAmount/ 2), vec2(-offsetAmount / 2, 0), 
vec2(0, -offsetAmount/ 2)};

const vec2 Poisson25[25] = vec2[](
    vec2(-0.978698, -0.0884121),
    vec2(-0.841121, 0.521165),
    vec2(-0.71746, -0.50322),
    vec2(-0.702933, 0.903134),
    vec2(-0.663198, 0.15482),
    vec2(-0.495102, -0.232887),
    vec2(-0.364238, -0.961791),
    vec2(-0.345866, -0.564379),
    vec2(-0.325663, 0.64037),
    vec2(-0.182714, 0.321329),
    vec2(-0.142613, -0.0227363),
    vec2(-0.0564287, -0.36729),
    vec2(-0.0185858, 0.918882),
    vec2(0.0381787, -0.728996),
    vec2(0.16599, 0.093112),
    vec2(0.253639, 0.719535),
    vec2(0.369549, -0.655019),
    vec2(0.423627, 0.429975),
    vec2(0.530747, -0.364971),
    vec2(0.566027, -0.940489),
    vec2(0.639332, 0.0284127),
    vec2(0.652089, 0.669668),
    vec2(0.773797, 0.345012),
    vec2(0.968871, 0.840449),
    vec2(0.991882, -0.657338));


int samples = 17;

float random(float x){
       return 0.5 - fract(sin(x) * 23363.14);
}

float random(vec2 v){
       return 0.5 - fract(sin(dot(v.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio   

float random_g(vec2 xy){
       return 0.5 - fract(sin(distance(xy*PHI, xy))*xy.x* 33141.1434135135);
}

float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 )
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = 0.0;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc)
{
	if (sc.x > 1.0 || sc.y > 1.0 || sc.z > 1.0 || sc.x < 0.0 || sc.y < 0.0 || sc.z < 0.0)
		return 1.0;
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.1;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);
	float shadowFactor = 0.0;
	int count = 0;
	float range = 1.5;
	for (float x = -range; x <= range; x+= 1.0)
	{
		for (float y = -range; y <= range; y+= 1.0)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	}
	return shadowFactor / 16.0;
}

float search_region_radius_uv(float z)
{
    return shadowUBO.blockerScale * inLightSize * (z - inLightClippingPlanes.x) / z;
}

float search_region_radius(float z)
{
    return shadowUBO.blockerScale * inLightSize * (z - inLightClippingPlanes.x) / z;
}

float penumbra_radius_uv(float zReceiver, float zBlocker)
{
    return abs(zReceiver - zBlocker) / zBlocker;
}

float project_to_light_uv(float penumbra_radius, float z)
{
    return penumbra_radius * inLightSize * inLightClippingPlanes.x / z;
}

float z_clip_to_eye(float z)
{
    return inLightClippingPlanes.x + (inLightClippingPlanes.y - inLightClippingPlanes.x) * z;
    //return inLightClippingPlanes.y * inLightClippingPlanes.x / (inLightClippingPlanes.y - z * (inLightClippingPlanes.y - inLightClippingPlanes.x));
}

void findBlockers(out float accum_blocker_depth,
                  out int num_blockers,
                  vec2      uv,
                  float     z0,
				  float     bias,
                  float     search_region_radius_uv)
{
	accum_blocker_depth = 0.0;
    num_blockers        = 0;
    float biased_depth  = z0 - bias;

    for (int i = 0; i < samples; ++i)
    {
        vec2 offset = offsets[i];

        offset *= search_region_radius_uv;
        float shadow_map_depth = texture(shadowMap, uv + offset).r;

        if (shadow_map_depth < biased_depth)
        {
            accum_blocker_depth += shadow_map_depth;
            num_blockers++;
        }
    }
}


float searchArea(float lightSize, float receiver)
{
	return shadowUBO.blockerScale * lightSize * (receiver - inLightClippingPlanes.x) / receiver;
}

float averageBlockerDepth(vec3 shadowCoords, float lightSize, float bias)
{

	int blockerCount = 0;
	float averageBlockerDepth = 0.0;
	float searchArea = searchArea(lightSize, shadowCoords.z);
	float biasDepth = shadowCoords.z - bias;

    for (int i = 0; i < samples; ++i)
    {
        vec2 offset = offsets[i];

        offset *= searchArea;
        float sampledDepth = texture(shadowMap, shadowCoords.xy + offset).r;

        if (sampledDepth < biasDepth)
        {
            averageBlockerDepth += sampledDepth;
            blockerCount++;
        }
    }


	if(blockerCount > 0)
		return averageBlockerDepth / float(blockerCount);

	return -1.0;
}

float pcf(vec2 uv, float z0, float bias, float filter_radius_uv)
{
    float sum = 0.0;

    for (int i = 0; i < samples; ++i)
    {
        vec2 offset = offsets[i];

        offset *= filter_radius_uv;
        float shadow_map_depth = texture(shadowMap, uv + offset).r;
        sum += shadow_map_depth < (z0 - bias) ? 0.0 : 1.0;
    }

    return sum / float(samples);
}

float PCSS_f(vec3 sc, vec3 shadowViewCoord)
{
	sc = sc * 0.5 + 0.5;
    float current_depth = sc.z;

	float bias = shadowUBO.bias;

	float z_vs = -(shadowViewCoord.z);
	float search_region_radius = search_region_radius(sc.z);
	float accum_blocker_depth;
	int num_blockers;
	findBlockers(accum_blocker_depth, num_blockers, sc.xy, sc.z, bias, search_region_radius);
	if (num_blockers < 1)
    {
        return 1.0;
    }

	float avg_blocker_depth = accum_blocker_depth / float(num_blockers);
    float avg_blocker_depth_vs = z_clip_to_eye(avg_blocker_depth);
	float penumbra_radius = penumbra_radius_uv(sc.z, avg_blocker_depth_vs);
	float filter_radius = project_to_light_uv(penumbra_radius, sc.z);

	if (sc.x > 1.0 || sc.y > 1.0 || sc.z > 1.0 || sc.x < 0.0 || sc.y < 0.0 || sc.z < 0.0)
		return 1.0;

    return pcf(sc.xy, sc.z, bias, filter_radius);
}

float pcss_f(vec3 shadowCoords)
{
	vec3 normalisedShadowCoords = shadowCoords * 0.5 + 0.5;

	if (normalisedShadowCoords.x > 1.0 || normalisedShadowCoords.y > 1.0 || normalisedShadowCoords.z > 1.0 || 
	normalisedShadowCoords.x < 0.0 || normalisedShadowCoords.y < 0.0 || normalisedShadowCoords.z < 0.0)
		return 1.0;

	float bias = shadowUBO.bias;

	return averageBlockerDepth(normalisedShadowCoords, inLightSize, bias);
}

void main() {

	vec4 col = texture(sampledTexture, inUV);
	if(col.a < 0.9)
		discard;

	float shadow = 0.0;
	//shadow = filterPCF(inShadowCoord / inShadowCoord.w);
	shadow = pcss_f(inShadowCoord.xyz / inShadowCoord.w);
	vec4 outColour;

	outColour = vec4(vec3(shadow), 1.0) * col;

	// vec3 normal = normalize(inNormal);
	// vec3 viewVec = fragVert - inCamPos;
	// vec3 viewDir = normalize(fragVert - inCamPos);
	// vec3 invViewDir = normalize(inCamPos - fragVert);

	// vec3 totalLight = vec3(0.0);
	// vec3 lightPos = inLightPos;
	// vec3 lightVec = fragVert - lightPos;
	// //vec3 lightDir = normalize(fragVert - lightPos);
	// vec3 invLightDir = -normalize(inLightDirection);
	// //vec3 invLightDir = -normalize(lightPos - fragVert);
	// totalLight = shadingGGX(normal, invViewDir, invLightDir, vec3(0.5), 0.8, 0.05);
	// outColour = col * vec4(totalLight, 1.0);
	// outColour *= shadow;


	// if(shadow < 1.0 && distance(inCamPos, fragVert) < inCull)
	// {
	// 	rayQueryEXT rayQuery;
	// 	vec4 tempColour = vec4(0.0);
	// 	bool firstTime = true;
	// 	float samples = shadow;
	// 	float ran = random(gl_FragCoord.xy/ vec2(800, 600) * samples);

	// 	for (int i = 0; i < max(int(samples * 16.0), 1); i++)
	// 	{

	// 		// initialise the ray to query but doesn't start the traversal
	// 		vec3 offset = vec3(-ran, 0, ran) * shadowUBO.alphaThreshold;
	// 		vec3 origin = firstTime ? fragVert : fragVert + offset;
	// 		ran  = random(ran);
	// 		firstTime = false;
	// 		rayQueryInitializeEXT(rayQuery, 
	// 		topLevelAS, 
	// 		gl_RayFlagsNoOpaqueEXT, 
	// 		0xFF, 
	// 		origin, 
	// 		0.01, 
	// 		normalize(lightPos - origin),
	// 		1000.0);

	// 		float storedDistance = 1001.0;
	// 		float storedAlpha = 0.0;
	// 		int hitCount = 0;
	// 		// Start the ray traversal, rayQueryProceedEXT returns false if the traversal is complete
	// 		while (rayQueryProceedEXT(rayQuery)) 
	// 		{ 
	// 			uint candidateType = rayQueryGetIntersectionTypeEXT(rayQuery, false);

	// 			if (candidateType == gl_RayQueryCandidateIntersectionTriangleEXT) 
	// 			{
	// 				hitCount++;
	// 				if(shadowUBO.terminateRay == 1)
	// 					rayQueryTerminateEXT(rayQuery);
	// 				if(shadowUBO.confirmIntersection == 1)	
	// 					rayQueryConfirmIntersectionEXT(rayQuery);

	// 				int objIndex = rayQueryGetIntersectionInstanceIdEXT(rayQuery, false);
	// 				ObjDesc objResource = objDesc.i[objIndex];
	// 				Indices indices = Indices(objResource.indicesAddress);
	// 				Vertices vertices = Vertices(objResource.vertexAddress);
	// 				// Indices of the triangle
	// 				ivec3 ind = indices.i[rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false)];

	// 				// // Vertex of the triangle
	// 				Vertex v0 = vertices.v[ind.y];
	// 				Vertex v1 = vertices.v[ind.z];
	// 				Vertex v2 = vertices.v[ind.x];
	// 				vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
	// 				vec3 barycentrics = vec3(attribs.x, attribs.y, 1.0 - attribs.y - attribs.x);
	// 				vec2 texCoord = v0.uvCoord * barycentrics.x + v1.uvCoord * barycentrics.y + v2.uvCoord * barycentrics.z;
	// 				float alpha = texture(textureSamplers[objResource.textureIndex], texCoord).a;
	// 				if(alpha >= 1.0){
	// 					storedAlpha = alpha;
	// 					rayQueryTerminateEXT(rayQuery);
	// 				}
	// 				else{
	// 					storedAlpha += alpha;
	// 				}
  
	// 			}
	// 		}
	// 		tempColour += outColour * (1.0 - clamp(storedAlpha, 0.0, 1.0));
	// 		// if(storedAlpha > shadowUBO.alphaThreshold){
	// 		// 	tempColour += outColour * (1.0 - clamp(storedAlpha, 0.0, 1.0));
	// 		// }
	// 	}
	// 	outColour = tempColour / max((samples * 16.0), 1.0);
	// 	// if(samples > 0.05)
	// 	// 	outColour.r = samples;
	// 	// if(samples > 0.45)
	// 	// 	outColour.g = samples;
	// 	// if(samples > 0.75)
	// 	// 	outColour.b = samples;
	// }
	// else
	// {
	// 	outColour *= shadow;
	// }

	// if(shadow < 1.0 && distance(inCamPos, fragVert) < inCull)
	// {
	// 	rayQueryEXT rayQuery;

	// 	// initialise the ray to query but doesn't start the traversal
	// 	rayQueryInitializeEXT(rayQuery, 
	// 	topLevelAS, 
	// 	gl_RayFlagsNoOpaqueEXT, 
	// 	0xFF, 
	// 	fragVert, 
	// 	0.01, 
	// 	invLightDir, 
	// 	1000.0);

	// 	float storedDistance = 1001.0;
	// 	float storedAlpha = 0.0;
	// 	int hitCount = 0;

	// 	// Start the ray traversal, rayQueryProceedEXT returns false if the traversal is complete
	// 	while (rayQueryProceedEXT(rayQuery)) 
	// 	{ 
	// 		uint candidateType = rayQueryGetIntersectionTypeEXT(rayQuery, false);

	// 		if (candidateType == gl_RayQueryCandidateIntersectionTriangleEXT) 
	// 		{
	// 			if(shadowUBO.terminateRay == 1)
	// 				rayQueryTerminateEXT(rayQuery);
	// 			if(shadowUBO.confirmIntersection == 1)	
	// 				rayQueryConfirmIntersectionEXT(rayQuery);

	// 			int objIndex = rayQueryGetIntersectionInstanceIdEXT(rayQuery, false);
	// 			ObjDesc objResource = objDesc.i[objIndex];
	// 			Indices indices = Indices(objResource.indicesAddress);
	// 			Vertices vertices = Vertices(objResource.vertexAddress);
	// 			// Indices of the triangle
	// 			ivec3 ind = indices.i[rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false)];

	// 			// // Vertex of the triangle
	// 			Vertex v0 = vertices.v[ind.y];
	// 			Vertex v1 = vertices.v[ind.z];
	// 			Vertex v2 = vertices.v[ind.x];
	// 			vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
	// 			vec3 barycentrics = vec3(attribs.x, attribs.y, 1.0 - attribs.y - attribs.x);
	// 			vec2 texCoord = v0.uvCoord * barycentrics.x + v1.uvCoord * barycentrics.y + v2.uvCoord * barycentrics.z;
	// 			float alpha = texture(textureSamplers[objResource.textureIndex], texCoord).a;
	// 			if(alpha >= 1.0){
	// 				storedAlpha = alpha;
	// 				rayQueryTerminateEXT(rayQuery);
	// 				break;
	// 			}
				
	// 			float d = rayQueryGetIntersectionTEXT(rayQuery, false);
	// 			storedAlpha += alpha;
	// 		}
	// 	}

	// 	if(storedAlpha > shadowUBO.alphaThreshold){
	// 		outColour *= 1.0 - storedAlpha;
	// 	}
	// }
	// else
	// {
	// 	outColour *= shadow;
	// }

	outFragColor = outColour;
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