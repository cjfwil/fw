#include "vs.hlsl"

#include "lighting_calculations.hlsl"
#include "light_properties.hlsl"

ps_output ps_main(ps_input input)
{
    ps_output output;

    float3 lightDir = defaultLightDir;
    float3 lightColour = defaultLightColour;
    float ambientStrength = defaultAmbientStrength;
    float specularStrength = defaultSpecularStrength;
    float3 objectColour = defaultObjectColour;

    float3 normal = normalize(input.interpolatedNormal);

    float3 ambient = lightColour * ambientStrength;
    float3 diffuse = lightColour * diffuse_shading(normal, lightDir);
    float3 specular = lightColour * specular_highlight_phong(normal, lightDir, input.pixelPosition, cameraInfoPos.xyz);

    float3 result = (ambient + diffuse) * objectColour + specular * specularStrength;
    output.pixelColour = float4(result, 1.0f);    
    return (output);
}
