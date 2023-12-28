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
    float3 vNormal : NORMAL;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION; // interpolated vertex position (system
    float2 uv : TEXCOORD;
    float4 Normal : NORMAL;         // interpolated diffuse color
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
    float4 n = float4(input.vNormal, 0.0f);
    Output.Normal = mul(n, mWorld);
    Output.uv = input.uv;
    return Output;
}

PS_OUTPUT ps_main(PS_INPUT In)
{
    PS_OUTPUT Output;
    Output.RGBColor = In.Normal;
    return Output;
}