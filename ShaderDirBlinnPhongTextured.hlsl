#include "vs.hlsl"

#include "textures.hlsl"
#include "lighting_calculations.hlsl"

ps_output ps_main(ps_input input)
{
    ps_output output;

    float3 lightColour = defaultLightColour;
    float4 sample = get_sample(diffuseTexture, input.interpolatedUV);
    float3 objectColour = sample.xyz;

    float3 normal = normalize(input.interpolatedNormal);

    float ambientStrength = defaultAmbientStrength;
    float3 ambient = lightColour * ambientStrength;

    float3 lightDir = defaultLightDir;
    float diff = diffuse_shading(normal, lightDir);
    float3 diffuse = lightColour * diff;

    float specularStrength = defaultSpecularStrength;
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    if (diff != 0.0f)
    {        
        specular = lightColour * specular_highlight_blinn_phong(normal, lightDir, input.pixelPosition, cameraInfoPos.xyz);        
    }
    float3 result = (ambient + diffuse) * objectColour + specular * specularStrength;
    output.pixelColour = float4(result, 1.0f);
    return (output);
}
