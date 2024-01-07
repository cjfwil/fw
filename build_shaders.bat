@REM fxc /T vs_5_0 /E vs_main /Fo CubeVertexShader.cso Shader.hlsl
@REM fxc /T ps_5_0 /E ps_main /Fo CubePixelShader.cso Shader.hlsl

@REM fxc /T vs_5_0 /E vs_main /Fo CubeVertexShaderTex.cso ShaderTex.hlsl
@REM fxc /T ps_5_0 /E ps_main /Fo CubePixelShaderTex.cso ShaderTex.hlsl

@REM fxc /T vs_5_0 /E vs_main /Fo CubeVertexShaderLighting.cso ShaderLighting.hlsl
@REM fxc /T ps_5_0 /E ps_main /Fo CubePixelShaderLighting.cso ShaderLighting.hlsl

fxc /T vs_5_0 /E vs_main /Fo VertexShaderShowUVs.cso ShaderShowUVs.hlsl
fxc /T ps_5_0 /E ps_main /Fo PixelShaderShowUVs.cso ShaderShowUVs.hlsl

fxc /T vs_5_0 /E vs_main /Fo VertexShaderShowNormals.cso ShaderShowNormals.hlsl
fxc /T ps_5_0 /E ps_main /Fo PixelShaderShowNormals.cso ShaderShowNormals.hlsl

fxc /T vs_5_0 /E vs_main /Fo VertexShaderShowDiffuse.cso ShaderShowDiffuse.hlsl
fxc /T ps_5_0 /E ps_main /Fo PixelShaderShowDiffuse.cso ShaderShowDiffuse.hlsl

fxc /T vs_5_0 /E vs_main /Fo VertexShaderShowDirLightingSpecular.cso ShaderShowDirLightingSpecular.hlsl
fxc /T ps_5_0 /E ps_main /Fo PixelShaderShowDirLightingSpecular.cso ShaderShowDirLightingSpecular.hlsl