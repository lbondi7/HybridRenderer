#version 460

layout (set = 1, binding = 1) uniform sampler2D sampledTexture;

layout (set = 3, binding = 0) uniform sampler2D shadowMap;
layout (set = 3, binding = 1) uniform ShadowUBO
{
	ivec4 shadow;
	vec4 blocker;
	uvec4 extra;
} shadowUBO;

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
layout (location = 11) in vec4 inLightColour;
layout (location = 12) in flat int inLightType;

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

float averageBlockerDepth(out int blockerCount, vec2 uv, float depth, 
float lightSize, float bias)
{
	blockerCount = 0;
	float averageBlockerDepth = 0.0;
	float searchArea =  shadowUBO.blocker.x * searchArea(lightSize, depth);

	vec2 ran = vec2(random_p(gl_FragCoord.xy * uv), 
	random_p(gl_FragCoord.yx * shadowUBO.shadow.w));

    for (int i = 0; i < shadowUBO.shadow.z / 2; ++i)
    {
		vec2 offset = discSample(inLightSize.x, ran);

        float z = texture(shadowMap, uv + offset * searchArea).r;
        //z = z * 0.5 + 0.5;
        if (z + bias <= depth)
        {
            averageBlockerDepth += z;
            blockerCount++;
		}

		ran = vec2(random_p(ran.x), random_p(ran.y));
    }

	return averageBlockerDepth / float(blockerCount);
}

float penumbraSize(float averageBlockerDepth, float receiver, 
float lightSize)
{
	return shadowUBO.blocker.x * ((lightSize) * (receiver - averageBlockerDepth)) / averageBlockerDepth;
}

float pcf(vec2 uv, float avBlockerDepth, float bias, float penumbraSize)
{
    float sum = 0.0;
	int count = 0;
	vec2 ran = vec2(random_p(gl_FragCoord.xy * shadowUBO.shadow.w), 
			random_p(gl_FragCoord.yx * uv));	

    for (int i = 0; i < shadowUBO.shadow.z; ++i)
    {
		vec2 offset = discSample(inLightSize.x, ran);
		float z = texture(shadowMap, uv + (offset * penumbraSize)).r;

		sum += (z + bias <= avBlockerDepth) ? 0.0 : 1.0;
		count++;
		ran = vec2(random_p(ran.x), random_p(ran.y));

		if(i == 4 && (sum <= 0.0 || sum == 5.0))
		{
			break;
		}
	}
	return sum / float(count);
}

float pcss_f(vec3 shadowCoords, out float diff, out int blockerCount)
{
	vec3 normalisedShadowCoords = shadowCoords * 0.5 + 0.5;

	if (normalisedShadowCoords.x > 1.0 || normalisedShadowCoords.y > 1.0 || 
	normalisedShadowCoords.z > 1.0 || 
	normalisedShadowCoords.x < 0.0 || normalisedShadowCoords.y < 0.0 || 
	normalisedShadowCoords.z < 0.0)
		return (diff = 1.0);
	
	blockerCount = 0;
	vec2 uv = normalisedShadowCoords.xy;
	float depth = shadowCoords.z;
	float averageBlockerDepth = averageBlockerDepth(blockerCount, 
		uv, depth, inLightSize.x, shadowUBO.blocker.w);
	
	if(blockerCount <= 0)
		return (diff = 1.0);

	diff = depth - averageBlockerDepth;

	float penumbraSize = penumbraSize(averageBlockerDepth, 
		depth, inLightSize.x);

	return max(pcf(uv, depth, shadowUBO.blocker.z, penumbraSize), 0.0);
}
void main() {

	
	vec4 col = texture(sampledTexture, inUV);
	if(col.a < 0.9)
		discard;

	float shadow = 0.0, diff = 0.0;
	int blockerCount = 0;
	shadow = shadowUBO.shadow.x == 1 ? 1.0 : pcss_f(inShadowCoord.xyz/inShadowCoord.w, diff, blockerCount);
	vec3 outColour;
	vec3 totalLight = vec3(0.0);
	vec3 lightPos = inLightPos;
	vec3 invViewDir = normalize(inCamPos - inWorldPos);
	vec3 invLightDir = -normalize(inWorldPos - inLightPos);

	totalLight = shadingGGX(inNormal, -invViewDir, invLightDir, 
		inLightColour.xyz * inLightColour.w, 0.8, 0.05);

	outColour = col.xyz * totalLight * shadow; 

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