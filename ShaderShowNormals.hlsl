cbuffer mvp : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

struct vs_input
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct ps_input
{
    float4 interpolatedPosition : SV_Position;
    float2 interpolatedUV : TEXCOORD;
    float3 interpolatedNormal : NORMAL;
};

struct ps_output
{
    float4 pixelColour : SV_Target;
};

ps_input vs_main(vs_input input)
{
    ps_input output;
    
    float4 pos = float4(input.position, 1.0f);
    pos = mul(pos, model);
    pos = mul(pos, view);
    pos = mul(pos, projection);
    output.interpolatedPosition = pos;
    
    output.interpolatedUV = input.uv;

    float4 normal = float4(input.normal, 1.0f);
    normal = mul(normal, model);    
    output.interpolatedNormal = normal.xyz;
    return (output);
}

ps_output ps_main(ps_input input)
{
    ps_output output;
    output.pixelColour = float4(abs(input.interpolatedNormal), 1.0f);
    return (output);
}
