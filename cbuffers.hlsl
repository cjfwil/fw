#ifndef CBUFFERS_HLSL
#define CBUFFERS_HLSL

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

#endif