cbuffer mvp : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

cbuffer camera_info : register(b1)
{
    float4 cameraInfoPos;
};

struct vs_input
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

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

ps_output ps_main(ps_input input)
{
    ps_output output;

    float3 lightColour = float3(1.0f, 1.0f, 1.0f);
    float3 objectColour = float3(1.0f, 0.5f, 0.31f);

    float3 normal = normalize(input.interpolatedNormal);

    float ambientStrength = 0.1f;
    float3 ambient = lightColour * ambientStrength;
    
    float3 lightDir = float3(0.25f, 0.5f, -1.0f);
    float diff = max(dot(lightDir, normal), 0.0f);
    float3 diffuse = lightColour * diff;

    float specularStrength = 0.5f;    
    float3 viewDir = normalize(cameraInfoPos.xyz - input.pixelPosition);
    float3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    float3 specular = specularStrength * spec * lightColour;

    float3 result = (ambient + diffuse + specular) * objectColour;
    output.pixelColour = float4(result, 1.0f);    
    return (output);
}
