#include "vs.hlsl"

ps_output ps_main(ps_input input)
{
    ps_output output;
    float3 normal = normalize(input.interpolatedNormal);
    output.pixelColour = float4(normal, 1.0f);
    return (output);
}
