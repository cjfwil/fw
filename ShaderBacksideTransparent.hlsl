#include "lighting_calculations.hlsl"
#include "textures.hlsl"
#include "vs.hlsl"

ps_output ps_main(ps_input input)
    : SV_Target
{
    ps_output output;

    float3 lightColour = defaultLightColour;
    float3 lightDir = defaultLightDir;
    float ambientStrength = defaultAmbientStrength;
    float specularStrength = defaultSpecularStrength;

    float4 diffuseSample = get_sample(diffuseTexture, input.interpolatedUV);
    float diffuseAlpha = diffuseSample.a;
    clip(diffuseAlpha < 0.1f ? -1 : 1);

    float4 specularSample = get_sample(specularTexture, input.interpolatedUV);
    float3 objectColour = diffuseSample.rgb;

    float3 normal = normalize(input.interpolatedNormal);

    float3 ambient = lightColour * ambientStrength;

    float diff = diffuse_shading(normal, lightDir);
    float3 diffuse = lightColour * diff;

    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float specularPower = pow(2.71828f, specularSample.a * 9.0f);
    if (diff != 0.0f)
    {
        specular = (lightColour * specularSample.rgb) * specular_highlight_blinn_phong(normal, lightDir, input.pixelPosition, cameraInfoPos.xyz, specularPower);
    }
    float3 result = (ambient + diffuse) * objectColour + specular * specularStrength * diff; //key point is multiply the specular strength by the diff to avoid specular points in darkness
    output.pixelColour = float4(saturate(result), 1.0f);
    return (output);
}
