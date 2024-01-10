#ifndef LIGHTING_PROPERTIES_HLSL
#define LIGHTING_PROPERTIES_HLSL

static float3 defaultLightColour = float3(1.0f, 1.0f, 1.0f);
static float3 defaultLightDir = float3(0.25f, 0.5f, -1.0f);
static float defaultAmbientStrength = 0.1f;
static float defaultSpecularStrength = 0.5f;
static float3 defaultObjectColour = float3(1.0f, 0.5f, 0.31f);

#define DEFAULT_SPECULAR_POWER 32

#endif