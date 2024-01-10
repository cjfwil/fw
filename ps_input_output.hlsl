#ifndef PS_INPUT_OUTPUT_HLSL
#define PS_INPUT_OUTPUT_HLSL

struct ps_input
{
    float4 screenPosition : SV_Position;
    float3 pixelPosition : POSITION;
    float2 interpolatedUV : TEXCOORD;
    float3 interpolatedNormal : NORMAL;
};

struct ps_output
{
    float4 pixelColour : SV_Target;
};

#endif