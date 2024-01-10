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

    float3 ambient = lightColour * ambientStrength;

    float diff = diffuse_shading(input.interpolatedNormal, lightDir);

    float3 diffuse = lightColour * diff;
    float3 result = (diffuse + ambient) * objectColour;
    
    output.pixelColour = float4(result, 1.0f);
    return (output);
}
