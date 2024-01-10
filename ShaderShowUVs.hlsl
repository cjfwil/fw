#include "vs.hlsl"

ps_output ps_main(ps_input input)
{
    ps_output output;
    output.pixelColour = float4(input.interpolatedUV.x % 1, input.interpolatedUV.y % 1, 0.5f, 1.0f);
    return (output);
}
