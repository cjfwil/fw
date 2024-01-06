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
    float4 cameraPos : POSITION;
};

struct PS_OUTPUT
{
    float4 RGBColor : SV_TARGET;
};

Texture2D tex : register(t0);
Texture2D spec : register(t1);
SamplerState smplr : register(s0);

PS_INPUT vs_main(VS_INPUT input) // main is the default function name
{
    PS_INPUT Output;
    float4 pos = float4(input.vPos, 1.0f);
    // Transform the position from object space to homogeneous projection space
    pos = mul(pos, mWorld);
    pos = mul(pos, View);
    Output.cameraPos = pos;
    pos = mul(pos, Projection);
    Output.Position = pos;
    float4 n = float4(input.vNormal, 0.0f);
    Output.Normal = mul(n, mWorld);
    Output.Normal = mul(Output.Normal, View);
    Output.uv = input.uv;
    
    return Output;
}

PS_OUTPUT ps_main(PS_INPUT In)
{
    PS_OUTPUT Output;

    float3 dir = float3(0.25f, 0.5f, -1.0f);
    float3 ambient = float3(0.05f, 0.05f, 0.05f);
    float3 diffuseLight = float3(1.0f, 1.0f, 1.0f);

    float4 diffuseSample = tex.Sample(smplr, In.uv);
    float4 specularSample = spec.Sample(smplr, In.uv);
    //float3 specColour = specularSample.xyz;
    //float specPower = 1.0f/specularSample.a;

    float3 specColour = float3(1.0f, 1.0f, 1.0f);
    float specPower = 30.0f;
    //clip(diffuseSample.a < 0.1f ? -1 : 1); //alpha test

    float3 lightPos = float3(0.0f, 0.0f, 0.0f);
    lightPos = mul(float4(lightPos, 1.0f), View).xyz;
    float3 vToL = lightPos - In.cameraPos.xyz;
    float distToL = length(vToL);
    float3 dirToL = vToL / distToL;

    float attConst = 1.0f;
    float attLin = 0.045f;
    float attQuad = 0.0075f;

    float att = attConst + attLin * distToL + attQuad * (distToL * distToL);


    // gouraud calculation
    // float3 outClr = diffuse.xyz * ambient;
    // outClr += saturate(dot(dir, In.Normal.xyz) * diffuseLight * diffuse.xyz);

    float diffuseIntensity = 1.0f;
    // float3 diffuse = diffuseSample.xyz * diffuseIntensity * att * max(0.0f, dot(dirToL, In.Normal.xyz));
    float3 diffuse = diffuseIntensity * 1.0f/att * max(0.0f, dot(dirToL, In.Normal.xyz));

    // float specularIntensity = 0.6f;
    float3 specularIntensity = specColour;
    float3 w = In.Normal.xyz * dot(vToL, In.Normal.xyz);
    float3 r = w * 2.0f - vToL;
    float3 specular = 1.0f/att * (diffuseSample.xyz * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(In.cameraPos.xyz))), specPower);

    float3 outClr = saturate(diffuse + ambient + specular);

    Output.RGBColor = float4(outClr * diffuseSample.xyz, 1.0f);
    return Output;
}