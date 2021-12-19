#version 460

#extension GL_EXT_ray_query : enable
#extension GL_EXT_ray_flags_primitive_culling : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(primitive_culling);

layout (set = 1, binding = 1) uniform sampler2D sampledTexture;

layout (set = 3, binding = 0) uniform sampler2D shadowMap;
layout (set = 3, binding = 1) uniform ShadowUBO
{
	ivec4 shadow;
	vec4 blocker;
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
layout (location = 6) in vec3 inWorldPos;
layout (location = 7) in float inCull;
layout (location = 8) in vec3 inLightDirection;
layout (location = 9) in vec2 inLightClippingPlanes;
layout (location = 10) in vec2 inLightSize;
layout (location = 11) in flat int inLightType;

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

//////////////////////////////////////////////////////////////

/// Spatial - stack overflow
// https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}



// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }



// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}



// Pseudo-random value in half-open range [0:1].
float random_p( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random_p( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random_p( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random_p( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))) - 0.5; }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))) - 0.5; }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))) - 0.5; }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))) - 0.5; }


//////////////////////////////////////////////////////////////

vec2 discSample(float size, vec2 ran){ 
	float r = size * sqrt(ran.x);
	float theta = ran.y * 2.0 * 3.14159265359;
	return vec2(r * cos(theta), r * sin(theta));
}

float sigmoidFunction(float x)
{
	return 1.0 / (1.0 + exp(-x));
}

float searchArea(float lightSize, float receiver)
{
	return (inLightType == 0 ? lightSize : lightSize  / inLightSize.y) * 
		(receiver - inLightClippingPlanes.x) / receiver;
}

float averageBlockerDepth(out int blockerCount, vec2 uv, vec3 shadowCoords, float lightSize, float bias)
{

	blockerCount = 0;
	float averageBlockerDepth = 0.0;
	float searchArea = shadowUBO.blocker.x * (lightSize / shadowCoords.z) * (shadowCoords.z - inLightClippingPlanes.x);
	float biasDepth = shadowCoords.z;

	vec2 ran = vec2(random_p(gl_FragCoord.xy), random_p(gl_FragCoord.yx * shadowUBO.shadow.w));
	vec2 offset = ran;

    for (int i = 0; i < shadowUBO.shadow.z; ++i)
    {
		vec2 offset = discSample(inLightSize.x, ran);
        float sampledDepth = texture(shadowMap, uv + offset * searchArea).r;
        sampledDepth = sampledDepth * 0.5 + 0.5;
        if (sampledDepth + bias < biasDepth)
        {
            averageBlockerDepth += sampledDepth;
            blockerCount++;
        }

		ran = vec2(random_p(ran.x), random_p(ran.y));
    }
	if (blockerCount > 0)
		return averageBlockerDepth / float(blockerCount);

	return 1.0;
}

float penumbraSize(float averageBlockerDepth, float receiver, float lightSize, float bias)
{
	return (lightSize * (receiver - averageBlockerDepth)) / averageBlockerDepth;
}

float pcf(vec2 uv, float z0, float bias, float penumbraSize)
{
    float sum = 0.0;
	vec2 ran = vec2(random_p(gl_FragCoord.xy * shadowUBO.shadow.w), random_p(gl_FragCoord.yx));	
	vec2 offset = ran;
    for (int i = 0; i < shadowUBO.shadow.z; ++i)
    {
		vec2 offset = discSample(penumbraSize, ran);
		float z = texture(shadowMap, uv + offset).r;
		z = z * 0.5 + 0.5;
		sum += (z <= z0) ? 1 : 0;

		ran = vec2(random_p(ran.x), random_p(ran.y));
	}
	return sum / shadowUBO.shadow.z;
}

float pcss_f(vec3 shadowCoords, out float diff)
{
	vec3 normalisedShadowCoords = shadowCoords * 0.5 + 0.5;

	if (normalisedShadowCoords.x > 1.0 || normalisedShadowCoords.y > 1.0 || normalisedShadowCoords.z > 1.0 || 
	normalisedShadowCoords.x < 0.0 || normalisedShadowCoords.y < 0.0 || normalisedShadowCoords.z < 0.0)
		return 1.0;

	int blockerCount;
	float averageBlockerDepth = averageBlockerDepth(blockerCount, normalisedShadowCoords.xy, normalisedShadowCoords, inLightSize.x * 0.5, shadowUBO.blocker.w);
	if(blockerCount <= 0) return 1.0;
	float penumbraSize = penumbraSize(averageBlockerDepth, normalisedShadowCoords.z, inLightSize.x * 0.25, shadowUBO.blocker.w);
	float uvRadius = penumbraSize;

	diff = penumbraSize;
	return max(1.0 - pcf(normalisedShadowCoords.xy, normalisedShadowCoords.z, averageBlockerDepth, uvRadius), 0.0);
}

void main() {

	
	vec4 col = texture(sampledTexture, inUV);
	if(col.a < 0.9)
		discard;

	float shadow = 0.0, diff = 0.0;
	int exit = 0;
	shadow = pcss_f(inShadowCoord.xyz / inShadowCoord.w, diff);
	vec3 outColour;

	//outColour = vec4(vec3(shadow), 1.0) * col;

	vec3 normal = normalize(inNormal);
	vec3 viewVec = inWorldPos - inCamPos;
	vec3 viewDir = normalize(inWorldPos - inCamPos);
	vec3 invViewDir = normalize(inCamPos - inWorldPos);

	vec3 totalLight = vec3(0.0);
	vec3 lightPos = inLightPos;

	float threshold = 1.0;
	float dist = distance(inCamPos, inWorldPos);
	if(dist < inCull + (threshold) && shadow < 1.0)
	{
		rayQueryEXT rayQuery;
		int samples;
		if(shadowUBO.shadow.y == 1)
			samples = max(int(shadow *  float(shadowUBO.shadow.w)), 4);
		else
			samples = int(shadowUBO.shadow.w);
		
		//float shadow.z = float(shadowUBO.shadow.w);
		vec2 ran = vec2(random_p(gl_FragCoord.xy), random_p(gl_FragCoord.xz * shadowUBO.shadow.w));
	
		//outColour = vec4(1.0 - shadow, 1.0 - shadow, 1.0 - shadow, 1.0);

		float storedShadow = 0.0;
		vec3 storedColour = vec3(0.0);
		vec3 origin = inWorldPos;
		int hitShit = 0;
		for (int i = 0; i < int(samples); i++)
		{
			// initialise the ray to query but doesn't start the traversal
			vec2 offset = discSample(inLightSize.x, ran);
			//vec2 offset = vec2(random(ran.y), random(ran.x)) * lightRadius;
			vec3 target = (inLightType == 0 ? lightPos : -inLightDirection) + vec3(offset, 0.0) * shadowUBO.blocker.y;
			ran  = vec2(random_p(ran.x), random_p(ran.y));
			rayQueryInitializeEXT(rayQuery, 
			topLevelAS, 
			gl_RayFlagsNoOpaqueEXT, 
			0xFF, 
			origin, 
			0.01, 
			inLightType == 0 ? normalize(target - origin) : normalize(target),
			1000.0);

			float hit = 1.0;
			// Start the ray traversal, rayQueryProceedEXT returns false if the traversal is complete
			while (rayQueryProceedEXT(rayQuery)) 
			{ 
				uint candidateType = rayQueryGetIntersectionTypeEXT(rayQuery, false);

				if (candidateType == gl_RayQueryCandidateIntersectionTriangleEXT) 
				{	
					if (rayQueryGetIntersectionFrontFaceEXT(rayQuery, true))
					{	
						int objIndex = rayQueryGetIntersectionInstanceIdEXT(rayQuery, false);
						ObjDesc objResource = objDesc.i[objIndex];
						Indices indices = Indices(objResource.indicesAddress);
						Vertices vertices = Vertices(objResource.vertexAddress);
						// Indices of the triangle
						ivec3 ind = indices.i[rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false)];

						// // Vertex of the triangle
						Vertex v0 = vertices.v[ind.y];
						Vertex v1 = vertices.v[ind.z];
						Vertex v2 = vertices.v[ind.x];
						vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
						vec3 barycentrics = vec3(attribs.x, attribs.y, 1.0 - attribs.y - attribs.x);
						vec2 texCoord = v0.uvCoord * barycentrics.x + 
										v1.uvCoord * barycentrics.y + 
										v2.uvCoord * barycentrics.z;
						float alpha = texture(textureSamplers[objResource.textureIndex], texCoord).a;
						if(alpha >= 0.9){
							hit = 0.0;
							hitShit = 1;
							//if(shadowUBO.shadow.y == 1)
								rayQueryTerminateEXT(rayQuery);
						}
					}
				}
			}
			storedShadow += (shadowUBO.shadow.x == 1 ? hitShit == 0 ? (hit) : shadow : hit);
			//storedShadow += hit;
			vec3 invLightDir = (inLightType == 0 ? normalize(target - origin) : normalize(-inLightDirection));
			totalLight += shadingGGX(normal, invViewDir, invLightDir, vec3(0.5), 0.8, 0.05);
		}

		totalLight /= samples;
		//storedColour = col.xyz * (shadowUBO.shadow.x == 1 ? storedShadow : storedShadow);
		//totalLight = vec3(clamp(totalLight.x, 0.0, 1.0), clamp(totalLight.y, 0.0, 1.0), clamp(totalLight.z, 0.0, 1.0));
		storedColour = col.xyz * totalLight;
		//outColour = storedColour * (hitShit == 1 ? min((storedShadow / samples), 1.0) : 1.0);
		//outColour = storedColour * storedShadow;
		//distance(storedShadow, shadow);
		// float shadowProximity = 0.1;
		if(dist > inCull - threshold)
		{
			// sigmoid function
			float sigmoid = sigmoidFunction(dist - (inCull - threshold));
			outColour = storedColour * (mix(shadow, (hitShit == 1 ? min((storedShadow / samples), 1.0) : 1.0), sigmoid));
		}
		else
		{
			outColour = storedColour * (hitShit == 1 ? min((storedShadow / samples), 1.0) : 1.0);
		}
	}
	else
	{
		vec3 invLightDir = (inLightType == 0 ? normalize(lightPos - inWorldPos) : normalize(-inLightDirection));
		totalLight = shadingGGX(normal, invViewDir, invLightDir, vec3(0.5), 0.8, 0.05);
		outColour = col.xyz * totalLight * shadow;
	}

	outFragColor = vec4(outColour, 1.0);
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