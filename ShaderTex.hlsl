cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix mWorld;     // world matrix for object
    matrix View;       // view matrix
    matrix Projection; // projection matrix
};

struct VS_INPUT
{
    float3 vPos : POSITION;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION; // interpolated vertex position (system
    float4 Color : COLOR0;         // interpolated diffuse color
};

struct PS_OUTPUT
{
    float4 RGBColor : SV_TARGET;
};

PS_INPUT vs_main(VS_INPUT input) // main is the default function name
{
    PS_INPUT Output;
    float4 pos = float4(input.vPos, 1.0f);
    // Transform the position from object space to homogeneous projection space
    pos = mul(pos, mWorld);
    pos = mul(pos, View);
    pos = mul(pos, Projection);
    Output.Position = pos;
    // Just pass through the color data
    Output.Color = float4(input.uv, 1.0f, 1.0f);
    return Output;
}

PS_OUTPUT ps_main(PS_INPUT In)
{
    PS_OUTPUT Output;
    Output.RGBColor = In.Color;
    return Output;
}