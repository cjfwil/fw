fxc /T vs_5_0 /E vs_main /Fo CubeVertexShader.cso Shader.hlsl
fxc /T ps_5_0 /E ps_main /Fo CubePixelShader.cso Shader.hlsl

fxc /T vs_5_0 /E vs_main /Fo CubeVertexShaderTex.cso ShaderTex.hlsl
fxc /T ps_5_0 /E ps_main /Fo CubePixelShaderTex.cso ShaderTex.hlsl

fxc /T vs_5_0 /E vs_main /Fo CubeVertexShaderLighting.cso ShaderLighting.hlsl
fxc /T ps_5_0 /E ps_main /Fo CubePixelShaderLighting.cso ShaderLighting.hlsl