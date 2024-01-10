#include "vs.hlsl"

#include "lighting_calculations.hlsl"

ps_output ps_main(ps_input input)
{
    ps_output output;

    float3 objectColour = defaultObjectColour;
    float3 lightDir = defaultLightDir;
    float3 lightColour = defaultLightColour;
    float specularStrength = defaultSpecularStrength;

    float3 normal = normalize(input.interpolatedNormal);

    float ambientStrength = 0.1f;
    float3 ambient = lightColour * ambientStrength;

    float diff = diffuse_shading(normal, lightDir);
    float3 diffuse = lightColour * diff;

    float3 specular = float3(0.0f, 0.0f, 0.0f);    
    if (diff != 0.0f)
    {                
        specular = lightColour * specular_highlight_blinn_phong(normal, lightDir, input.pixelPosition, cameraInfoPos.xyz);
    }
    float3 result = (ambient + diffuse) * objectColour + specular * specularStrength;
    output.pixelColour = float4(result, 1.0f);
    return (output);
}
