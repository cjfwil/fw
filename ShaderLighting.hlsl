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
    float4 Normal : NORMAL;
};

struct PS_OUTPUT
{
    float4 RGBColor : SV_TARGET;
};

Texture2D tex : register(t0);
SamplerState smplr : register(s0);

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

    float3 dir = float3(0.25f, 0.5f, -1.0f);
    float3 ambient = float3(0.2f, 0.2f, 0.2f);
    float3 diffuseLight = float3(1.0f, 1.0f, 1.0f);

    float4 diffuse = tex.Sample(smplr, float2(In.uv.x % 1, In.uv.y % 1));

    // gouraud calculation
    float3 outClr = diffuse.xyz * ambient;
    outClr += saturate(dot(dir, In.Normal.xyz) * diffuseLight * diffuse.xyz);

    Output.RGBColor = float4(outClr, 1.0f);
    return Output;
}