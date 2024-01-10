#ifndef TEXTURES_HLSL
#define TEXTURES_HLSL

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
SamplerState samplerState : register(s0);

inline float4 get_sample(Texture2D tex, float2 uv)
{
    return (tex.Sample(samplerState, uv));
}

#endif