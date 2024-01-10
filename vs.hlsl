#ifndef VS_HLSL
#define VS_HLSL

#include "cbuffers.hlsl"
#include "ps_input_output.hlsl"

struct vs_input
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

ps_input vs_main(vs_input input)
{
    ps_input output;

    float4 pos = float4(input.position, 1.0f);
    pos = mul(pos, model);
    output.pixelPosition = pos.xyz;

    pos = mul(pos, view);
    pos = mul(pos, projection);
    output.screenPosition = pos;

    output.interpolatedUV = input.uv;

    float4 normal = float4(input.normal, 1.0f);
    normal = mul(normal, model);
    output.interpolatedNormal = normal.xyz;
    return (output);
}

#endif