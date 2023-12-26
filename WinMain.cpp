#include "D3D11Window.cpp"
#include "D3D11Renderer.cpp"


static bool mouseLookOn;

void Init()
{
    mouseLookOn = d3d11_window.cursorHidden;
    if (mouseLookOn)
    {
        CentreCursor();

        BoundCursorMovement();
        HideCursor();
        dx = 0;
        dy = 0;
    }
}

void Update()
{
    bool escapeKeyPressed = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
    static bool escapeKeyWasPressed = false;

    mouseLookOn = d3d11_window.cursorHidden;

    //TODO move out to keyboard state system
    if (escapeKeyPressed && !escapeKeyWasPressed)
    {
        mouseLookOn = !mouseLookOn;
        if (mouseLookOn)
        {
            BoundCursorMovement();
            HideCursor();
            dx = 0;
            dy = 0;
        }
        else
        {
            UnboundCursorMovement();
            UnhideCursor();
            dx = 0;
            dy = 0;
        }
    }

    static float yaw = -3.14f / 2, pitch = 0;
    static DirectX::XMVECTOR viewDir = at;

    if (mouseLookOn)
    {
        float cameraSpeed = 0.0025f;
        yaw -= (float)dx * -cameraSpeed;
        pitch -= (float)dy * cameraSpeed;
        pitch = max(-DirectX::XM_PIDIV2 + 0.0001f, min(DirectX::XM_PIDIV2 - 0.0001f, pitch));

        dx = 0;
        dy = 0;

        viewDir = DirectX::XMVectorSet(
            cosf(yaw) * cosf(pitch),
            sinf(pitch),
            sinf(yaw) * cosf(pitch),
            0.0f);
        viewDir = DirectX::XMVector4Normalize(viewDir);
        at = DirectX::XMVectorAdd(eye, viewDir);
    }

    DirectX::XMVECTOR right = DirectX::XMVector3Cross(viewDir, up);
    viewDir = DirectX::XMVector3Normalize(viewDir);
    right = DirectX::XMVector3Normalize(right);

    float moveSpeed = 1 / 60.0f;
    DirectX::XMVECTOR dir = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    if (GetAsyncKeyState('W') & 0x8000)
        dir = DirectX::XMVectorAdd(dir, viewDir);
    if (GetAsyncKeyState('S') & 0x8000)
        dir = DirectX::XMVectorSubtract(dir, viewDir);
    if (GetAsyncKeyState('A') & 0x8000)
        dir = DirectX::XMVectorSubtract(dir, right);
    if (GetAsyncKeyState('D') & 0x8000)
        dir = DirectX::XMVectorAdd(dir, right);

    dir = DirectX::XMVectorScale(dir, moveSpeed);

    eye = DirectX::XMVectorAdd(eye, dir);

    at = DirectX::XMVectorAdd(at, dir);

    DirectX::XMStoreFloat4x4(
        &constantBufferData.view,
        DirectX::XMMatrixTranspose(
            DirectX::XMMatrixLookAtRH(
                eye,
                at,
                up)));

    // Rotate the cube 1 degree per frame.
    // DirectX::XMStoreFloat4x4(
    //     &constantBufferData.world,
    //     DirectX::XMMatrixTranspose(
    //         DirectX::XMMatrixRotationY(
    //             DirectX::XMConvertToRadians(
    //                 (float)frameCount++))));
    // if (frameCount == MAXUINT)
    //     frameCount = 0;

    // Update the state of the escape key
    escapeKeyWasPressed = escapeKeyPressed;
}

void Render()
{
    // Use the Direct3D device context to draw.
    ID3D11DeviceContext *context = d3d11_window.context;
    ID3D11RenderTargetView *renderTarget = d3d11_window.renderTargetView;
    ID3D11DepthStencilView *depthStencil = d3d11_window.depthStencilView;
    context->UpdateSubresource(constantBuffer, 0, nullptr, &constantBufferData, 0, 0);
    // Clear the render target and the z-buffer.
    const float teal[] = {0.098f, 0.439f, 0.439f, 1.000f};
    context->ClearRenderTargetView(renderTarget, teal);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    // Set the render target.
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);
    // Set up the IA stage by setting the input topology and layout.
    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    context->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(inputLayout);
    // Set up the vertex shader stage.
    context->VSSetShader(vertexShader, nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    // Set up the pixel shader stage.
    context->PSSetShader(pixelShader, nullptr, 0);
    context->PSSetShaderResources(0u, 1u, &textureShaderResourceView);
    context->PSSetSamplers(0, 1, &samplerState);
    // Calling Draw tells Direct3D to start sending commands to the graphics
    context->DrawIndexed(indexCount, 0, 0);
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

                RAWINPUTDEVICE mouseRID;
                mouseRID.usUsagePage = 0x0001;
                mouseRID.usUsage = 0x0002;
                mouseRID.dwFlags = 0;
                mouseRID.hwndTarget = d3d11_window.hwnd;
                if (RegisterRawInputDevices(&mouseRID, 1, sizeof(mouseRID)) == FALSE)
                {
                    // TODO: error no mouse input
                }

                Init();

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
                        if (d3d11_window.resize)
                        {
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