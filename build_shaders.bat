@echo off

fxc /T vs_5_0 /E vs_main /Fo VertexShaderShowUVs.cso ShaderShowUVs.hlsl -nologo
fxc /T ps_5_0 /E ps_main /Fo PixelShaderShowUVs.cso ShaderShowUVs.hlsl -nologo

fxc /T vs_5_0 /E vs_main /Fo VertexShaderShowNormals.cso ShaderShowNormals.hlsl -nologo
fxc /T ps_5_0 /E ps_main /Fo PixelShaderShowNormals.cso ShaderShowNormals.hlsl -nologo

fxc /T vs_5_0 /E vs_main /Fo VertexShaderShowDiffuse.cso ShaderShowDiffuse.hlsl -nologo
fxc /T ps_5_0 /E ps_main /Fo PixelShaderShowDiffuse.cso ShaderShowDiffuse.hlsl -nologo

fxc /T vs_5_0 /E vs_main /Fo VertexShaderShowDirLightingSpecular.cso ShaderShowDirLightingSpecular.hlsl -nologo
fxc /T ps_5_0 /E ps_main /Fo PixelShaderShowDirLightingSpecular.cso ShaderShowDirLightingSpecular.hlsl -nologo

fxc /T vs_5_0 /E vs_main /Fo VertexShaderShowDirLightingDiffuse.cso ShaderShowDirLightingDiffuse.hlsl -nologo
fxc /T ps_5_0 /E ps_main /Fo PixelShaderShowDirLightingDiffuse.cso ShaderShowDirLightingDiffuse.hlsl -nologo

fxc /T vs_5_0 /E vs_main /Fo VertexShaderDirBlinnPhong.cso ShaderDirBlinnPhong.hlsl -nologo
fxc /T ps_5_0 /E ps_main /Fo PixelShaderDirBlinnPhong.cso ShaderDirBlinnPhong.hlsl -nologo

fxc /T vs_5_0 /E vs_main /Fo VertexShaderDirBlinnPhongTextured.cso ShaderDirBlinnPhongTextured.hlsl -nologo
fxc /T ps_5_0 /E ps_main /Fo PixelShaderDirBlinnPhongTextured.cso ShaderDirBlinnPhongTextured.hlsl -nologo

fxc /T vs_5_0 /E vs_main /Fo VertexShaderMasterShader.cso ShaderMasterShader.hlsl -nologo
fxc /T ps_5_0 /E ps_main /Fo PixelShaderMasterShader.cso ShaderMasterShader.hlsl -nologo