#pragma once


#include <string_view>


#if defined(USE_D3D12)
constexpr const char* g_pRGenShader =
R"raw(
struct RayPayload
{
    float3 hitValue;
    int    depth;
    float3 attenuation;
    int    done;
    float3 rayOrigin;
    float  u;
    float3 rayDir;
    float  v;
};

RWTexture2D<float4> outputImage : register(u0, space0);
RaytracingAccelerationStructure topLevelAS : register(t1, space0);

struct RayData
{
    row_major float4x4 viewInv;
    row_major float4x4 projInv;
};
ConstantBuffer<RayData> rayData : register(b2, space0);

[shader("raygeneration")]
void main()
{
    const uint2 launchID = DispatchRaysIndex().xy;
    const uint2 launchSize = DispatchRaysDimensions().xy;

    const float2 pixelCenter = float2(launchID) + float2(0.5f, 0.5f);
    const float2 inUV = pixelCenter / float2(launchSize);
    float2 d = inUV * 2.0f - 1.0f;
    d.y = -d.y; // Match D3D Y-up convention

    float4 origin = mul(float4(0.0f, 0.0f, 0.0f, 1.0f), rayData.viewInv);
    float4 target = mul(float4(d.x, d.y, 1.0f, 1.0f), rayData.projInv);
    
    // Clean perspective divide calculation matching previous matrix resolutions
    float3 viewDirection = normalize(target.xyz / target.w);
    float4 direction = mul(float4(viewDirection, 0.0f), rayData.viewInv);

    float tmin = 0.001f;
    float tmax = 10000.0f;

    RayPayload payload;
    payload.depth       = 0;
    payload.hitValue    = float3(0.0f, 0.0f, 0.0f);
    payload.attenuation = float3(1.0f, 1.0f, 1.0f);
    payload.done        = 1;
    payload.rayOrigin   = origin.xyz;
    payload.rayDir      = direction.xyz;
    payload.u           = d.x;
    payload.v           = d.y;

    float3 hitValue = float3(0.0f, 0.0f, 0.0f);
    
    // Explicit iterative ray bouncing loop execution
    [loop]
    for (;;)
    {
        float3 attenuation = payload.attenuation;
        
        RayDesc ray;
        ray.Origin    = origin.xyz;
        ray.TMin      = tmin;
        ray.Direction = normalize(direction.xyz);
        ray.TMax      = tmax;

        TraceRay(
            topLevelAS,
            RAY_FLAG_FORCE_OPAQUE,
            0xFF,
            0,
            0,
            0,
            ray,
            payload
        );
        
        hitValue += payload.hitValue * attenuation;

        payload.depth++;
        if (payload.done == 1 || payload.depth >= 10)
        {
            break;
        }

        origin.xyz    = payload.rayOrigin;
        direction.xyz = payload.rayDir;
        payload.done  = 1; // Default to terminal termination unless reset by reflection hit
    }

    outputImage[launchID] = float4(hitValue, 1.0f);
}
)raw";
constexpr auto g_RGenSize = std::char_traits<char>::length(g_pRGenShader);


constexpr const char* g_pRMissShader =
R"raw(
struct RayPayload
{
    float3 hitValue;
    int    depth;
    float3 attenuation;
    int    done;
    float3 rayOrigin;
    float  u;
    float3 rayDir;
    float  v;
};

cbuffer PC : register(b3)
{
	float3 pcColor;
};

float3 verticalGradient(float3 topColor, float uvY)
{
	return topColor * uvY;
}

[shader("miss")]
void main(inout RayPayload payload)
{
	float2 uv = payload.hitValue.xy;
	float3 color = verticalGradient(pcColor, 1.0f - uv.y);
	payload.hitValue = color;
}
)raw";
constexpr auto g_RMissSize = std::char_traits<char>::length(g_pRMissShader);


constexpr const char* g_pRCHitShader =
R"raw(
struct RayPayload
{
    float3 hitValue;
    int    depth;
    float3 attenuation;
    int    done;
    float3 rayOrigin;
    float  u;
    float3 rayDir;
    float  v;
};

struct ShadowPayload
{
    bool inShadow;
};

struct Vertex
{
    float3 pos;
    float2 uv;
    float3 normal;
};

RaytracingAccelerationStructure topLevelAS : register(t1, space0);

StructuredBuffer<Vertex> vertices : register(t0, space1);
StructuredBuffer<uint3> indices : register(t1, space1); // Uses uint3 for grouped triangle indices mapping uvec3

cbuffer HitData : register(b2, space1)
{
    row_major float4x4 modelMatrix[3];
    float3 lightDir;
};

Vertex GetVertex(uint index, uint vertexOffset)
{
    return vertices[index + vertexOffset];
}

uint3 GetIndices(uint primIndex, uint indexOffset)
{
    uint basePrimIndex = indexOffset / 3;
    return indices[basePrimIndex + primIndex];
}

[shader("closesthit")]
void main(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const uint primIndex = PrimitiveIndex();
    const uint instanceId = InstanceID();

    // Precompute the per-instance offsets only once
    uint vertexOffset = 0;
    uint indexOffset = 0;
    if (instanceId > 0)
    {
        vertexOffset = 24; // 24 vertices for the grid
        indexOffset = 36;  // 36 indices for the grid
    }

    const uint3 ind = GetIndices(primIndex, indexOffset);

    const Vertex v0 = GetVertex(ind.x, vertexOffset);
    const Vertex v1 = GetVertex(ind.y, vertexOffset);
    const Vertex v2 = GetVertex(ind.z, vertexOffset);

    const float3 barycentrics = float3(1.0f - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    float3 normal = normalize(v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z);
    normal = normalize(mul(float4(normal, 0.0f), modelMatrix[instanceId]).xyz);
    
    float3 worldPos = mul(float4(v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z, 1.0f), modelMatrix[instanceId]).xyz;

    float3 baseColor = float3(0.0f, 0.0f, 0.0f);
    float attenuation = 1.0f;

    float3 lDir = normalize(-lightDir);
    float NdotL = dot(normal, lDir);

    if (NdotL > 0.0f)
    {
        // Compute world-space hit position
        float3 rayOrigin = (WorldRayOrigin() + RayTCurrent() * WorldRayDirection()) + normal * 0.001f;
        float3 rayDirection = lDir;
        uint rayFlags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;

        ShadowPayload shadowPayload;
        shadowPayload.inShadow = true; // Assume shadowed

        RayDesc shadowRay;
        shadowRay.Origin    = rayOrigin;
        shadowRay.TMin      = 0.001f;
        shadowRay.Direction = rayDirection;
        shadowRay.TMax      = 10000.0f;

        TraceRay(
            topLevelAS,
            rayFlags,
            0xFF,
            0, // Shadow ray SBT offset
            0,
            1, // Miss index for shadow rays
            shadowRay,
            shadowPayload
        );
	
        if (shadowPayload.inShadow)
        {
            attenuation = 0.3f; // Darken if in shadow
        }
        else
        {
            baseColor = float3(NdotL, NdotL, NdotL);
        }
    }

    // Reflection Logic Pass
    if (instanceId == 2)
    {
        float3 origin = worldPos;
        float3 rayDir = reflect(WorldRayDirection(), normal);
        payload.attenuation *= 0.7f;
        payload.done        = 0;
        payload.rayOrigin   = origin;
        payload.rayDir      = rayDir;
    }
    else
    {
        payload.hitValue = baseColor * attenuation;
    }
}
)raw";
constexpr auto g_RCHitSize = std::char_traits<char>::length(g_pRCHitShader);


constexpr const char* g_pRMissShadowShader =
R"raw(
struct ShadowPayload {
    bool inShadow;
};

[shader("miss")]
void main(inout ShadowPayload payload)
{
    // The shadow ray missed all geometry, meaning this pixel position is lit!
    payload.inShadow = false;
}
)raw";
constexpr auto g_RMissShadowSize = std::char_traits<char>::length(g_pRMissShadowShader);


constexpr const char* g_pVSShader =
R"raw(
struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

VSOutput main(uint vertexID : SV_VertexID)
{
    // Local lookups matching your hardcoded GLSL arrays
    const float2 vertices[6] = 
    {
        float2(-1.0f,  1.0f),
        float2( 1.0f,  1.0f),
        float2( 1.0f, -1.0f),
        float2(-1.0f,  1.0f),
        float2( 1.0f, -1.0f),
        float2(-1.0f, -1.0f)
    };

    const float2 uvs[6] = 
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 1.0f)
    };

    VSOutput output;
    output.pos = float4(vertices[vertexID], 0.0f, 1.0f);
    output.uv = uvs[vertexID];
    return output;
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);

constexpr const char* g_pPSShader =
R"raw(
struct PSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

Texture2D samplerTexture : register(t0);
SamplerState samplerObj : register(s1);

float4 main(PSInput input) : SV_Target
{
    return samplerTexture.Sample(samplerObj, input.uv);
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#else
constexpr const char* g_pRGenShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload
{
	vec3 hitValue;
	int  depth;
	vec3 attenuation;
	int  done;
	vec3 rayOrigin;
	float u;
	vec3 rayDir;
	float v;
};

layout(binding = 0, rgba32f) uniform image2D outputImage;
layout(binding = 1) uniform accelerationStructureEXT topLevelAS;
layout(binding = 2) uniform RayData
{
	mat4 viewInv;
	mat4 projInv;
} rayData;

layout(location = 0) rayPayloadEXT RayPayload payload;

void main()
{
	const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
	const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeEXT.xy);
	vec2 d = inUV * 2.0 - 1.0;

	vec4 origin = rayData.viewInv * vec4(0,0,0,1);
	vec4 target = rayData.projInv * vec4(d.x, d.y, 1, 1);
	vec4 direction = rayData.viewInv * vec4(normalize(target.xyz), 0);

	float tmin = 0.001;
	float tmax = 10000.0;

	payload.depth       = 0;
	payload.hitValue    = vec3(0);
	payload.attenuation = vec3(1.f, 1.f, 1.f);
	payload.done        = 1;
	payload.rayOrigin   = origin.xyz;
	payload.rayDir      = direction.xyz;
	payload.u			= d.x;
	payload.v			= d.y;

	vec3 hitValue = vec3(0.0);
	for (;;)
	{
		vec3 attenuation = payload.attenuation; // Attenuation will be applied on bouncing rays.
		traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, origin.xyz, tmin, direction.xyz, tmax, 0);
		hitValue += payload.hitValue * attenuation;

		payload.depth++;
		if(payload.done == 1 || payload.depth >= 10)
		  break;

		origin.xyz    = payload.rayOrigin;
		direction.xyz = payload.rayDir;
		payload.done      = 1;  // Will stop if a reflective material isn't hit
	}


    imageStore(outputImage, ivec2(gl_LaunchIDEXT.xy), vec4(hitValue, 1.0));
}
)raw";
constexpr auto g_RGenSize = std::char_traits<char>::length(g_pRGenShader);


constexpr const char* g_pRMissShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload
{
	vec3 hitValue;
	int  depth;
	vec3 attenuation;
	int  done;
	vec3 rayOrigin;
	float u;
	vec3 rayDir;
	float v;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

layout(push_constant) uniform PC
{
	vec3 color;
} pc;

vec3 verticalGradient(vec3 topColor, float uvY)
{
    return topColor * uvY;
}

void main()
{
	vec2 uv = vec2(payload.u, payload.v);
	vec3 color = verticalGradient(pc.color, 1.0 - uv.y);
    payload.hitValue = color;
}
)raw";
constexpr auto g_RMissSize = std::char_traits<char>::length(g_pRMissShader);


constexpr const char* g_pRCHitShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

struct RayPayload
{
	vec3 hitValue;
	int  depth;
	vec3 attenuation;
	int  done;
	vec3 rayOrigin;
	float u;
	vec3 rayDir;
	float v;
};

struct ShadowPayload
{
	bool inShadow;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;
layout(location = 1) rayPayloadEXT ShadowPayload shadowPayload;

hitAttributeEXT vec2 attribs;

struct Vertex
{
    vec3 pos;
	vec2 uv;
    vec3 normal;
};

layout(set = 0, binding = 1) uniform accelerationStructureEXT topLevelAS;

layout(set = 1, binding = 0) readonly buffer VertexBuffer
{
    vec4 vertices[];
};

layout(set = 1, binding = 1) readonly buffer IndexBuffer
{
    uint indices[];
};

layout(set = 1, binding = 2) uniform HitData
{
	mat4 modelMatrix[3];
	vec3 lightDir;
};

Vertex GetVertex(uint index, uint vertexOffset)
{
    uint baseIndex = 2 * (index + vertexOffset);
    Vertex v;
    v.pos = vertices[baseIndex].xyz;
    v.uv = vec2(vertices[baseIndex].w, vertices[baseIndex + 1].x);
    v.normal = vertices[baseIndex + 1].yzw;
    return v;
}

uint GetIndex(uint primIndex, uint vertexInTri, uint indexOffset)
{
    return indices[indexOffset + primIndex * 3 + vertexInTri];
}

void main()
{
    const uint primIndex = gl_PrimitiveID;
    const uint instanceId = gl_InstanceCustomIndexEXT;

    // Precompute the per-instance offsets only once
    uint vertexOffset = 0;
    uint indexOffset = 0;
    if (instanceId > 0)
    {
        vertexOffset = 24; // 24 vertices for the grid
        indexOffset = 36;  // 36 indices for the grid
    }

    const uint index0 = GetIndex(primIndex, 0, indexOffset);
    const uint index1 = GetIndex(primIndex, 1, indexOffset);
    const uint index2 = GetIndex(primIndex, 2, indexOffset);

    const Vertex v0 = GetVertex(index0, vertexOffset);
    const Vertex v1 = GetVertex(index1, vertexOffset);
    const Vertex v2 = GetVertex(index2, vertexOffset);

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 normal = normalize(v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z);
	
    normal = normalize((modelMatrix[instanceId] * vec4(normal, 0.0)).xyz); // Transform to world space
	vec3 worldPos = (modelMatrix[instanceId] * vec4(v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z, 1.0)).xyz;

    vec3 baseColor = vec3(0.0);
	float attenuation = 1.0;

	vec3 lDir = normalize(-lightDir);
	float NdotL = dot(normal, lDir);

	//if (instanceId < 2)
	{
		if (NdotL > 0.0)
		{
			// Compute world-space hit position
			vec3 rayOrigin = (gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT) + normal * 0.001; // Offset a little to avoid self-hit
			vec3 rayDirection = lDir;   // Towards light (negate because lightDir points TO surface)
			uint flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;

			shadowPayload.inShadow = true; // Assume shadowed
			traceRayEXT(
				topLevelAS, // (you must declare this somewhere, usually a uniform acceleration structure)
				flags,
				0xFF,       // cull mask
				0, 0, 1,    // SBT record offsets for shadow rays
				rayOrigin,
				0.001,      // min t
				rayDirection,
				10000.0,     // max t
				1           // payload location
			);
		
			if (shadowPayload.inShadow)
			{
				attenuation = 0.3; // Darken if in shadow
			}
			else
			{
				baseColor = vec3(NdotL);
			}
		}
	}

	// Reflection
	if(instanceId == 2)
	{
		vec3 origin = worldPos;
		vec3 rayDir = reflect(gl_WorldRayDirectionEXT, normal);
		payload.attenuation *= 0.7;
		payload.done      = 0;
		payload.rayOrigin = origin;
		payload.rayDir    = rayDir;
	}
	//else
	{
		payload.hitValue = baseColor * attenuation;
	}
}
)raw";
constexpr auto g_RCHitSize = std::char_traits<char>::length(g_pRCHitShader);


constexpr const char* g_pRMissShadowShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

struct ShadowPayload
{
	bool inShadow;
};

layout(location = 1) rayPayloadInEXT ShadowPayload payload;

void main()
{
	payload.inShadow = false;
}
)raw";
constexpr auto g_RMissShadowSize = std::char_traits<char>::length(g_pRMissShadowShader);


constexpr const char* g_pVSShader =
R"raw(
#version 450

layout (location = 0) out vec2 outUV;

vec2 vertices[] = 
{
	vec2(-1.0f,  1.0f),
	vec2( 1.0f,  1.0f),
	vec2( 1.0f, -1.0f),
	vec2(-1.0f,  1.0f),
	vec2( 1.0f, -1.0f),
	vec2(-1.0f, -1.0f)
};

vec2 uvs[] = 
{
	vec2(0.0f, 0.0f),
	vec2(1.0f, 0.0f),
	vec2(1.0f, 1.0f),
	vec2(0.0f, 0.0f),
	vec2(1.0f, 1.0f),
	vec2(0.0f, 1.0f)
};

void main()
{
	gl_Position = vec4(vertices[gl_VertexIndex], 0.0f, 1.0f);
	outUV = uvs[gl_VertexIndex];
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);

constexpr const char* g_pPSShader =
R"raw(
#version 450

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform texture2D samplerTexture;
layout (binding = 1) uniform sampler samplerObj;

void main()
{
	outColor = texture(sampler2D(samplerTexture, samplerObj), inUV);
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#endif