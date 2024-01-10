#include "vs.hlsl"

#include "textures.hlsl"

ps_output ps_main(ps_input input)
{
    ps_output output;    
    output.pixelColour = get_sample(diffuseTexture, input.interpolatedUV);
    return (output);
}
