#include "D3D11Window.cpp"
#include "D3D11Renderer.cpp"

void Update()
{
    // Rotate the cube 1 degree per frame.
    DirectX::XMStoreFloat4x4(
        &constantBufferData.world,
        DirectX::XMMatrixTranspose(
            DirectX::XMMatrixRotationY(
                DirectX::XMConvertToRadians(
                    (float)frameCount++))));
    if (frameCount == MAXUINT)
        frameCount = 0;
}

void Render()
{
    // Use the Direct3D device context to draw.
    ID3D11DeviceContext *context = d3d11_window.context;
    ID3D11RenderTargetView *renderTarget = d3d11_window.renderTargetView;
    ID3D11DepthStencilView *depthStencil = d3d11_window.depthStencilView;
    context->UpdateSubresource(
        constantBuffer,
        0,
        nullptr,
        &constantBufferData,
        0,
        0);
    // Clear the render target and the z-buffer.
    const float teal[] = {0.098f, 0.439f, 0.439f, 1.000f};
    context->ClearRenderTargetView(
        renderTarget,
        teal);
    context->ClearDepthStencilView(
        depthStencil,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f,
        0);
    // Set the render target.
    context->OMSetRenderTargets(1,
        &renderTarget,
        depthStencil);
    // Set up the IA stage by setting the input topology and layout.
    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    context->IASetVertexBuffers(
        0,
        1,
        &vertexBuffer,
        &stride,
        &offset);
    context->IASetIndexBuffer(
        indexBuffer,
        DXGI_FORMAT_R16_UINT,
        0);

    context->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(inputLayout);
    // Set up the vertex shader stage.
    context->VSSetShader(
        vertexShader,
        nullptr,
        0);
    context->VSSetConstantBuffers(
        0,
        1,
        &constantBuffer);
    // Set up the pixel shader stage.
    context->PSSetShader(
        pixelShader,
        nullptr,
        0);
    // Calling Draw tells Direct3D to start sending commands to the graphics
    context->DrawIndexed(
        indexCount,
        0,
        0);
}

INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    HRESULT hr = S_OK;
    hr = CreateWin32Window();
    if (SUCCEEDED(hr))
    {
        hr = CreateD3D11DeviceResources();
        if (SUCCEEDED(hr))
        {
            CreateDeviceDependentResources();
            hr = CreateD3D11WindowResource();            
            if (SUCCEEDED(hr))
            {
                CreateWindowSizeDependentResources();
                if (!IsWindowVisible(d3d11_window.hwnd))
                    ShowWindow(d3d11_window.hwnd, SW_SHOW);

                bool recievedMessage;
                MSG msg;
                msg.message = WM_NULL;
                PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
                while (WM_QUIT != msg.message)
                {
                    recievedMessage = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
                    if (recievedMessage)
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    else
                    {
                        if (d3d11_window.resize) {
                            CreateWindowSizeDependentResources();
                            d3d11_window.resize = FALSE;
                        }
                        Update();
                        Render();                        
                        d3d11_window.swapChain->Present(1, 0);
                    }
                }
            }
        }
    }
    return (0);
}