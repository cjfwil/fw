#ifndef LIGHTING_CALCULATIONS_HLSL
#define LIGHTING_CALCULATIONS_HLSL

#include "light_properties.hlsl"



// 0 to 1, how dark is this pixel. 0: darkness, 1: lightness
inline float diffuse_shading(float3 normal, float3 lightDir)
{
    float diff = max(dot(normalize(lightDir), normalize(normal)), 0.0f);
    return (diff);
}

inline float specular_highlight_phong(float3 normal, float3 lightDir, float3 pixelPosition, float3 cameraPos, float specularPower = DEFAULT_SPECULAR_POWER)
{
    float3 viewDir = normalize(cameraPos - pixelPosition);
    float3 reflectDir = reflect(-normalize(lightDir), normalize(normal));

    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), specularPower);
    return (spec);
}

inline float specular_highlight_blinn_phong(float3 normal, float3 lightDir, float3 pixelPosition, float3 cameraPos, float specularPower = DEFAULT_SPECULAR_POWER)
{
    float3 viewDir = normalize(cameraPos - pixelPosition);
    float3 halfwayVec = normalize(viewDir + lightDir);

    float spec = pow(max(dot(normal, halfwayVec), 0.0f), specularPower);
    return (spec);
}

#endif