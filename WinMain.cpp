#include "D3D11Window.cpp"
#include "D3D11Renderer.cpp"

static bool mouseLookOn = true;

void Init()
{
    RECT rc;
    GetClientRect(d3d11_window.hwnd, &rc);
    MapWindowPoints(d3d11_window.hwnd, nullptr, (POINT *)(&rc), 2);

    bool boundCursor = false;
    if (boundCursor) ClipCursor(&rc);
}

void Update()
{
    static POINT currentMousePos;
    POINT lastMousePos = currentMousePos;
    GetCursorPos(&currentMousePos);

    // RECT rect;
    // GetWindowRect(d3d11_window.hwnd, &rect);
    // POINT screenCenter = {rect.left + (rect.right - rect.left) / 2, rect.top + (rect.bottom - rect.top) / 2};

    // if (currentMousePos.x > rect.right || currentMousePos.x < rect.left || currentMousePos.y > rect.bottom || currentMousePos.y < rect.top) {
    //     SetCursorPos(screenCenter.x, screenCenter.y);
    // }

    if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0)
    {
        mouseLookOn = true;
    }
    

    RECT rc;
    POINT topLeft, bottomRight;
    GetClientRect(d3d11_window.hwnd, &rc);
    topLeft.x = rc.left;
    topLeft.y = rc.top;
    bottomRight.x = rc.right;
    bottomRight.y = rc.bottom;

    ClientToScreen(d3d11_window.hwnd, &topLeft);
    ClientToScreen(d3d11_window.hwnd, &bottomRight);
    // GetWindowRect(d3d11_window.hwnd, &rc);

    // if (mouseLookOn && (currentMousePos.y >= bottomRight.y || currentMousePos.y <= topLeft.y || currentMousePos.x >= bottomRight.x || currentMousePos.x <= topLeft.x))
    // {
    //     POINT centre;
    //     centre.x = rc.right / 2;
    //     centre.y = rc.bottom / 2;
    //     ClientToScreen(d3d11_window.hwnd, &centre);
    //     SetCursorPos(centre.x, centre.y);
    //     lastMousePos = centre;
    // }

    static float yaw, pitch;
    if (mouseLookOn)
    {
        // int dx = currentMousePos.x - lastMousePos.x; // calculate mouse movement
        // int dy = currentMousePos.y - lastMousePos.y;

        float cameraSpeed = 0.025f; // adjust as needed
        yaw -= (float)dx * -cameraSpeed;
        pitch -= (float)dy * cameraSpeed;
        pitch = max(-DirectX::XM_PIDIV2 + 0.0001f, min(DirectX::XM_PIDIV2 - 0.0001f, pitch));

        dx = 0;
        dy = 0;
    }

    // update camera direction based on yaw and pitch
    DirectX::XMVECTOR viewDir = DirectX::XMVectorSet(
        cosf(yaw) * cosf(pitch),
        sinf(pitch),
        sinf(yaw) * cosf(pitch),
        0.0f);
    viewDir = DirectX::XMVector4Normalize(viewDir);

    if (mouseLookOn)
    {
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
                if (RegisterRawInputDevices(&mouseRID, 1, sizeof(mouseRID)) == FALSE) {
                    //TODO: error no mouse input 
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

                            // RECT rect;
                            // GetClientRect(d3d11_window.hwnd, &rect); // get dimensions of your window
                            // ClipCursor(&rect);                       // constrain cursor to your window
                            // SetCapture(d3d11_window.hwnd);           // capture all mouse input

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