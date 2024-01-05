#pragma warning(push, 0)
#pragma warning(disable : 4365)
#pragma warning(disable : 4668)
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma warning(pop)

#include "win32_expandable_list.hpp"
#include "D3D11Window.cpp"
#include "D3D11Renderer.cpp"

static bool mouseLookOn;

void InitImgui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(d3d11_window.hwnd);
    ImGui_ImplDX11_Init(d3d11_window.device, d3d11_window.context);
}

void StartImgui()
{
    // (Your code process and dispatch Win32 messages)
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    // ImGui::ShowDemoWindow(); // Show demo window! :)

    ImGui::Begin("Info");
    ImGui::Text("%.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Text("%d meshes", mainModel.numElements);
    ImGui::Text("%d loaded textures", numLoadedTextures);
    ImGui::Text("%d null textures", numNullTextures);
    ImGui::End();
}

void EndImgui()
{
    // Rendering
    // (Your code clears your framebuffer, renders your other stuff etc.)
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    // (Your code calls swapchain's Present() function)
}

void CleanupImgui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

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

    InitImgui();
}

void Update()
{
    bool escapeKeyPressed = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
    static bool escapeKeyWasPressed = false;

    mouseLookOn = d3d11_window.cursorHidden;
    if (mouseLookOn && !d3d11_window.cursorIsBound)
        BoundCursorMovement();
    // TODO move out to keyboard state system
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
    if (frameCount == MAXUINT)
        frameCount = 0;

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
    //context->OMSetBlendState(blendState, NULL, 0xffffffff);

    for (unsigned int i = 0; i < mainModel.numElements; ++i)
    {
        mesh_buffers vi = mainModel.data[i];
        // Set up the IA stage by setting the input topology and layout.
        UINT stride = sizeof(VertexPositionUVNormal);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &vi.vertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(vi.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->IASetInputLayout(inputLayout);
        // Set up the vertex shader stage.
        context->VSSetShader(vertexShader, nullptr, 0);
        context->VSSetConstantBuffers(0, 1, &constantBuffer);
        // Set up the pixel shader stage.
        context->PSSetShader(pixelShader, nullptr, 0);
        context->PSSetShaderResources(0u, 1u, &textureViews.data[vi.textureIndex]);
        context->PSSetSamplers(0, 1, &samplerState);
        // Calling Draw tells Direct3D to start sending commands to the graphics
        context->DrawIndexed(vi.indexCount, 0, 0);
    }
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
                            // TODO: maybe rethink how this works (onresize func inside struct called from wndproc?)
                            CreateWindowSizeDependentResources();
                            d3d11_window.resize = FALSE;
                        }
                        Update();

                        StartImgui();
                        Render();
                        EndImgui();
                        d3d11_window.swapChain->Present(0, 0);
                    }
                }
            }
        }
    }
    CleanupImgui();
    return (0);
}